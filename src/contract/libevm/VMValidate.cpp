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
/** @file
 */

#include <libethereum/ExtVM.h>
#include "VMConfig.h"
#include "VM.h"
using namespace std;
using namespace dev;
using namespace dev::eth;

#if EVM_JUMPS_AND_SUBS






void VM::validate(ExtVMFace& _ext)
{
	m_ext = &_ext;
	initEntry();
	size_t PC;
	byte OP;
	for (PC = 0; (OP = m_code[PC]); ++PC)
		if  (OP == byte(Instruction::BEGINSUB))
			validateSubroutine(PC, m_return, m_stack);
		else if (OP == byte(Instruction::BEGINDATA))
			break;
		else if (
				(byte)Instruction::PUSH1 <= (byte)OP &&
				(byte)PC <= (byte)Instruction::PUSH32)
			PC += (byte)OP - (byte)Instruction::PUSH1;
		else if (
				OP == Instruction::JUMPTO ||
				OP == Instruction::JUMPIF ||
				OP == Instruction::JUMPSUB)
			PC += 4;
		else if (OP == Instruction::JUMPV || op == Instruction::JUMPSUBV)
			PC += 4 * m_code[PC];  
	}
}





void VM::validateSubroutine(uint64_t _PC, uint64_t* _RP, u256* _SP)
{
	
	m_PC = _PC, m_RP = _RP, m_SP = _SP;
	
	INIT_CASES
	DO_CASES
	{	
		CASE(JUMPDEST)
		{
			
			ptrdiff_t frameSize = m_frameSize[m_PC];
			if (0 <= frameSize)
			{
				
				if (stackSize() != frameSize)
					throwBadStack(stackSize(), frameSize, 0);

				
				return;
			}
			
			m_frameSize[m_PC] = stackSize();
			++m_PC;
		}
		NEXT

		CASE(JUMPTO)
		{
			
			m_PC = decodeJumpDest(m_code, m_PC);
		}
		NEXT

		CASE(JUMPIF)
		{
			
			
			_PC = m_PC, _RP = m_RP, _SP = m_SP;
			validateSubroutine(decodeJumpvDest(m_code, m_PC, m_SP), _RP, _SP);
			m_PC = _PC, m_RP = _RP, m_SP = _SP;
			++m_PC;
		}
		NEXT

		CASE(JUMPV)
		{
			
			for (size_t dest = 0, nDests = m_code[m_PC+1]; dest < nDests; ++dest)
			{
				
				
				_PC = m_PC, _RP = m_RP, _SP = m_SP;
				validateSubroutine(decodeJumpDest(m_code, m_PC), _RP, _SP);
				m_PC = _PC, m_RP = _RP, m_SP = _SP;
			}
		}
		RETURN

		CASE(JUMPSUB)
		{
			
			size_t destPC = decodeJumpDest(m_code, m_PC);
			byte nArgs = m_code[destPC+1];
			if (stackSize() < nArgs) 
				throwBadStack(stackSize(), nArgs, 0);
		}
		NEXT

		CASE(JUMPSUBV)
		{
			
			_PC = m_PC;
			for (size_t sub = 0, nSubs = m_code[m_PC+1]; sub < nSubs; ++sub)
			{
				
				u256 slot = sub;
				_SP = &slot;
				size_t destPC = decodeJumpvDest(m_code, _PC, _SP);
				byte nArgs = m_code[destPC+1];
				if (stackSize() < nArgs) 
					throwBadStack(stackSize(), nArgs, 0);
			}
			m_PC = _PC;
		}
		NEXT

		CASE(RETURNSUB)
		CASE(RETURN)
		CASE(SUICIDE)
		CASE(STOP)
		{
			
		}
		BREAK;
		
		CASE(BEGINSUB)
		CASE(BEGINDATA)
		CASE(BAD)
		DEFAULT
		{
			throwBadInstruction();
		}
	}
	END_CASES
}

#endif
