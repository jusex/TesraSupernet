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
/** @file NodeTable.h
 * @author Alex Leverington <nessence@gmail.com>
 * @date 2014
 */

#pragma once

#include <algorithm>
#include <deque>

#include <boost/integer/static_log2.hpp>

#include <libp2p/UDP.h>
#include "Common.h"

namespace dev
{
namespace p2p
{

/**
 * NodeEntry
 * @brief Entry in Node Table
 */
struct NodeEntry: public Node
{
	NodeEntry(NodeID const& _src, Public const& _pubk, NodeIPEndpoint const& _gw);
	unsigned const distance;	
	bool pending = true;		
};

enum NodeTableEventType
{
	NodeEntryAdded,
	NodeEntryDropped
};

class NodeTable;
class NodeTableEventHandler
{
	friend class NodeTable;
public:
	virtual void processEvent(NodeID const& _n, NodeTableEventType const& _e) = 0;

protected:
	
	void processEvents()
	{
		std::list<std::pair<NodeID, NodeTableEventType>> events;
		{
			Guard l(x_events);
			if (!m_nodeEventHandler.size())
				return;
			m_nodeEventHandler.unique();
			for (auto const& n: m_nodeEventHandler)
				events.push_back(std::make_pair(n,m_events[n]));
			m_nodeEventHandler.clear();
			m_events.clear();
		}
		for (auto const& e: events)
			processEvent(e.first, e.second);
	}

	
	virtual void appendEvent(NodeID _n, NodeTableEventType _e) { Guard l(x_events); m_nodeEventHandler.push_back(_n); m_events[_n] = _e; }

	Mutex x_events;
	std::list<NodeID> m_nodeEventHandler;
	std::unordered_map<NodeID, NodeTableEventType> m_events;
};

class NodeTable;
inline std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);

/**
 * NodeTable using modified kademlia for node discovery and preference.
 * Node table requires an IO service, creates a socket for incoming
 * UDP messages and implements a kademlia-like protocol. Node requests and
 * responses are used to build a node table which can be queried to
 * obtain a list of potential nodes to connect to, and, passes events to
 * Host whenever a node is added or removed to/from the table.
 *
 * Thread-safety is ensured by modifying NodeEntry details via
 * shared_ptr replacement instead of mutating values.
 *
 * NodeTable accepts a port for UDP and will listen to the port on all available
 * interfaces.
 *
 * [Optimization]
 * @todo serialize evictions per-bucket
 * @todo store evictions in map, unit-test eviction logic
 * @todo store root node in table
 * @todo encapsulate discover into NetworkAlgorithm (task)
 * @todo expiration and sha3(id) 'to' for messages which are replies (prevents replay)
 * @todo cache Ping and FindSelf
 *
 * [Networking]
 * @todo eth/upnp/natpmp/stun/ice/etc for public-discovery
 * @todo firewall
 *
 * [Protocol]
 * @todo optimize knowledge at opposite edges; eg, s_bitsPerStep lookups. (Can be done via pointers to NodeBucket)
 * @todo ^ s_bitsPerStep = 8; 
 */
class NodeTable: UDPSocketEvents, public std::enable_shared_from_this<NodeTable>
{
	friend std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable);
	using NodeSocket = UDPSocket<NodeTable, 1280>;
	using TimePoint = std::chrono::steady_clock::time_point;	
	using NodeIdTimePoint = std::pair<NodeID, TimePoint>;
	using EvictionTimeout = std::pair<NodeIdTimePoint, NodeID>;	
	
public:
	enum NodeRelation { Unknown = 0, Known };
	enum DiscoverType { Random = 0 };
	
	
	NodeTable(ba::io_service& _io, KeyPair const& _alias, NodeIPEndpoint const& _endpoint, bool _enabled = true);
	~NodeTable();

	
	static unsigned distance(NodeID const& _a, NodeID const& _b) { u256 d = sha3(_a) ^ sha3(_b); unsigned ret; for (ret = 0; d >>= 1; ++ret) {}; return ret; }

	
	void setEventHandler(NodeTableEventHandler* _handler) { m_nodeEventHandler.reset(_handler); }

	
	void processEvents();

	
	std::shared_ptr<NodeEntry> addNode(Node const& _node, NodeRelation _relation = NodeRelation::Unknown);

	
	std::list<NodeID> nodes() const;

	
	unsigned count() const { return m_nodes.size(); }

	
	std::list<NodeEntry> snapshot() const;

	
	bool haveNode(NodeID const& _id) { Guard l(x_nodes); return m_nodes.count(_id) > 0; }

	
	Node node(NodeID const& _id);

#if defined(BOOST_AUTO_TEST_SUITE) || defined(_MSC_VER) 
protected:
#else
private:
#endif

	

