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
/** @file Executive.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <functional>
#ifndef TESRA_BUILD
#include <json/json.h>
#endif
#include <libdevcore/Log.h>
#include <libevmcore/Instruction.h>
#include <libethcore/Common.h>
#include <libevm/VMFace.h>
#include "Transaction.h"

namespace Json
{
	class Value;
}

namespace dev
{

class OverlayDB;

namespace eth
{

class State;
class Block;
class BlockChain;
class ExtVM;
class SealEngineFace;
struct Manifest;

struct VMTraceChannel: public LogChannel { static const char* name(); static const int verbosity = 11; };
struct ExecutiveWarnChannel: public LogChannel { static const char* name(); static const int verbosity = 1; };

class StandardTrace
{
public:
	struct DebugOptions
	{
		bool disableStorage = false;
		bool disableMemory = false;
		bool disableStack = false;
		bool fullStorage = false;
	};

	StandardTrace();
	void operator()(uint64_t _steps, uint64_t _PC, Instruction _inst, bigint _newMemSize, bigint _gasCost, bigint _gas, VM* _vm, ExtVMFace const* _extVM);

	void setShowMnemonics() { m_showMnemonics = true; }
	void setOptions(DebugOptions _options) { m_options = _options; }

	std::string json(bool _styled = false) const;

	OnOpFunc onOp() { return [=](uint64_t _steps, uint64_t _PC, Instruction _inst, bigint _newMemSize, bigint _gasCost, bigint _gas, VM* _vm, ExtVMFace const* _extVM) { (*this)(_steps, _PC, _inst, _newMemSize, _gasCost, _gas, _vm, _extVM); }; }

private:
	bool m_showMnemonics = false;
	std::vector<Instruction> m_lastInst;
	bytes m_lastCallData;
#ifndef TESRA_BUILD
	Json::Value m_trace;
#endif
	DebugOptions m_options;
};


/**
 * @brief Message-call/contract-creation executor; useful for executing transactions.
 *
 * Two ways of using this class - either as a transaction executive or a CALL/CREATE executive.
 *
 * In the first use, after construction, begin with initialize(), then execute() and end with finalize(). Call go()
 * after execute() only if it returns false.
 *
 * In the second use, after construction, begin with call() or create() and end with
 * accrueSubState(). Call go() after call()/create() only if it returns false.
 *
 * Example:
 * @code
 * Executive e(state, blockchain, 0);
 * e.initialize(transaction);
 * if (!e.execute())
 *    e.go();
 * e.finalize();
 * @endcode
 */
class Executive
{
public:
	
	Executive(State& _s, EnvInfo const& _envInfo, SealEngineFace const& _sealEngine, unsigned _level = 0): m_s(_s), m_envInfo(_envInfo), m_depth(_level), m_sealEngine(_sealEngine) {}

	/** Easiest constructor.
	 * Creates executive to operate on the state of end of the given block, populating environment
	 * info from given Block and the LastHashes portion from the BlockChain.
	 */
	Executive(Block& _s, BlockChain const& _bc, unsigned _level = 0);

	/** LastHashes-split constructor.
	 * Creates executive to operate on the state of end of the given block, populating environment
	 * info accordingly, with last hashes given explicitly.
	 */
	Executive(Block& _s, LastHashes const& _lh = LastHashes(), unsigned _level = 0);

	/** Previous-state constructor.
	 * Creates executive to operate on the state of a particular transaction in the given block,
	 * populating environment info from the given Block and the LastHashes portion from the BlockChain.
	 * State is assigned the resultant value, but otherwise unused.
	 */
	Executive(State& _s, Block const& _block, unsigned _txIndex, BlockChain const& _bc, unsigned _level = 0);

	Executive(Executive const&) = delete;
	void operator=(Executive) = delete;

	
	void initialize(bytesConstRef _transaction) { initialize(Transaction(_transaction, CheckTransaction::None)); }
	void initialize(Transaction const& _transaction);
	
	
	void finalize();
	
	
	bool execute();
	
	
	Transaction const& t() const { return m_t; }
	
	
	LogEntries const& logs() const { return m_logs; }
	
	
	u256 gasUsed() const;

	owning_bytes_ref takeOutput() { return std::move(m_output); }

	
	
	bool create(Address _txSender, u256 _endowment, u256 _gasPrice, u256 _gas, bytesConstRef _code, Address _originAddress);
	
	
	bool call(Address _receiveAddress, Address _txSender, u256 _txValue, u256 _gasPrice, bytesConstRef _txData, u256 _gas);
	bool call(CallParameters const& _cp, u256 const& _gasPrice, Address const& _origin);
	
	void accrueSubState(SubState& _parentContext);

	
	
	bool go(OnOpFunc const& _onOp = OnOpFunc());

	
	static OnOpFunc simpleTrace();

	
	static OnOpFunc standardTrace(std::ostream& o_output);

	
	u256 gas() const { return m_gas; }

	
	Address newAddress() const { return m_newAddress; }
	
	bool excepted() const { return m_excepted != TransactionException::None; }

	
	void setResultRecipient(ExecutionResult& _res) { m_res = &_res; }

	
	void revert();

private:
	State& m_s;							
	
	EnvInfo m_envInfo;					
	std::shared_ptr<ExtVM> m_ext;		
	owning_bytes_ref m_output;			
	ExecutionResult* m_res = nullptr;	

	unsigned m_depth = 0;				
	TransactionException m_excepted = TransactionException::None;	
	int64_t m_baseGasRequired;			
	u256 m_gas = 0;						
	u256 m_refunded = 0;				

	Transaction m_t;					
	LogEntries m_logs;					

	u256 m_gasCost;
	SealEngineFace const& m_sealEngine;

	bool m_isCreation = false;
	Address m_newAddress;
	size_t m_savepoint = 0;
};

}
}
