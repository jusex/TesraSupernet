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
/** @file ICAP.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Ethereum-specific data structures & algorithms.
 */

#pragma once

#include <string>
#include <functional>
#include <boost/algorithm/string/case_conv.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/FixedHash.h>
#include "Common.h"

namespace dev
{
namespace eth
{

DEV_SIMPLE_EXCEPTION(InvalidICAP);

/**
 * @brief Encapsulation of an ICAP address.
 * Can be encoded, decoded, looked-up and inspected.
 */
class ICAP
{
public:
	
	ICAP() = default;
	
	ICAP(Address const& _target): m_type(Direct), m_direct(_target) {}
	
	ICAP(std::string const& _client, std::string const& _inst): m_type(Indirect), m_client(boost::algorithm::to_upper_copy(_client)), m_institution(boost::algorithm::to_upper_copy(_inst)), m_asset("XET") {}
	
	ICAP(std::string const& _c, std::string const& _i, std::string const& _a): m_type(Indirect), m_client(boost::algorithm::to_upper_copy(_c)), m_institution(boost::algorithm::to_upper_copy(_i)), m_asset(boost::algorithm::to_upper_copy(_a)) {}

	
	enum Type
	{
		Invalid,
		Direct,
		Indirect
	};

	
	static Secret createDirect();

	
	static std::string iban(std::string _c, std::string _d);
	
	static std::pair<std::string, std::string> fromIBAN(std::string _iban);

	
	static ICAP decoded(std::string const& _encoded);

	
	std::string encoded() const;
	
	Type type() const { return m_type; }
	
	Address const& direct() const { return m_type == Direct ? m_direct : ZeroAddress; }
	
	std::string const& asset() const { return m_type == Indirect ? m_asset : EmptyString; }
	
	std::string const& target() const { return m_type == Indirect && m_asset == "ETH" ? m_client : EmptyString; }
	
	std::string const& institution() const { return m_type == Indirect && m_asset == "XET" ? m_institution : EmptyString; }
	
	std::string const& client() const { return m_type == Indirect && m_asset == "XET" ? m_client : EmptyString; }
	
	std::pair<Address, bytes> address(std::function<bytes(Address, bytes)> const& _call, Address const& _reg) const { return m_type == Direct ? make_pair(direct(), bytes()) : m_type == Indirect ? lookup(_call, _reg) : make_pair(Address(), bytes()); }

	
	std::pair<Address, bytes> lookup(std::function<bytes(Address, bytes)> const& _call, Address const& _reg) const;

private:
	Type m_type = Invalid;
	Address m_direct;
	std::string m_client;
	std::string m_institution;
	std::string m_asset;
};


}
}
