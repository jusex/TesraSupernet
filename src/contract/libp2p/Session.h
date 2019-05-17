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
/** @file Session.h
 * @author Gav Wood <i@gavwood.com>
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <array>
#include <deque>
#include <set>
#include <memory>
#include <utility>

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/Guards.h>
#include "RLPXFrameCoder.h"
#include "RLPXSocket.h"
#include "Common.h"
#include "RLPXFrameWriter.h"
#include "RLPXFrameReader.h"

namespace dev
{

namespace p2p
{

class Peer;
class ReputationManager;

class SessionFace
{
public:
	virtual ~SessionFace() {}

	virtual void start() = 0;
	virtual void disconnect(DisconnectReason _reason) = 0;

	virtual void ping() = 0;

	virtual bool isConnected() const = 0;

	virtual NodeID id() const = 0;

	virtual void sealAndSend(RLPStream& _s, uint16_t _protocolID) = 0;

	virtual int rating() const = 0;
	virtual void addRating(int _r) = 0;

	virtual void addNote(std::string const& _k, std::string const& _v) = 0;

	virtual PeerSessionInfo info() const = 0;
	virtual std::chrono::steady_clock::time_point connectionTime() = 0;

	virtual void registerCapability(CapDesc const& _desc, std::shared_ptr<Capability> _p) = 0;
	virtual void registerFraming(uint16_t _id) = 0;

	virtual std::map<CapDesc, std::shared_ptr<Capability>> const&  capabilities() const = 0;

	virtual std::shared_ptr<Peer> peer() const = 0;

	virtual std::chrono::steady_clock::time_point lastReceived() const = 0;

	virtual ReputationManager& repMan() = 0;
};

/**
 * @brief The Session class
 * @todo Document fully.
 */
class Session: public SessionFace, public std::enable_shared_from_this<SessionFace>
{
public:
	static bool isFramingAllowedForVersion(unsigned _version) { return _version > 4; }

	Session(Host* _server, std::unique_ptr<RLPXFrameCoder>&& _io, std::shared_ptr<RLPXSocket> const& _s, std::shared_ptr<Peer> const& _n, PeerSessionInfo _info);
	virtual ~Session();

	void start() override;
	void disconnect(DisconnectReason _reason) override;

	void ping() override;

	bool isConnected() const override { return m_socket->ref().is_open(); }

	NodeID id() const override;

	void sealAndSend(RLPStream& _s, uint16_t _protocolID) override;

	int rating() const override;
	void addRating(int _r) override;

	void addNote(std::string const& _k, std::string const& _v) override { Guard l(x_info); m_info.notes[_k] = _v; }

	PeerSessionInfo info() const override { Guard l(x_info); return m_info; }
	std::chrono::steady_clock::time_point connectionTime() override { return m_connect; }

	void registerCapability(CapDesc const& _desc, std::shared_ptr<Capability> _p) override;
	void registerFraming(uint16_t _id) override;

	std::map<CapDesc, std::shared_ptr<Capability>> const& capabilities() const override { return m_capabilities; }

	std::shared_ptr<Peer> peer() const override { return m_peer; }

	std::chrono::steady_clock::time_point lastReceived() const override { return m_lastReceived; }

	ReputationManager& repMan() override;

private:
	static RLPStream& prep(RLPStream& _s, PacketType _t, unsigned _args = 0);

	void send(bytes&& _msg, uint16_t _protocolID);

	
	void drop(DisconnectReason _r);

	
	void doRead();
	void doReadFrames();
	
	
	bool checkRead(std::size_t _expected, boost::system::error_code _ec, std::size_t _length);

	
	void write();
	void writeFrames();

	
	bool readPacket(uint16_t _capId, PacketType _t, RLP const& _r);

	
	bool interpret(PacketType _t, RLP const& _r);
	
	
	static bool checkPacket(bytesConstRef _msg);

	Host* m_server;							

	std::unique_ptr<RLPXFrameCoder> m_io;	
	std::shared_ptr<RLPXSocket> m_socket;		
	Mutex x_framing;						
	std::deque<bytes> m_writeQueue;			
	std::vector<byte> m_data;			    
	bytes m_incoming;						

	std::shared_ptr<Peer> m_peer;			
	bool m_dropped = false;					

	mutable Mutex x_info;
	PeerSessionInfo m_info;						

	std::chrono::steady_clock::time_point m_connect;		
	std::chrono::steady_clock::time_point m_ping;			
	std::chrono::steady_clock::time_point m_lastReceived;	

	std::map<CapDesc, std::shared_ptr<Capability>> m_capabilities;	

	
	struct Framing
	{
		Framing() = delete;
		Framing(uint16_t _protocolID): writer(_protocolID), reader(_protocolID) {}
		RLPXFrameWriter writer;
		RLPXFrameReader reader;
	};

	std::map<uint16_t, std::shared_ptr<Framing> > m_framing;
	std::deque<bytes> m_encFrames;

	bool isFramingEnabled() const { return isFramingAllowedForVersion(m_info.protocolVersion); }
	unsigned maxFrameSize() const { return 1024; }
	std::shared_ptr<Framing> getFraming(uint16_t _protocolID);
	void multiplexAll();
};

template <class PeerCap>
std::shared_ptr<PeerCap> capabilityFromSession(SessionFace const& _session, u256 const& _version = PeerCap::version())
{ 
	try 
	{ 
		return std::static_pointer_cast<PeerCap>(_session.capabilities().at(std::make_pair(PeerCap::name(), _version)));
	}
	catch (...)
	{
		return nullptr;
	}
}

}
}