	static unsigned const s_addressByteSize = h256::size;					
	static unsigned const s_bits = 8 * s_addressByteSize;					
	static unsigned const s_bins = s_bits - 1;								
	static unsigned const s_maxSteps = boost::static_log2<s_bits>::value;	

	

	static unsigned const s_bucketSize = 16;			
	static unsigned const s_alpha = 3;				

	

	
	std::chrono::milliseconds const c_evictionCheckInterval = std::chrono::milliseconds(75);	
	std::chrono::milliseconds const c_reqTimeout = std::chrono::milliseconds(300);						
	std::chrono::milliseconds const c_bucketRefresh = std::chrono::milliseconds(7200);							

	struct NodeBucket
	{
		unsigned distance;
		std::list<std::weak_ptr<NodeEntry>> nodes;
	};

	
	void ping(NodeIPEndpoint _to) const;

	
	void ping(NodeEntry* _n) const;

	
	NodeEntry center() const { return NodeEntry(m_node.id, m_node.publicKey(), m_node.endpoint); }

	
	std::shared_ptr<NodeEntry> nodeEntry(NodeID _id);

	
	
	void doDiscover(NodeID _target, unsigned _round = 0, std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>> _tried = std::shared_ptr<std::set<std::shared_ptr<NodeEntry>>>());

	
	std::vector<std::shared_ptr<NodeEntry>> nearestNodeEntries(NodeID _target);

	
	void evict(std::shared_ptr<NodeEntry> _leastSeen, std::shared_ptr<NodeEntry> _new);

	
	void noteActiveNode(Public const& _pubk, bi::udp::endpoint const& _endpoint);

	
	void dropNode(std::shared_ptr<NodeEntry> _n);

	
	
	
	NodeBucket& bucket_UNSAFE(NodeEntry const* _n);

	

	
	void onReceived(UDPSocketFace*, bi::udp::endpoint const& _from, bytesConstRef _packet);

	
	void onDisconnected(UDPSocketFace*) {}

	

	
	void doCheckEvictions();

	
	void doDiscovery();

	std::unique_ptr<NodeTableEventHandler> m_nodeEventHandler;		

	Node m_node;													
	Secret m_secret;												

	mutable Mutex x_nodes;											
	std::unordered_map<NodeID, std::shared_ptr<NodeEntry>> m_nodes;	

	mutable Mutex x_state;											
	std::array<NodeBucket, s_bins> m_state;							

	Mutex x_evictions;												
	std::deque<EvictionTimeout> m_evictions;						
	
	Mutex x_pubkDiscoverPings;										
	std::unordered_map<bi::address, TimePoint> m_pubkDiscoverPings;	

	Mutex x_findNodeTimeout;
	std::list<NodeIdTimePoint> m_findNodeTimeout;					

	std::shared_ptr<NodeSocket> m_socket;							
	NodeSocket* m_socketPointer;									

	DeadlineOps m_timers; 
};

