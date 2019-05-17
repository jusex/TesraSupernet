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
/** @file RLPXHandshake.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2015
 */


#pragma once

#include <memory>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/ECDHE.h>
#include "RLPXSocket.h"
#include "RLPXFrameCoder.h"
#include "Common.h"
namespace ba = boost::asio;
namespace bi = boost::asio::ip;

namespace dev
{
namespace p2p
{

static const unsigned c_rlpxVersion = 4;

/**
 * @brief Setup inbound or outbound connection for communication over RLPXFrameCoder.
 * RLPx Spec: https:
 *
 * @todo Implement StartSession transition via lambda which is passed to constructor.
 *
 * Thread Safety
 * Distinct Objects: Safe.
 * Shared objects: Unsafe.
 */
class RLPXHandshake: public std::enable_shared_from_this<RLPXHandshake>
{
	friend class RLPXFrameCoder;

public:
	
	RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket): m_host(_host), m_originated(false), m_socket(_socket), m_idleTimer(m_socket->ref().get_io_service()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }
	
	
	RLPXHandshake(Host* _host, std::shared_ptr<RLPXSocket> const& _socket, NodeID _remote): m_host(_host), m_remote(_remote), m_originated(true), m_socket(_socket), m_idleTimer(m_socket->ref().get_io_service()) { crypto::Nonce::get().ref().copyTo(m_nonce.ref()); }

	~RLPXHandshake() {}

	
	void start() { transition(); }

	
	void cancel();

protected:
	
	enum State
	{
		Error = -1,
		New,
		AckAuth,
		AckAuthEIP8,
		WriteHello,
		ReadHello,
		StartSession
	};

	
	void writeAuth();

	
	void readAuth();

	
	void readAuthEIP8();

	
	void setAuthValues(Signature const& sig, Public const& remotePubk, h256 const& remoteNonce, uint64_t remoteVersion);
	
	
	void writeAck();

	
	void writeAckEIP8();
	
	
	void readAck();

	
	void readAckEIP8();
	
	
	void error();
	
	
	virtual void transition(boost::system::error_code _ech = boost::system::error_code());

	
	boost::posix_time::milliseconds const c_timeout = boost::posix_time::milliseconds(1800);

	State m_nextState = New;			
	bool m_cancel = false;			
	
	Host* m_host;					
	
	
	NodeID m_remote;					
	bool m_originated = false;		
	
	
	bytes m_auth;					
	bytes m_authCipher;				
	bytes m_ack;						
	bytes m_ackCipher;				
	bytes m_handshakeOutBuffer;		
	bytes m_handshakeInBuffer;		
	
	crypto::ECDHE m_ecdhe;			
	h256 m_nonce;					
	
	Public m_remoteEphemeral;			
	h256 m_remoteNonce;				
	uint64_t m_remoteVersion;
	
	
	
	std::unique_ptr<RLPXFrameCoder> m_io;
	
	std::shared_ptr<RLPXSocket> m_socket;		
	boost::asio::deadline_timer m_idleTimer;	
};
	
}
}
