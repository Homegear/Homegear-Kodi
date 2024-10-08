/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#include "GD.h"
#include "KodiInterface.h"

namespace Kodi {

KodiInterface::KodiInterface() {
  _out.init(GD::bl);
  _out.setPrefix(GD::out.getPrefix() + "Kodi interface: ");

  signal(SIGPIPE, SIG_IGN);

  C1Net::TcpSocketInfo tcp_socket_info;
  auto dummy_socket = std::make_shared<C1Net::Socket>(-1);
  _socket = std::make_unique<C1Net::TcpSocket>(tcp_socket_info, dummy_socket);

  _jsonEncoder.reset(new BaseLib::Rpc::JsonEncoder(GD::bl));
  _jsonDecoder.reset(new BaseLib::Rpc::JsonDecoder(GD::bl));
}

KodiInterface::~KodiInterface() {
  try {
    _stopCallbackThread = true;
    GD::bl->threadManager.join(_listenThread);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::setConnectedCallback(std::function<void(bool connected)> callback) {
  _connectedCallback = callback;
}

void KodiInterface::setPacketReceivedCallback(std::function<void(std::shared_ptr < KodiPacket > packet)> callback) {
  _packetReceivedCallback = callback;
}

std::string KodiInterface::getHostname() { return _hostname; }

void KodiInterface::setHostname(std::string &hostname) {
  try {
    _hostname = hostname;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t KodiInterface::getPort() { return _port; }

void KodiInterface::setPort(int32_t port) {
  try {
    _port = port;
    if (_port < 1 || _port > 65535) _port = 9090;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::getResponse(BaseLib::PVariable &request, BaseLib::PVariable &response) {
  try {
    if (_stopped || request->type != BaseLib::VariableType::tStruct) return;

    uint32_t requestId = _currentRequestId++;
    request->structValue->insert(BaseLib::StructElement("id", BaseLib::PVariable(new Variable(requestId))));

    std::string json;
    _jsonEncoder->encode(request, json);
    if (json.empty()) return;

    std::lock_guard<std::mutex> getResponseGuard(_getResponseMutex);
    std::shared_ptr<Request> request(new Request());
    _requestsMutex.lock();
    _requests[requestId] = request;
    _requestsMutex.unlock();
    std::unique_lock<std::mutex> lock(request->mutex);

    try {
      _out.printInfo("Info: Sending packet " + json);
      _socket->Send((uint8_t *)json.data(), json.size());
    }
    catch (const BaseLib::Exception &ex) {
      _out.printError("Error sending packet to Kodi: " + std::string(ex.what()));
      return;
    }

    if (!request->conditionVariable.wait_for(lock, std::chrono::milliseconds(10000), [&] { return request->mutexReady; })) {
      _out.printError("Error: No response received to packet: " + json);
    }
    response = request->response;

    _requestsMutex.lock();
    _requests.erase(requestId);
    _requestsMutex.unlock();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    _requestsMutex.unlock();
  }
}

void KodiInterface::sendPacket(std::shared_ptr<BaseLib::Systems::Packet> packet) {
  try {
    if (!packet) {
      _out.printWarning("Warning: Packet was nullptr.");
      return;
    }

    std::shared_ptr<KodiPacket> kodiPacket(std::dynamic_pointer_cast<KodiPacket>(packet));
    if (!kodiPacket) return;

    PVariable json = kodiPacket->getJson();
    if (!json) return;

    json->print(false, true);

    PVariable response;
    getResponse(json, response);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::reconnect() {
  try {
    if (_connectedCallback) _connectedCallback(false);
    _socket->Shutdown();
    _out.printDebug("Connecting to Kodi with hostname " + _hostname + " on port " + std::to_string(_port) + "...");
    _socket->Open();
    _out.printInfo("Connected to Kodi with hostname " + _hostname + " on port " + std::to_string(_port) + ".");
    _stopped = false;
    if (_connectedCallback) _connectedCallback(true);
  }
  catch (const std::exception &ex) {
    _out.printDebug(ex.what());
  }
}

void KodiInterface::startListening() {
  try {
    stopListening();
    if (_hostname.empty()) return;

    C1Net::TcpSocketInfo tcp_socket_info;
    tcp_socket_info.read_timeout = 5000;
    tcp_socket_info.write_timeout = 5000;

    C1Net::TcpSocketHostInfo tcp_socket_host_info{
        .host = _hostname,
        .port = (uint16_t)_port,
        .connection_retries = 1
    };

    _socket = std::make_unique<C1Net::TcpSocket>(tcp_socket_info, tcp_socket_host_info);

    GD::bl->threadManager.start(_listenThread, true, &KodiInterface::listen, this);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::stopListening() {
  try {
    if (_connectedCallback) _connectedCallback(false);
    _stopCallbackThread = true;
    GD::bl->threadManager.join(_listenThread);
    _stopCallbackThread = false;
    _socket->Shutdown();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::listen() {
  try {
    uint32_t receivedBytes = 0;
    int32_t bufferMax = 4096;
    std::vector<char> buffer(bufferMax);
    std::vector<char> data;
    bool more_data = false;

    while (!_stopCallbackThread) {
      if (_stopped) {
        data.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (_stopCallbackThread) return;
        _out.printDebug("Debug: Connection to Kodi closed. Trying to reconnect...");
        reconnect();
        continue;
      }
      try {
        do {
          receivedBytes = _socket->Read((uint8_t *)buffer.data(), buffer.size(), more_data);
          if (receivedBytes > 0) {
            data.insert(data.end(), buffer.begin(), buffer.begin() + receivedBytes);
            if (data.size() > 1000000) {
              _out.printError("Could not read from Kodi: Too much data.");
              break;
            }
          }
        } while (receivedBytes == (unsigned)bufferMax);
      }
      catch (const C1Net::TimeoutException &ex) {
        data.clear();
        continue;
      }
      catch (const C1Net::ClosedException &ex) {
        _stopped = true;
        _out.printInfo("Info: " + std::string(ex.what()));
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        continue;
      }
      catch (const C1Net::Exception &ex) {
        _stopped = true;
        _out.printError("Error: " + std::string(ex.what()));
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        continue;
      }
      if (data.empty() || data.size() > 1000000) continue;

      if (GD::bl->debugLevel >= 5) {
        std::string debug((char *)&data.at(0), data.size());
        _out.printDebug("Debug: Packet received from Kodi: " + debug);
      }

      BaseLib::PVariable json;
      uint32_t bytesRead = 0;
      try {
        while (bytesRead < data.size()) {
          json = _jsonDecoder->decode(data, bytesRead);
          if (bytesRead < data.size()) {
            std::vector<char> newData(data.begin() + bytesRead, data.end());
            data.swap(newData);
            bytesRead = 0;
            if (json) processData(json);
          } else {
            data.clear();
            if (json) processData(json);
            break;
          }
        }
      }
      catch (BaseLib::Rpc::JsonDecoderException &ex) {
        continue;
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void KodiInterface::processData(BaseLib::PVariable &json) {
  try {
    BaseLib::Struct::iterator idIterator = json->structValue->find("id");
    if (idIterator != json->structValue->end()) {
      _requestsMutex.lock();
      std::map < uint32_t, std::shared_ptr < Request >> ::iterator
      requestIterator = _requests.find(idIterator->second->integerValue);
      if (requestIterator != _requests.end()) {
        std::shared_ptr<Request> request = requestIterator->second;
        _requestsMutex.unlock();
        request->response = json;
        {
          std::lock_guard<std::mutex> lock(request->mutex);
          request->mutexReady = true;
        }
        request->conditionVariable.notify_one();
        return;
      } else _requestsMutex.unlock();
    }

    std::shared_ptr<KodiPacket> packet(new KodiPacket(json, BaseLib::HelperFunctions::getTime()));
    if (_packetReceivedCallback) _packetReceivedCallback(packet);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
