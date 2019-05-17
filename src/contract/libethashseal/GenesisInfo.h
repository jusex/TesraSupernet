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
/** @file GenesisInfo.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <string>
#include <libdevcore/FixedHash.h>
#include <libethcore/Common.h>

namespace dev
{
namespace eth
{


enum class Network
{
	
	MainNetwork = 1,		
	
	Ropsten = 3,			
	MainNetworkTest = 69,	
	TransitionnetTest = 70,	
	FrontierTest = 71,		
	HomesteadTest = 72,		
	EIP150Test = 73,		
	EIP158Test = 74,		
	MetropolisTest = 75,    
	Special = 0xff,			
	tesraMainNetwork = 9,    
	tesraTestNetwork = 10
};

std::string const& genesisInfo(Network _n);
h256 const& genesisStateRoot(Network _n);

}
}
