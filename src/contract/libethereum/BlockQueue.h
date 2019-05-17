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
/** @file BlockQueue.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <condition_variable>
#include <thread>
#include <deque>
#include <boost/thread.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/Log.h>
#include <libethcore/Common.h>
#include <libdevcore/Guards.h>
#include <libethcore/Common.h>
#include <libethcore/BlockHeader.h>
#include "VerifiedBlock.h"

namespace dev
{

namespace eth
{

class BlockChain;

struct BlockQueueChannel: public LogChannel { static const char* name(); static const int verbosity = 4; };
struct BlockQueueTraceChannel: public LogChannel { static const char* name(); static const int verbosity = 7; };
#define cblockq dev::LogOutputStream<dev::eth::BlockQueueChannel, true>()

struct BlockQueueStatus
{
	size_t importing;
	size_t verified;
	size_t verifying;
	size_t unverified;
	size_t future;
	size_t unknown;
	size_t bad;
};

enum class QueueStatus
{
	Ready,
	Importing,
	UnknownParent,
	Bad,
	Unknown
};

std::ostream& operator<< (std::ostream& os, QueueStatus const& obj);

template<class T>
class SizedBlockQueue
{
public:
	std::size_t count() const { return m_queue.size(); }

	std::size_t size() const { return m_size; }
	
	bool isEmpty() const { return m_queue.empty(); }

	h256 nextHash() const { return m_queue.front().verified.info.sha3Uncles(); }

	T const& next() const { return m_queue.front(); }

	void clear()
	{
		m_queue.clear();
		m_size = 0;
	}

	void enqueue(T&& _t)
	{
		m_queue.emplace_back(std::move(_t));
		m_size += m_queue.back().blockData.size();
	}

	T dequeue()
	{
		T t;
		std::swap(t, m_queue.front());
		m_queue.pop_front();
		m_size -= t.blockData.size();

		return t;
	}

	std::vector<T> dequeueMultiple(std::size_t _n)
	{
		return removeRange(m_queue.begin(), m_queue.begin() + _n);
	}

	bool remove(h256 const& _hash)
	{
		std::vector<T> removed = removeIf(sha3UnclesEquals(_hash));
		return !removed.empty();
	}

	template<class Pred>
	std::vector<T> removeIf(Pred _pred)
	{
		auto const removedBegin = std::remove_if(m_queue.begin(), m_queue.end(), _pred);

		return removeRange(removedBegin, m_queue.end());
	}

	bool replace(h256 const& _hash, T&& _t)
	{
		auto const it = std::find_if(m_queue.begin(), m_queue.end(), sha3UnclesEquals(_hash));

		if (it == m_queue.end())
			return false;

		m_size -= it->blockData.size();
		m_size += _t.blockData.size();
		*it = std::move(_t);

		return true;
	}

private:
	static std::function<bool(T const&)> sha3UnclesEquals(h256 const& _hash)
	{
		return [&_hash](T const& _t) { return _t.verified.info.sha3Uncles() == _hash; };
	}

	std::vector<T> removeRange(typename std::deque<T>::iterator _begin, typename std::deque<T>::iterator _end)
	{
		std::vector<T> ret(std::make_move_iterator(_begin), std::make_move_iterator(_end));

		for (auto it = ret.begin(); it != ret.end(); ++it)
			m_size -= it->blockData.size();

		m_queue.erase(_begin, _end);
		return ret;
	}

	std::deque<T> m_queue;
	std::atomic<size_t> m_size = {0};	
};

template<class KeyType>
class SizedBlockMap
{
public:
	std::size_t count() const { return m_map.size(); }

	std::size_t size() const { return m_size; }

	bool isEmpty() const { return m_map.empty(); }

	KeyType firstKey() const { return m_map.begin()->first; }

	void clear()
	{
		m_map.clear();
		m_size = 0;
	}

	void insert(KeyType const& _key, h256 const& _hash, bytes&& _blockData)
	{
		auto hashAndBlock = std::make_pair(_hash, std::move(_blockData));
		auto keyAndValue = std::make_pair(_key, std::move(hashAndBlock));
		m_map.insert(std::move(keyAndValue));
		m_size += _blockData.size();
	}

	std::vector<std::pair<h256, bytes>> removeByKeyEqual(KeyType const& _key)
	{
		auto const equalRange = m_map.equal_range(_key);
		return removeRange(equalRange.first, equalRange.second);
	}

	std::vector<std::pair<h256, bytes>> removeByKeyNotGreater(KeyType const& _key)
	{
		return removeRange(m_map.begin(), m_map.upper_bound(_key));
	}

private:
	using BlockMultimap = std::multimap<KeyType, std::pair<h256, bytes>>;

	std::vector<std::pair<h256, bytes>> removeRange(typename BlockMultimap::iterator _begin, typename BlockMultimap::iterator _end)
	{
		std::vector<std::pair<h256, bytes>> removed;
		std::size_t removedSize = 0;
		for (auto it = _begin; it != _end; ++it)
		{
			removed.push_back(std::move(it->second));
			removedSize += removed.back().second.size();
		}

		m_size -= removedSize;
		m_map.erase(_begin, _end);

		return removed;
	}
		
	BlockMultimap m_map;
	std::atomic<size_t> m_size = {0};	
};

/**
 * @brief A queue of blocks. Sits between network or other I/O and the BlockChain.
 * Sorts them ready for blockchain insertion (with the BlockChain::sync() method).
 * @threadsafe
 */
