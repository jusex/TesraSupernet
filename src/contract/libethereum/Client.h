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
/** @file Client.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <list>
#include <queue>
#include <atomic>
#include <string>
#include <array>
#include <libdevcore/Common.h>
#include <libdevcore/CommonIO.h>
#include <libdevcore/Guards.h>
#include <libdevcore/Worker.h>
#include <libethcore/SealEngine.h>
#include <libethcore/ABI.h>
#include <libp2p/Common.h>
#include "BlockChain.h"
#include "Block.h"
#include "CommonNet.h"
#include "ClientBase.h"

namespace dev
{
namespace eth
{

class Client;
class DownloadMan;

enum ClientWorkState
{
	Active = 0,
	Deleting,
	Deleted
};

struct ClientNote: public LogChannel { static const char* name(); static const int verbosity = 2; };
struct ClientChat: public LogChannel { static const char* name(); static const int verbosity = 4; };
struct ClientTrace: public LogChannel { static const char* name(); static const int verbosity = 7; };
struct ClientDetail: public LogChannel { static const char* name(); static const int verbosity = 14; };

struct ActivityReport
{
	unsigned ticks = 0;
	std::chrono::system_clock::time_point since = std::chrono::system_clock::now();
};

std::ostream& operator<<(std::ostream& _out, ActivityReport const& _r);

/**
 * @brief Main API hub for interfacing with Ethereum.
 */
class Client: public ClientBase, protected Worker
{
public:
	Client(
		ChainParams const& _params,
		int _networkID,
		p2p::Host* _host,
		std::shared_ptr<GasPricer> _gpForAdoption,
		std::string const& _dbPath = std::string(),
		WithExisting _forceAction = WithExisting::Trust,
		TransactionQueue::Limits const& _l = TransactionQueue::Limits{1024, 1024}
	);
	
	virtual ~Client();

	
	ChainParams const& chainParams() const { return bc().chainParams(); }

	
	void setGasPricer(std::shared_ptr<GasPricer> _gp) { m_gp = _gp; }
	std::shared_ptr<GasPricer> gasPricer() const { return m_gp; }

	
	virtual void flushTransactions() override;

	
	ImportResult queueBlock(bytes const& _block, bool _isSafe = false);

	using Interface::call; 
	
	ExecutionResult call(Address _dest, bytes const& _data = bytes(), u256 _gas = 125000, u256 _value = 0, u256 _gasPrice = 1 * ether, Address const& _from = Address());

	
	virtual u256 gasLimitRemaining() const override { return m_postSeal.gasLimitRemaining(); }
	
	virtual u256 gasBidPrice() const override { return m_gp->bid(); }

	
	
	dev::eth::Block block(h256 const& _blockHash, PopulationStatistics* o_stats) const;
	
	
	dev::eth::State state(unsigned _txi, h256 const& _block) const;
	
	
	dev::eth::State state(unsigned _txi) const;

	
	dev::eth::Block postState() const { ReadGuard l(x_postSeal); return m_postSeal; }
	
	BlockChain const& blockChain() const { return bc(); }
	
	BlockQueueStatus blockQueueStatus() const { return m_bq.status(); }
	
	SyncStatus syncStatus() const override;
	
	BlockQueue const& blockQueue() const { return m_bq; }
	
	OverlayDB const& stateDB() const { return m_stateDB; }
	
	TransactionQueue::Status transactionQueueStatus() const { return m_tq.status(); }
	TransactionQueue::Limits transactionQueueLimits() const { return m_tq.limits(); }

	
	std::tuple<ImportRoute, bool, unsigned> syncQueue(unsigned _max = 1);

	
	

	virtual Address author() const override { ReadGuard l(x_preSeal); return m_preSeal.author(); }
	virtual void setAuthor(Address const& _us) override { WriteGuard l(x_preSeal); m_preSeal.setAuthor(_us); }

	
	strings sealers() const { return sealEngine()->sealers(); }
	
	std::string sealer() const { return sealEngine()->sealer(); }
	
	void setSealer(std::string const& _id) { sealEngine()->setSealer(_id); if (wouldSeal()) startSealing(); }
	
	bytes sealOption(std::string const& _name) const { return sealEngine()->option(_name); }
	
	bool setSealOption(std::string const& _name, bytes const& _value) { auto ret = sealEngine()->setOption(_name, _value); if (wouldSeal()) startSealing(); return ret; }

	
	void startSealing() override;
	
	void stopSealing() override { m_wouldSeal = false; }
	
