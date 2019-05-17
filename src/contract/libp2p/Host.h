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
/** @file Host.h
 * @author Alex Leverington <nessence@gmail.com>
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>

#include <libdevcore/Guards.h>
#include <libdevcore/Worker.h>
#include <libdevcrypto/Common.h>
#include <libdevcrypto/ECDHE.h>
#include "NodeTable.h"
#include "HostCapability.h"
#include "Network.h"
#include "Peer.h"
#include "RLPXSocket.h"
#include "RLPXFrameCoder.h"
#include "Common.h"
namespace ba = boost::asio;
namespace bi = ba::ip;

namespace std
{
template<> struct hash<pair<dev::p2p::NodeID, string>>
{
	size_t operator()(pair<dev::p2p::NodeID, string> const& _value) const
	{
		size_t ret = hash<dev::p2p::NodeID>()(_value.first);
		return ret ^ (hash<string>()(_value.second) + 0x9e3779b9 + (ret << 6) + (ret >> 2));
	}
};
}

namespace dev
{

namespace p2p
{

class Host;

class HostNodeTableHandler: public NodeTableEventHandler
{
public:
	HostNodeTableHandler(Host& _host);

	Host const& host() const { return m_host; }

private:
	virtual void processEvent(NodeID const& _n, NodeTableEventType const& _e);

	Host& m_host;
};

struct SubReputation
{
	bool isRude = false;
	int utility = 0;
	bytes data;
};

struct Reputation
{
	std::unordered_map<std::string, SubReputation> subs;
};

class ReputationManager
{
public:
	ReputationManager();

	void noteRude(SessionFace const& _s, std::string const& _sub = std::string());
	bool isRude(SessionFace const& _s, std::string const& _sub = std::string()) const;
	void setData(SessionFace const& _s, std::string const& _sub, bytes const& _data);
	bytes data(SessionFace const& _s, std::string const& _subs) const;

private:
	std::unordered_map<std::pair<p2p::NodeID, std::string>, Reputation> m_nodes;	
	SharedMutex mutable x_nodes;
};

struct NodeInfo
{
	NodeInfo() = default;
	NodeInfo(NodeID const& _id, std::string const& _address, unsigned _port, std::string const& _version):
		id(_id), address(_address), port(_port), version(_version) {}

	std::string enode() const { return "enode:

	NodeID id;
	std::string address;
	unsigned port;
	std::string version;
};

/**
 * @brief The Host class
 * Capabilities should be registered prior to startNetwork, since m_capabilities is not thread-safe.
 *
 * @todo determinePublic: ipv6, udp
 * @todo per-session keepalive/ping instead of broadcast; set ping-timeout via median-latency
 */
class Host: public Worker
{
	friend class HostNodeTableHandler;
	friend class RLPXHandshake;
	
	friend class Session;
	friend class HostCapabilityFace;

public:
	
	Host(
		std::string const& _clientVersion,
		NetworkPreferences const& _n = NetworkPreferences(),
		bytesConstRef _restoreNetwork = bytesConstRef()
	);

	
	
	Host(
		std::string const& _clientVersion,
		KeyPair const& _alias,
		NetworkPreferences const& _n = NetworkPreferences()
	);

	
	virtual ~Host();

	
	static std::unordered_map<Public, std::string> pocHosts();

	
	template <class T> std::shared_ptr<T> registerCapability(std::shared_ptr<T> const& _t) { _t->m_host = this; m_capabilities[std::make_pair(T::staticName(), T::staticVersion())] = _t; return _t; }
	template <class T> void addCapability(std::shared_ptr<T> const & _p, std::string const& _name, u256 const& _version) { m_capabilities[std::make_pair(_name, _version)] = _p; }

	bool haveCapability(CapDesc const& _name) const { return m_capabilities.count(_name) != 0; }
	CapDescs caps() const { CapDescs ret; for (auto const& i: m_capabilities) ret.push_back(i.first); return ret; }
	template <class T> std::shared_ptr<T> cap() const { try { return std::static_pointer_cast<T>(m_capabilities.at(std::make_pair(T::staticName(), T::staticVersion()))); } catch (...) { return nullptr; } }

	
	void addPeer(NodeSpec const& _s, PeerType _t);

	
	void addNode(NodeID const& _node, NodeIPEndpoint const& _endpoint);
	
	
	void requirePeer(NodeID const& _node, NodeIPEndpoint const& _endpoint);

	
	void requirePeer(NodeID const& _node, bi::address const& _addr, unsigned short _udpPort, unsigned short _tcpPort) { requirePeer(_node, NodeIPEndpoint(_addr, _udpPort, _tcpPort)); }

	
	void relinquishPeer(NodeID const& _node);
	
	
	void setIdealPeerCount(unsigned _n) { m_idealPeerCount = _n; }

	
	void setPeerStretch(unsigned _n) { m_stretchPeers = _n; }
	
	
	PeerSessionInfos peerSessionInfo() const;

	
	size_t peerCount() const;

	
	std::string listenAddress() const { return m_tcpPublic.address().is_unspecified() ? "0.0.0.0" : m_tcpPublic.address().to_string(); }

	
	unsigned short listenPort() const { return std::max(0, m_listenPort); }

	
	bytes saveNetwork() const;

	
	Peers getPeers() const { RecursiveGuard l(x_sessions); Peers ret; for (auto const& i: m_peers) ret.push_back(*i.second); return ret; }