inline std::ostream& operator<<(std::ostream& _out, NodeTable const& _nodeTable)
{
	_out << _nodeTable.center().address() << "\t" << "0\t" << _nodeTable.center().endpoint.address << ":" << _nodeTable.center().endpoint.udpPort << std::endl;
	auto s = _nodeTable.snapshot();
	for (auto n: s)
		_out << n.address() << "\t" << n.distance << "\t" << n.endpoint.address << ":" << n.endpoint.udpPort << std::endl;
	return _out;
}

struct DiscoveryDatagram: public RLPXDatagramFace
{
	
	DiscoveryDatagram(bi::udp::endpoint const& _to): RLPXDatagramFace(_to), ts(futureFromEpoch(std::chrono::seconds(60))) {}

	
	DiscoveryDatagram(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): RLPXDatagramFace(_from), sourceid(_fromid), echo(_echo) {}

	
	NodeID sourceid; 
	h256 echo;       

	
	
	uint32_t ts = 0;
	bool isExpired() const { return secondsSinceEpoch() > ts; }

	
	static std::unique_ptr<DiscoveryDatagram> interpretUDP(bi::udp::endpoint const& _from, bytesConstRef _packet);
};

/**
 * Ping packet: Sent to check if node is alive.
 * PingNode is cached and regenerated after ts + t, where t is timeout.
 *
 * Ping is used to implement evict. When a new node is seen for
 * a given bucket which is full, the least-responsive node is pinged.
 * If the pinged node doesn't respond, then it is removed and the new
 * node is inserted.
 */
struct PingNode: DiscoveryDatagram
{
	using DiscoveryDatagram::DiscoveryDatagram;
	PingNode(NodeIPEndpoint const& _src, NodeIPEndpoint const& _dest): DiscoveryDatagram(_dest), source(_src), destination(_dest) {}
	PingNode(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

	static const uint8_t type = 1;
	uint8_t packetType() const { return type; }

	unsigned version = 0;
	NodeIPEndpoint source;
	NodeIPEndpoint destination;

	void streamRLP(RLPStream& _s) const
	{
		_s.appendList(4);
		_s << dev::p2p::c_protocolVersion;
		source.streamRLP(_s);
		destination.streamRLP(_s);
		_s << ts;
	}
	void interpretRLP(bytesConstRef _bytes)
	{
		RLP r(_bytes, RLP::AllowNonCanon|RLP::ThrowOnFail);
		version = r[0].toInt<unsigned>();
		source.interpretRLP(r[1]);
		destination.interpretRLP(r[2]);
		ts = r[3].toInt<uint32_t>();
	}
};

/**
 * Pong packet: Sent in response to ping
 */
struct Pong: DiscoveryDatagram
{
	Pong(NodeIPEndpoint const& _dest): DiscoveryDatagram((bi::udp::endpoint)_dest), destination(_dest) {}
	Pong(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

	static const uint8_t type = 2;
	uint8_t packetType() const { return type; }

	NodeIPEndpoint destination;

	void streamRLP(RLPStream& _s) const
	{
		_s.appendList(3);
		destination.streamRLP(_s);
		_s << echo;
		_s << ts;
	}
	void interpretRLP(bytesConstRef _bytes)
	{
		RLP r(_bytes, RLP::AllowNonCanon|RLP::ThrowOnFail);
		destination.interpretRLP(r[0]);
		echo = (h256)r[1];
		ts = r[2].toInt<uint32_t>();
	}
};

/**
 * FindNode Packet: Request k-nodes, closest to the target.
 * FindNode is cached and regenerated after ts + t, where t is timeout.
 * FindNode implicitly results in finding neighbours of a given node.
 *
 * RLP Encoded Items: 2
 * Minimum Encoded Size: 21 bytes
 * Maximum Encoded Size: 30 bytes
 *
 * target: NodeID of node. The responding node will send back nodes closest to the target.
 *
 */
struct FindNode: DiscoveryDatagram
{
	FindNode(bi::udp::endpoint _to, h512 _target): DiscoveryDatagram(_to), target(_target) {}
	FindNode(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

	static const uint8_t type = 3;
	uint8_t packetType() const { return type; }

	h512 target;

	void streamRLP(RLPStream& _s) const
	{
		_s.appendList(2); _s << target << ts;
	}
	void interpretRLP(bytesConstRef _bytes)
	{
		RLP r(_bytes, RLP::AllowNonCanon|RLP::ThrowOnFail);
		target = r[0].toHash<h512>();
		ts = r[1].toInt<uint32_t>();
	}
};

/**
 * Node Packet: One or more node packets are sent in response to FindNode.
 */
struct Neighbours: DiscoveryDatagram
{
	Neighbours(bi::udp::endpoint _to, std::vector<std::shared_ptr<NodeEntry>> const& _nearest, unsigned _offset = 0, unsigned _limit = 0): DiscoveryDatagram(_to)
	{
		auto limit = _limit ? std::min(_nearest.size(), (size_t)(_offset + _limit)) : _nearest.size();
		for (auto i = _offset; i < limit; i++)
			neighbours.push_back(Neighbour(*_nearest[i]));
	}
	Neighbours(bi::udp::endpoint const& _to): DiscoveryDatagram(_to) {}
	Neighbours(bi::udp::endpoint const& _from, NodeID const& _fromid, h256 const& _echo): DiscoveryDatagram(_from, _fromid, _echo) {}

