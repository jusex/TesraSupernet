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
/** @file BlockChainSync.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <unordered_map>

#include <libdevcore/Guards.h>
#include <libethcore/Common.h>
#include <libethcore/BlockHeader.h>
#include <libp2p/Common.h>
#include "CommonNet.h"

namespace dev
{

class RLPStream;

namespace eth
{

class EthereumHost;
class BlockQueue;
class EthereumPeer;

/**
 * @brief Base BlockChain synchronization strategy class.
 * Syncs to peers and keeps up to date. Base class handles blocks downloading but does not contain any details on state transfer logic.
 */
class BlockChainSync: public HasInvariants
{
public:
	BlockChainSync(EthereumHost& _host);
	~BlockChainSync();
	void abortSync(); 

	
	bool isSyncing() const;

	
	void restartSync();

	
	
	void completeSync();

	
	void onPeerStatus(std::shared_ptr<EthereumPeer> _peer);

	
	void onPeerBlockHeaders(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);

	
	void onPeerBlockBodies(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);

	
	void onPeerNewBlock(std::shared_ptr<EthereumPeer> _peer, RLP const& _r);

	void onPeerNewHashes(std::shared_ptr<EthereumPeer> _peer, std::vector<std::pair<h256, u256>> const& _hashes);

	
	void onPeerAborting();

	
	void onBlockImported(BlockHeader const& _info);

	
	SyncStatus status() const;

	static char const* stateName(SyncState _s) { return s_stateNames[static_cast<int>(_s)]; }

private:
	
	void continueSync();

	
	void pauseSync();

	EthereumHost& host() { return m_host; }
	EthereumHost const& host() const { return m_host; }

	void resetSync();
	void syncPeer(std::shared_ptr<EthereumPeer> _peer, bool _force);
	void requestBlocks(std::shared_ptr<EthereumPeer> _peer);
	void clearPeerDownload(std::shared_ptr<EthereumPeer> _peer);
	void clearPeerDownload();
	void collectBlocks();

private:
	struct Header
	{
		bytes data;		
		h256 hash;		
		h256 parent;	
	};

	
	struct HeaderId
	{
		h256 transactionsRoot;
		h256 uncles;

		bool operator==(HeaderId const& _other) const
		{
			return transactionsRoot == _other.transactionsRoot && uncles == _other.uncles;
		}
	};

	struct HeaderIdHash
	{
		std::size_t operator()(const HeaderId& _k) const
		{
			size_t seed = 0;
			h256::hash hasher;
			boost::hash_combine(seed, hasher(_k.transactionsRoot));
			boost::hash_combine(seed, hasher(_k.uncles));
			return seed;
		}
	};

	EthereumHost& m_host;
	Handler<> m_bqRoomAvailable;				
	mutable RecursiveMutex x_sync;
	SyncState m_state = SyncState::Idle;		
	h256Hash m_knownNewHashes; 					
	unsigned m_startingBlock = 0;      	    	
	unsigned m_highestBlock = 0;       	     	
	std::unordered_set<unsigned> m_downloadingHeaders;		
	std::unordered_set<unsigned> m_downloadingBodies;		
	std::map<unsigned, std::vector<Header>> m_headers;	    
	std::map<unsigned, std::vector<bytes>> m_bodies;	    
	std::map<std::weak_ptr<EthereumPeer>, std::vector<unsigned>, std::owner_less<std::weak_ptr<EthereumPeer>>> m_headerSyncPeers; 
	std::map<std::weak_ptr<EthereumPeer>, std::vector<unsigned>, std::owner_less<std::weak_ptr<EthereumPeer>>> m_bodySyncPeers; 
	std::unordered_map<HeaderId, unsigned, HeaderIdHash> m_headerIdToNumber;
	bool m_haveCommonHeader = false;			
	unsigned m_lastImportedBlock = 0; 			
	h256 m_lastImportedBlockHash;				
	u256 m_syncingTotalDifficulty;				

private:
	static char const* const s_stateNames[static_cast<int>(SyncState::Size)];
	bool invariants() const override;
	void logNewBlock(h256 const& _h);
};

std::ostream& operator<<(std::ostream& _out, SyncStatus const& _sync);

}
}
