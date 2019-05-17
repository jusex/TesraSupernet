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
/** @file EthereumPeer.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <array>
#include <memory>
#include <utility>

#include <libdevcore/RLP.h>
#include <libdevcore/Guards.h>
#include <libethcore/Common.h>
#include <libp2p/Capability.h>
#include "CommonNet.h"

namespace dev
{
namespace eth
{

class EthereumPeerObserverFace
{
public:
	virtual ~EthereumPeerObserverFace() {}

	virtual void onPeerStatus(std::shared_ptr<EthereumPeer> _peer) = 0;

	virtual void onPeerTransactions(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0;

	virtual void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _headers) = 0;

	virtual void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0;

	virtual void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes) = 0;

	virtual void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0;

	virtual void onPeerNodeData(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0;

	virtual void onPeerReceipts(std::shared_ptr<EthereumPeer> _peer, RLP const& _r) = 0;

	virtual void onPeerAborting() = 0;
};

class EthereumHostDataFace
{
public:
	virtual ~EthereumHostDataFace() {}

	virtual std::pair<bytes, unsigned> blockHeaders(RLP const& _blockId, unsigned _maxHeaders, u256 _skip, bool _reverse) const = 0;

	virtual std::pair<bytes, unsigned> blockBodies(RLP const& _blockHashes) const = 0;

	virtual strings nodeData(RLP const& _dataHashes) const = 0;

	virtual std::pair<bytes, unsigned> receipts(RLP const& _blockHashes) const = 0;
};

/**
 * @brief The EthereumPeer class
 * @todo Document fully.
 * @todo make state transitions thread-safe.
 */
class EthereumPeer: public p2p::Capability
{
	friend class EthereumHost; 
	friend class BlockChainSync; 

public:
	
	EthereumPeer(std::shared_ptr<p2p::SessionFace> _s, p2p::HostCapabilityFace* _h, unsigned _i, p2p::CapDesc const& _cap, uint16_t _capID);

	
	virtual ~EthereumPeer();

	
	static std::string name() { return "eth"; }

	
	static u256 version() { return c_protocolVersion; }

	
	static unsigned messageCount() { return PacketCount; }

	void init(unsigned _hostProtocolVersion, u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash, std::shared_ptr<EthereumHostDataFace> _hostData, std::shared_ptr<EthereumPeerObserverFace> _observer);

	p2p::NodeID id() const { return session()->id(); }

	
	void setIdle();

	
	void requestBlockHeaders(h256 const& _startHash, unsigned _count, unsigned _skip, bool _reverse);
	void requestBlockHeaders(unsigned _startNumber, unsigned _count, unsigned _skip, bool _reverse);

	
	void requestBlockBodies(h256s const& _blocks);

	
	void requestNodeData(h256s const& _hashes);

	
	void requestReceipts(h256s const& _blocks);

	
	bool isRude() const;

	
	void setRude();

	
	void abortSync();

private:
	using p2p::Capability::sealAndSend;

	
	unsigned askOverride() const;

	
	virtual bool interpret(unsigned _id, RLP const& _r);

	
	void requestStatus(u256 _hostNetworkId, u256 _chainTotalDifficulty, h256 _chainCurrentHash, h256 _chainGenesisHash);

	
	void clearKnownTransactions() { std::lock_guard<std::mutex> l(x_knownTransactions); m_knownTransactions.clear(); }

	
	void requestByHashes(h256s const& _hashes, Asking _asking, SubprotocolPacketType _packetType);

	
	void setAsking(Asking _g);

	
	bool needsSyncing() const { return !isRude() && !!m_latestHash; }

	
	bool isConversing() const;

	
	bool isCriticalSyncing() const;

	
	void tick();

	unsigned m_hostProtocolVersion = 0;

	
	unsigned m_protocolVersion;

	
	u256 m_networkId;

	
	Asking m_asking = Asking::Nothing;
	
	std::atomic<time_t> m_lastAsk;

	
	h256 m_latestHash;						
	u256 m_totalDifficulty;					
	h256 m_genesisHash;						

	u256 const m_peerCapabilityVersion;			
	
	bool m_requireTransactions = false;

	Mutex x_knownBlocks;
	h256Hash m_knownBlocks;					
	Mutex x_knownTransactions;
	h256Hash m_knownTransactions;			
	unsigned m_unknownNewBlocks = 0;		
	unsigned m_lastAskedHeaders = 0;		

	std::shared_ptr<EthereumPeerObserverFace> m_observer;
	std::shared_ptr<EthereumHostDataFace> m_hostData;
};

}
}
