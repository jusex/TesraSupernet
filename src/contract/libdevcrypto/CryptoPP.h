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
/** @file CryptoPP.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 *
 * CryptoPP headers and primitive helper methods
 */

#pragma once

#include "Common.h"

namespace dev
{
namespace crypto
{

static const unsigned c_eciesOverhead = 113;

/**
 * CryptoPP secp256k1 algorithms.
 * @todo Collect ECIES methods into class.
 */
class Secp256k1PP
{	
public:
	static Secp256k1PP* get();

	
	void encrypt(Public const& _k, bytes& io_cipher);
	
	
	void decrypt(Secret const& _k, bytes& io_text);
	
	
	void encryptECIES(Public const& _k, bytes& io_cipher);
	
	
	void encryptECIES(Public const& _k, bytesConstRef _sharedMacData, bytes& io_cipher);
	
	
	bool decryptECIES(Secret const& _k, bytes& io_text);
	
	
	bool decryptECIES(Secret const& _k, bytesConstRef _sharedMacData, bytes& io_text);
	
	
	bytes eciesKDF(Secret const& _z, bytes _s1, unsigned kdBitLen = 256);

private:
	Secp256k1PP() = default;
};

}
}

