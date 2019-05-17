/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http:
*/
/** @file Precompiled.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <unordered_map>
#include <functional>
#include <libdevcore/CommonData.h>
#include <libdevcore/Exceptions.h>

namespace dev
{
namespace eth
{

using PrecompiledExecutor = std::function<std::pair<bool, bytes>(bytesConstRef _in)>;

DEV_SIMPLE_EXCEPTION(ExecutorNotFound);

class PrecompiledRegistrar
{
public:
	
	static PrecompiledExecutor const& executor(std::string const& _name);

	
	static PrecompiledExecutor registerPrecompiled(std::string const& _name, PrecompiledExecutor const& _exec) { return (get()->m_execs[_name] = _exec); }
	
	static void unregisterPrecompiled(std::string const& _name) { get()->m_execs.erase(_name); }

private:
	static PrecompiledRegistrar* get() { if (!s_this) s_this = new PrecompiledRegistrar; return s_this; }

	std::unordered_map<std::string, PrecompiledExecutor> m_execs;
	static PrecompiledRegistrar* s_this;
};


#define ETH_REGISTER_PRECOMPILED(Name) static std::pair<bool, bytes> __eth_registerPrecompiledFunction ## Name(bytesConstRef _in); static PrecompiledExecutor __eth_registerPrecompiledFactory ## Name = ::dev::eth::PrecompiledRegistrar::registerPrecompiled(#Name, &__eth_registerPrecompiledFunction ## Name); static std::pair<bool, bytes> __eth_registerPrecompiledFunction ## Name

}
}