class BlockQueue: HasInvariants
{
public:
	BlockQueue();
	~BlockQueue();

	void setChain(BlockChain const& _bc) { m_bc = &_bc; }

	
	ImportResult import(bytesConstRef _block, bool _isOurs = false);

	
	void tick();

	
	
	void drain(std::vector<VerifiedBlock>& o_out, unsigned _max);

	
	
	bool doneDrain(h256s const& _knownBad = h256s());

	
	void noteReady(h256 const& _b) { WriteGuard l(m_lock); noteReady_WITH_LOCK(_b); }

	
	void retryAllUnknown();

	
	std::pair<unsigned, unsigned> items() const { ReadGuard l(m_lock); return std::make_pair(m_readySet.size(), m_unknownSet.size()); }

	
	void clear();

	
	void stop();

	
	h256 firstUnknown() const { ReadGuard l(m_lock); return m_unknownSet.size() ? *m_unknownSet.begin() : h256(); }

	
	BlockQueueStatus status() const;

	
	QueueStatus blockStatus(h256 const& _h) const;

	template <class T> Handler<> onReady(T const& _t) { return m_onReady.add(_t); }
	template <class T> Handler<> onRoomAvailable(T const& _t) { return m_onRoomAvailable.add(_t); }

	template <class T> void setOnBad(T const& _t) { m_onBad = _t; }

	bool knownFull() const;
	bool unknownFull() const;
	u256 difficulty() const;	
	bool isActive() const;

private:
	struct UnverifiedBlock
	{
		h256 hash;
		h256 parentHash;
		bytes blockData;
	};

	void noteReady_WITH_LOCK(h256 const& _b);

	bool invariants() const override;

	void verifierBody();
	void collectUnknownBad_WITH_BOTH_LOCKS(h256 const& _bad);
	void updateBad_WITH_LOCK(h256 const& _bad);
	void drainVerified_WITH_BOTH_LOCKS();

	std::size_t knownSize() const;
	std::size_t knownCount() const;
	std::size_t unknownSize() const;
	std::size_t unknownCount() const;

	BlockChain const* m_bc;												

	mutable boost::shared_mutex m_lock;									
	h256Hash m_drainingSet;												
	h256Hash m_readySet;												
	h256Hash m_unknownSet;												
	SizedBlockMap<h256> m_unknown;										
	h256Hash m_knownBad;												
	SizedBlockMap<time_t> m_future;										
	Signal<> m_onReady;													
	Signal<> m_onRoomAvailable;											

	mutable Mutex m_verification;										
	std::condition_variable m_moreToVerify;								
	SizedBlockQueue<VerifiedBlock> m_verified;								
	SizedBlockQueue<VerifiedBlock> m_verifying;								
	SizedBlockQueue<UnverifiedBlock> m_unverified;							

	std::vector<std::thread> m_verifiers;								
	std::atomic<bool> m_deleting = {false};								

	std::function<void(Exception&)> m_onBad;							
	u256 m_difficulty;													
	u256 m_drainingDifficulty;											
};

std::ostream& operator<<(std::ostream& _out, BlockQueueStatus const& _s);

}
}