	struct Neighbour
	{
		Neighbour(Node const& _node): endpoint(_node.endpoint), node(_node.id) {}
		Neighbour(RLP const& _r): endpoint(_r) { node = h512(_r[3].toBytes()); }
		NodeIPEndpoint endpoint;
		NodeID node;
		void streamRLP(RLPStream& _s) const { _s.appendList(4); endpoint.streamRLP(_s, NodeIPEndpoint::StreamInline); _s << node; }
	};

	static const uint8_t type = 4;
	uint8_t packetType() const { return type; }

	std::vector<Neighbour> neighbours;

	void streamRLP(RLPStream& _s) const
	{
		_s.appendList(2);
		_s.appendList(neighbours.size());
		for (auto const& n: neighbours)
			n.streamRLP(_s);
		_s << ts;
	}
	void interpretRLP(bytesConstRef _bytes)
	{
		RLP r(_bytes, RLP::AllowNonCanon|RLP::ThrowOnFail);
		for (auto const& n: r[0])
			neighbours.emplace_back(n);
		ts = r[1].toInt<uint32_t>();
	}
};

struct NodeTableWarn: public LogChannel { static const char* name(); static const int verbosity = 0; };
struct NodeTableNote: public LogChannel { static const char* name(); static const int verbosity = 1; };
struct NodeTableMessageSummary: public LogChannel { static const char* name(); static const int verbosity = 2; };
struct NodeTableMessageDetail: public LogChannel { static const char* name(); static const int verbosity = 5; };
struct NodeTableConnect: public LogChannel { static const char* name(); static const int verbosity = 10; };
struct NodeTableEvent: public LogChannel { static const char* name(); static const int verbosity = 10; };
struct NodeTableTimer: public LogChannel { static const char* name(); static const int verbosity = 10; };
struct NodeTableUpdate: public LogChannel { static const char* name(); static const int verbosity = 10; };
struct NodeTableTriviaSummary: public LogChannel { static const char* name(); static const int verbosity = 10; };
struct NodeTableTriviaDetail: public LogChannel { static const char* name(); static const int verbosity = 11; };
struct NodeTableAllDetail: public LogChannel { static const char* name(); static const int verbosity = 13; };
struct NodeTableEgress: public LogChannel { static const char* name(); static const int verbosity = 14; };
struct NodeTableIngress: public LogChannel { static const char* name(); static const int verbosity = 15; };

}
}
