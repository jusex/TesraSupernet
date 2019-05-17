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
/** @file ECDHE.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 *
 * Elliptic curve Diffie-Hellman ephemeral key exchange
 */

#pragma once

#include "Common.h"

namespace dev
{
namespace crypto
{

/**
 * @brief Derive DH shared secret from EC keypairs.
 * As ephemeral keys are single-use, agreement is limited to a single occurrence.
 */
class ECDHE
{
public:
	
	ECDHE(): m_ephemeral(KeyPair::create()) {};

	
	Public pubkey() { return m_ephemeral.pub(); }
	
	Secret seckey() { return m_ephemeral.secret(); }
	
	
	void agree(Public const& _remoteEphemeral, Secret& o_sharedSecret) const;
	
protected:
	KeyPair m_ephemeral;					
	mutable Public m_remoteEphemeral;		
};

}
}

