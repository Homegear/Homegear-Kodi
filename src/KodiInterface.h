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

#ifndef KODIINTERFACE_H_
#define KODIINTERFACE_H_

#include "KodiPacket.h"
#include <homegear-base/BaseLib.h>

namespace Kodi
{

using namespace BaseLib;

class KodiInterface
{
public:
	KodiInterface();
	virtual ~KodiInterface();

	void setConnectedCallback(std::function<void(bool connected)> callback);
	void setPacketReceivedCallback(std::function<void(std::shared_ptr<KodiPacket> packet)> callback);
	void sendPacket(std::shared_ptr<BaseLib::Systems::Packet> packet);
	std::string getHostname();
	void setHostname(std::string& hostname);
	int32_t getPort();
	void setPort(int32_t port);
	void startListening();
	void stopListening();
protected:
	class Request
	{
	public:
		std::mutex mutex;
		std::condition_variable conditionVariable;
		bool mutexReady = false;
		BaseLib::PVariable response;

		Request() {}
		virtual ~Request() {}
	private:
	};

	BaseLib::Output _out;
	std::unique_ptr<C1Net::TcpSocket> _socket;
	std::string _hostname;
	int32_t _port = 9090;
	std::unique_ptr<BaseLib::Rpc::JsonEncoder> _jsonEncoder;
	std::unique_ptr<BaseLib::Rpc::JsonDecoder> _jsonDecoder;
	std::function<void(bool connected)> _connectedCallback;
	std::function<void(std::shared_ptr<KodiPacket> packet)> _packetReceivedCallback;

	std::thread _listenThread;
	bool _stopCallbackThread = false;
	bool _stopped = true;

	uint32_t _currentRequestId = 0;
	std::mutex _getResponseMutex;
	std::mutex _requestsMutex;
	std::map<uint32_t, std::shared_ptr<Request>> _requests;
	std::mutex _sendMutex;

	void getResponse(BaseLib::PVariable& request, BaseLib::PVariable& response);
	void reconnect();
	void listen();
	void processData(BaseLib::PVariable& json);
};

}

#endif