	NetworkPreferences const& networkPreferences() const { return m_netPrefs; }

	void setNetworkPreferences(NetworkPreferences const& _p, bool _dropPeers = false) { m_dropPeers = _dropPeers; auto had = isStarted(); if (had) stop(); m_netPrefs = _p; if (had) start(); }

	
	void start();

	
	
	void stop();

	
	bool isStarted() const { return isWorking(); }

	
	ReputationManager& repMan() { return m_repMan; }

	
	bool haveNetwork() const { Guard l(x_runTimer); return m_run && !!m_nodeTable; }
	
	
	void startPeerSession(Public const& _id, RLP const& _hello, std::unique_ptr<RLPXFrameCoder>&& _io, std::shared_ptr<RLPXSocket> const& _s);

	
	std::shared_ptr<SessionFace> peerSession(NodeID const& _id) { RecursiveGuard l(x_sessions); return m_sessions.count(_id) ? m_sessions[_id].lock() : std::shared_ptr<SessionFace>(); }

	
	NodeID id() const { return m_alias.pub(); }

	
	bi::tcp::endpoint const& tcpPublic() const { return m_tcpPublic; }

	
	std::string enode() const { return "enode:

	
	p2p::NodeInfo nodeInfo() const { return NodeInfo(id(), (networkPreferences().publicIPAddress.empty() ? m_tcpPublic.address().to_string() : networkPreferences().publicIPAddress), m_tcpPublic.port(), m_clientVersion); }

protected:
	void onNodeTableEvent(NodeID const& _n, NodeTableEventType const& _e);

	
	void restoreNetwork(bytesConstRef _b);

private:
	enum PeerSlotType { Egress, Ingress };
	
	unsigned peerSlots(PeerSlotType _type) { return _type == Egress ? m_idealPeerCount : m_idealPeerCount * m_stretchPeers; }
	
	bool havePeerSession(NodeID const& _id) { return !!peerSession(_id); }

	
	void determinePublic();

	void connect(std::shared_ptr<Peer> const& _p);

	
	bool peerSlotsAvailable(PeerSlotType _type = Ingress) { Guard l(x_pendingNodeConns); return peerCount() + m_pendingPeerConns.size() < peerSlots(_type); }
	
	
	void keepAlivePeers();

	
	void disconnectLatePeers();

	
	void runAcceptor();

	
	virtual void startedWorking();
	
	void run(boost::system::error_code const& error);			

	
	virtual void doWork();

	
	virtual void doneWorking();

	
	static KeyPair networkAlias(bytesConstRef _b);

	bytes m_restoreNetwork;										

	bool m_run = false;													
	mutable std::mutex x_runTimer;	

	std::string m_clientVersion;											

	NetworkPreferences m_netPrefs;										

	
	std::set<bi::address> m_ifAddresses;								

	int m_listenPort = -1;												

	ba::io_service m_ioService;											
	bi::tcp::acceptor m_tcp4Acceptor;										

	std::unique_ptr<boost::asio::deadline_timer> m_timer;					
	static const unsigned c_timerInterval = 100;							

	std::set<Peer*> m_pendingPeerConns;									
	Mutex x_pendingNodeConns;

	bi::tcp::endpoint m_tcpPublic;											
	KeyPair m_alias;															
	std::shared_ptr<NodeTable> m_nodeTable;									

	
	std::unordered_map<NodeID, std::shared_ptr<Peer>> m_peers;
	
	
	std::set<NodeID> m_requiredPeers;
	Mutex x_requiredPeers;

	
	
	mutable std::unordered_map<NodeID, std::weak_ptr<SessionFace>> m_sessions;
	mutable RecursiveMutex x_sessions;
	
	std::list<std::weak_ptr<RLPXHandshake>> m_connecting;					
	Mutex x_connecting;													

	unsigned m_idealPeerCount = 11;										
	unsigned m_stretchPeers = 7;										

	std::map<CapDesc, std::shared_ptr<HostCapabilityFace>> m_capabilities;	
	
	
	std::list<std::shared_ptr<boost::asio::deadline_timer>> m_timers;
	Mutex x_timers;

	std::chrono::steady_clock::time_point m_lastPing;						
	bool m_accepting = false;
	bool m_dropPeers = false;

	ReputationManager m_repMan;
};

}
}
