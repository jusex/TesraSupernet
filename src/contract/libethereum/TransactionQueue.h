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
/** @file TransactionQueue.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <functional>
#include <condition_variable>
#include <thread>
#include <deque>
#include <libdevcore/Common.h>
#include <libdevcore/Guards.h>
#include <libdevcore/Log.h>
#include <libethcore/Common.h>
#include "Transaction.h"

namespace dev
{
namespace eth
{

struct TransactionQueueChannel: public LogChannel { static const char* name(); static const int verbosity = 4; };
struct TransactionQueueTraceChannel: public LogChannel { static const char* name(); static const int verbosity = 7; };
#define ctxq dev::LogOutputStream<dev::eth::TransactionQueueTraceChannel, true>()

/**
 * @brief A queue of Transactions, each stored as RLP.
 * Maintains a transaction queue sorted by nonce diff and gas price.
 * @threadsafe
 */
class TransactionQueue
{
public:
	struct Limits { size_t current; size_t future; };

	
	
	
	TransactionQueue(unsigned _limit = 1024, unsigned _futureLimit = 1024);
	TransactionQueue(Limits const& _l): TransactionQueue(_l.current, _l.future) {}
	~TransactionQueue();
	
	
	
	void enqueue(RLP const& _data, h512 const& _nodeId);

	
	
	
	
	ImportResult import(bytes const& _tx, IfDropped _ik = IfDropped::Ignore) { return import(&_tx, _ik); }

	
	
	
	
	ImportResult import(Transaction const& _tx, IfDropped _ik = IfDropped::Ignore);

	
	
	void drop(h256 const& _txHash);

	
	
	unsigned waiting(Address const& _a) const;

	
	
	
	
	Transactions topTransactions(unsigned _limit, h256Hash const& _avoid = h256Hash()) const;

	
	
	h256Hash knownTransactions() const;

	
	
	u256 maxNonce(Address const& _a) const;

	
	
	void setFuture(h256 const& _t);

	
	
	void dropGood(Transaction const& _t);

	struct Status
	{
		size_t current;
		size_t future;
		size_t unverified;
		size_t dropped;
	};
	
	Status status() const { Status ret; DEV_GUARDED(x_queue) { ret.unverified = m_unverified.size(); } ReadGuard l(m_lock); ret.dropped = m_dropped.size(); ret.current = m_currentByHash.size(); ret.future = m_future.size(); return ret; }

	
	Limits limits() const { return Limits{m_limit, m_futureLimit}; }

	
	void clear();

	
	template <class T> Handler<> onReady(T const& _t) { return m_onReady.add(_t); }

	
	template <class T> Handler<ImportResult, h256 const&, h512 const&> onImport(T const& _t) { return m_onImport.add(_t); }

	
	template <class T> Handler<h256 const&> onReplaced(T const& _t) { return m_onReplaced.add(_t); }

private:

	
	struct VerifiedTransaction
	{
		VerifiedTransaction(Transaction const& _t): transaction(_t) {}
		VerifiedTransaction(VerifiedTransaction&& _t): transaction(std::move(_t.transaction)) {}

		VerifiedTransaction(VerifiedTransaction const&) = delete;
		VerifiedTransaction& operator=(VerifiedTransaction const&) = delete;

		Transaction transaction; 
	};

	
	struct UnverifiedTransaction
	{
		UnverifiedTransaction() {}
		UnverifiedTransaction(bytesConstRef const& _t, h512 const& _nodeId): transaction(_t.toBytes()), nodeId(_nodeId) {}
		UnverifiedTransaction(UnverifiedTransaction&& _t): transaction(std::move(_t.transaction)), nodeId(std::move(_t.nodeId)) {}
		UnverifiedTransaction& operator=(UnverifiedTransaction&& _other)
		{
			assert(&_other != this);

			transaction = std::move(_other.transaction);
			nodeId = std::move(_other.nodeId);
			return *this;
		}

		UnverifiedTransaction(UnverifiedTransaction const&) = delete;
		UnverifiedTransaction& operator=(UnverifiedTransaction const&) = delete;

		bytes transaction;	
		h512 nodeId;		
	};

	struct PriorityCompare
	{
		TransactionQueue& queue;
		
		bool operator()(VerifiedTransaction const& _first, VerifiedTransaction const& _second) const
		{
			u256 const& height1 = _first.transaction.nonce() - queue.m_currentByAddressAndNonce[_first.transaction.sender()].begin()->first;
			u256 const& height2 = _second.transaction.nonce() - queue.m_currentByAddressAndNonce[_second.transaction.sender()].begin()->first;
			return height1 < height2 || (height1 == height2 && _first.transaction.gasPrice() > _second.transaction.gasPrice());
		}
	};

	
	using PriorityQueue = std::multiset<VerifiedTransaction, PriorityCompare>;

	ImportResult import(bytesConstRef _tx, IfDropped _ik = IfDropped::Ignore);
	ImportResult check_WITH_LOCK(h256 const& _h, IfDropped _ik);
	ImportResult manageImport_WITH_LOCK(h256 const& _h, Transaction const& _transaction);

	void insertCurrent_WITH_LOCK(std::pair<h256, Transaction> const& _p);
	void makeCurrent_WITH_LOCK(Transaction const& _t);
	bool remove_WITH_LOCK(h256 const& _txHash);
	u256 maxNonce_WITH_LOCK(Address const& _a) const;
	void verifierBody();

	mutable SharedMutex m_lock;													
	h256Hash m_known;															

	std::unordered_map<h256, std::function<void(ImportResult)>> m_callbacks;	
	h256Hash m_dropped;															

	PriorityQueue m_current;
	std::unordered_map<h256, PriorityQueue::iterator> m_currentByHash;			
	std::unordered_map<Address, std::map<u256, PriorityQueue::iterator>> m_currentByAddressAndNonce; 
	std::unordered_map<Address, std::map<u256, VerifiedTransaction>> m_future;	

	Signal<> m_onReady;															
	Signal<ImportResult, h256 const&, h512 const&> m_onImport;					
	Signal<h256 const&> m_onReplaced;											
	unsigned m_limit;															
	unsigned m_futureLimit;														
	unsigned m_futureSize = 0;													

	std::condition_variable m_queueReady;										
	std::vector<std::thread> m_verifiers;
	std::deque<UnverifiedTransaction> m_unverified;								
	mutable Mutex x_queue;														
	std::atomic<bool> m_aborting = {false};										
};

}
}

