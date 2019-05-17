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
/** @file RLPXFrameCoder.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2015
 */


#pragma once

#include <memory>
#include <libdevcore/Guards.h>
#include <libdevcrypto/ECDHE.h>
#include <libdevcrypto/CryptoPP.h>
#include "Common.h"

namespace dev
{
namespace p2p
{

struct RLPXFrameDecryptFailed: virtual dev::Exception {};

/**
 * @brief Encapsulation of Frame
 * @todo coder integration; padding derived from coder
 */
struct RLPXFrameInfo
{
	RLPXFrameInfo() = default;
	
	RLPXFrameInfo(bytesConstRef _frameHeader);

	uint32_t const length;			
	uint8_t const padding;			
	
	bytes const data;				
	RLP const header;				
	
	uint16_t const protocolId;		
	bool const multiFrame;			
	uint16_t const sequenceId;		
	uint32_t const totalLength;		
};

class RLPXHandshake;

/**
 * @brief Encoder/decoder transport for RLPx connection established by RLPXHandshake.
 *
 * @todo rename to RLPXTranscoder
 * @todo Remove 'Frame' nomenclature and expect caller to provide RLPXFrame
 * @todo Remove handshake as friend, remove handshake-based constructor
 *
 * Thread Safety 
 * Distinct Objects: Unsafe.
 * Shared objects: Unsafe.
 */
class RLPXFrameCoder
{
	friend class RLPXFrameIOMux;
	friend class Session;
public:
	
	RLPXFrameCoder(RLPXHandshake const& _init);
	
	
	RLPXFrameCoder(bool _originated, h512 const& _remoteEphemeral, h256 const& _remoteNonce, crypto::ECDHE const& _ephemeral, h256 const& _nonce, bytesConstRef _ackCipher, bytesConstRef _authCipher);
	
	~RLPXFrameCoder();
	
	
	void setup(bool _originated, h512 const& _remoteEphemeral, h256 const& _remoteNonce, crypto::ECDHE const& _ephemeral, h256 const& _nonce, bytesConstRef _ackCipher, bytesConstRef _authCipher);
	
	
	void writeFrame(uint16_t _protocolType, bytesConstRef _payload, bytes& o_bytes);

	
	void writeFrame(uint16_t _protocolType, uint16_t _seqId, bytesConstRef _payload, bytes& o_bytes);
	
	
	void writeFrame(uint16_t _protocolType, uint16_t _seqId, uint32_t _totalSize, bytesConstRef _payload, bytes& o_bytes);
	
	
	void writeSingleFramePacket(bytesConstRef _packet, bytes& o_bytes);

	
	bool authAndDecryptHeader(bytesRef io_cipherWithMac);
	
	
	bool authAndDecryptFrame(bytesRef io_cipherWithMac);
	
	
	h128 egressDigest();

	
	h128 ingressDigest();

protected:
	void writeFrame(RLPStream const& _header, bytesConstRef _payload, bytes& o_bytes);
	
	
	void updateEgressMACWithHeader(bytesConstRef _headerCipher);

	
	void updateEgressMACWithFrame(bytesConstRef _cipher);
	
	
	void updateIngressMACWithHeader(bytesConstRef _headerCipher);
	
	
	void updateIngressMACWithFrame(bytesConstRef _cipher);

private:
	std::unique_ptr<class RLPXFrameCoderImpl> m_impl;
};

}
}