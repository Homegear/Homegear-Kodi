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

#include "KodiPacket.h"

#include "GD.h"

namespace Kodi
{
KodiPacket::KodiPacket()
{
}

KodiPacket::KodiPacket(BaseLib::PVariable& json, int64_t timeReceived)
{
	_timeReceived = timeReceived;
	BaseLib::Struct::iterator jsonIterator = json->structValue->find("method");
	if(jsonIterator != json->structValue->end()) _method = jsonIterator->second->stringValue;
	jsonIterator = json->structValue->find("params");
	if(jsonIterator != json->structValue->end()) _parameters = jsonIterator->second;
	jsonIterator = json->structValue->find("result");
	if(jsonIterator != json->structValue->end()) _result = jsonIterator->second;
}

KodiPacket::KodiPacket(std::string method, BaseLib::PVariable parameters, int64_t timeReceived)
{
	_timeReceived = timeReceived;
	_method = method;
	_parameters = parameters;
}

KodiPacket::~KodiPacket()
{
}

std::string KodiPacket::getMethod()
{
	return _method;
}

BaseLib::PVariable KodiPacket::getParameters()
{
	return _parameters;
}

BaseLib::PVariable KodiPacket::getResult()
{
	return _result;
}

BaseLib::PVariable KodiPacket::getJson()
{
	try
	{
		BaseLib::PVariable json(new BaseLib::Variable(BaseLib::VariableType::tStruct));
		json->structValue->insert(BaseLib::StructElement("jsonrpc", BaseLib::PVariable(new BaseLib::Variable(std::string("2.0")))));
		json->structValue->insert(BaseLib::StructElement("method", BaseLib::PVariable(new BaseLib::Variable(_method))));
		json->structValue->insert(BaseLib::StructElement("params", _parameters));
		return json;
	}
	catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return BaseLib::PVariable();
}

}
