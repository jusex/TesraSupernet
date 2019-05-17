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
/** @file VM.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <unordered_map>
#include <libdevcore/Exceptions.h>
#include <libethcore/Common.h>
#include <libevmcore/Instruction.h>
#include <libdevcore/SHA3.h>
#include <libethcore/BlockHeader.h>
#include "VMFace.h"

namespace dev
{
namespace eth
{



inline Address asAddress(u256 _item)
{
	return right160(h256(_item));
}

inline u256 fromAddress(Address _a)
{
	return (u160)_a;
}


struct InstructionMetric
{
	Tier gasPriceTier;
	int args;
	int ret;
};


/**
 */
class VM: public VMFace
{
public:
	virtual owning_bytes_ref exec(u256& io_gas, ExtVMFace& _ext, OnOpFunc const& _onOp) override final;

#if EVM_JUMPS_AND_SUBS
	
	void validate(ExtVMFace& _ext);
	void validateSubroutine(uint64_t _PC, uint64_t* _RP, u256* _SP);
#endif

	bytes const& memory() const { return m_mem; }
	u256s stack() const { assert(m_stack <= m_SP + 1); return u256s(m_stack, m_SP + 1); };

private:

	u256* io_gas = 0;
	uint64_t m_io_gas = 0;
	ExtVMFace* m_ext = 0;
	OnOpFunc m_onOp;

	static std::array<InstructionMetric, 256> c_metrics;
	static void initMetrics();
	static u256 exp256(u256 _base, u256 _exponent);
	void copyCode(int);
	const void* const* c_jumpTable = 0;
	bool m_caseInit = false;
	
	typedef void (VM::*MemFnPtr)();
	MemFnPtr m_bounce = 0;
	MemFnPtr m_onFail = 0;
	uint64_t m_nSteps = 0;
	EVMSchedule const* m_schedule = nullptr;

	
	owning_bytes_ref m_output;

	
	bytes m_mem;

	
	bytes m_codeSpace;
	byte* m_code = nullptr;

	
	u256 m_stackSpace[1025];
	u256* m_stack = m_stackSpace + 1;
	ptrdiff_t stackSize() { return m_SP - m_stack; }
	
#if EVM_JUMPS_AND_SUBS
	
	uint64_t m_returnSpace[1025];
	uint64_t* m_return = m_returnSpace + 1;
	
	
	std::vector<size_t> m_frameSize;
#endif

	
	u256 m_pool[256];

	
	Instruction m_OP;                   
	uint64_t    m_PC = 0;               
	u256*       m_SP = m_stack - 1;     
#if EVM_JUMPS_AND_SUBS
	uint64_t*   m_RP = m_return - 1;    
#endif

	
	uint64_t m_runGas = 0;
	uint64_t m_newMemSize = 0;
	uint64_t m_copyMemSize = 0;

	
	void initEntry();
	void optimize();

	
	void interpretCases();

	
	void caseCreate();
	bool caseCallSetup(CallParameters*, bytesRef& o_output);
	void caseCall();

	void copyDataToMemory(bytesConstRef _data, u256*& m_SP);
	uint64_t memNeed(u256 _offset, u256 _size);

	void throwOutOfGas();
	void throwBadInstruction();
	void throwBadJumpDestination();
	void throwBadStack(unsigned _size, unsigned _n, unsigned _d);

	void reportStackUse();

	std::vector<uint64_t> m_beginSubs;
	std::vector<uint64_t> m_jumpDests;
	int64_t verifyJumpDest(u256 const& _dest, bool _throw = true);

	int poolConstant(const u256&);

	void onOperation();
	void checkStack(unsigned _n, unsigned _d);
	uint64_t gasForMem(u512 _size);
	void updateIOGas();
	void updateGas();
	void updateMem();
	void logGasMem();
	void fetchInstruction();
	
	uint64_t decodeJumpDest(const byte* const _code, uint64_t& _pc);
	uint64_t decodeJumpvDest(const byte* const _code, uint64_t& _pc, u256*& _sp);

	template<class T> uint64_t toInt63(T v)
	{
		
		if (v > 0x7FFFFFFFFFFFFFFF)
			throwOutOfGas();
		uint64_t w = uint64_t(v);
		return w;
	}
	};

}
}