	bool wouldSeal() const override { return m_wouldSeal; }

	
	bool isSyncing() const override;
	
	bool isMajorSyncing() const override;

	
	u256 networkId() const override;
	
	void setNetworkId(u256 const& _n) override;

	
	SealEngineFace* sealEngine() const override { return bc().sealEngine(); }

	

	DownloadMan const* downloadMan() const;
	
	void clearPending();
	
	void killChain() { reopenChain(WithExisting::Kill); }
	
	void reopenChain(ChainParams const& _p, WithExisting _we = WithExisting::Trust);
	void reopenChain(WithExisting _we);
	
	void retryUnknown() { m_bq.retryAllUnknown(); }
	
	ActivityReport activityReport() { ActivityReport ret; std::swap(m_report, ret); return ret; }
	
	void setExtraData(bytes const& _extraData) { m_extraData = _extraData; }
	
	void rewind(unsigned _n);
	
	void rescue() { bc().rescue(m_stateDB); }

	
	void executeInMainThread(std::function<void()> const& _function);

	virtual Block block(h256 const& _block) const override;
	using ClientBase::block;

protected:
	
	
	void init(p2p::Host* _extNet, std::string const& _dbPath, WithExisting _forceAction, u256 _networkId);

	
	BlockChain& bc() override { return m_bc; }
	BlockChain const& bc() const override { return m_bc; }

	
	
	virtual Block preSeal() const override { ReadGuard l(x_preSeal); return m_preSeal; }
	virtual Block postSeal() const override { ReadGuard l(x_postSeal); return m_postSeal; }
	virtual void prepareForTransaction() override;

	
	
	void appendFromNewPending(TransactionReceipt const& _receipt, h256Hash& io_changed, h256 _sha3);

	
	
	void appendFromBlock(h256 const& _blockHash, BlockPolarity _polarity, h256Hash& io_changed);

	
	
	void noteChanged(h256Hash const& _filters);

	
	virtual bool submitSealed(bytes const& _s);

protected:
	
	void startedWorking() override;

	
	void doWork(bool _doWait);
	void doWork() override { doWork(true); }

	
	void doneWorking() override;

	
	void rejigSealing();

	
	void onDeadBlocks(h256s const& _blocks, h256Hash& io_changed);

	
	virtual void onNewBlocks(h256s const& _blocks, h256Hash& io_changed);

	
	void resyncStateFromChain();

	
	void resetState();

	
	
	void onChainChanged(ImportRoute const& _ir);

	
	void syncBlockQueue();

	
	void syncTransactionQueue();

	
	void onTransactionQueueReady() { m_syncTransactionQueue = true; m_signalled.notify_all(); }

	
	void onBlockQueueReady() { m_syncBlockQueue = true; m_signalled.notify_all(); }

	
	
	void onPostStateChanged();

	
	void checkWatchGarbage();

	
	void tick();

	
	
	void onBadBlock(Exception& _ex) const;

	
	void callQueuedFunctions();

	BlockChain m_bc;						
	BlockQueue m_bq;						
	std::shared_ptr<GasPricer> m_gp;		

	OverlayDB m_stateDB;					
	mutable SharedMutex x_preSeal;			
	Block m_preSeal;						
	mutable SharedMutex x_postSeal;			
	Block m_postSeal;						
	mutable SharedMutex x_working;			
	Block m_working;						
	BlockHeader m_sealingInfo;				
	bool remoteActive() const;				
	bool m_remoteWorking = false;			
	std::atomic<bool> m_needStateReset = { false };			
	std::chrono::system_clock::time_point m_lastGetWork;	

	std::weak_ptr<EthereumHost> m_host;		

	Handler<> m_tqReady;
	Handler<h256 const&> m_tqReplaced;
	Handler<> m_bqReady;

	bool m_wouldSeal = false;				
	bool m_wouldButShouldnot = false;		

	mutable std::chrono::system_clock::time_point m_lastGarbageCollection;
											
	mutable std::chrono::system_clock::time_point m_lastTick = std::chrono::system_clock::now();
											

	unsigned m_syncAmount = 50;				

	ActivityReport m_report;

	SharedMutex x_functionQueue;
	std::queue<std::function<void()>> m_functionQueue;	

	std::condition_variable m_signalled;
	Mutex x_signalled;
	std::atomic<bool> m_syncTransactionQueue = {false};
	std::atomic<bool> m_syncBlockQueue = {false};

	bytes m_extraData;
};

}
}
