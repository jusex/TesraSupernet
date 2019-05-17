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
/** @file VerfiedBlock.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */


#include <libdevcore/Common.h>
#include <libethcore/BlockHeader.h>

#pragma once

namespace dev
{
namespace eth
{

class Transaction;


struct VerifiedBlockRef
{
	bytesConstRef block; 					
	BlockHeader info;							
	std::vector<Transaction> transactions;	
};


struct VerifiedBlock
{
	VerifiedBlock() {}

	VerifiedBlock(BlockHeader&& _bi)
	{
		verified.info = std::move(_bi);
	}

	VerifiedBlock(VerifiedBlock&& _other):
		verified(std::move(_other.verified)),
		blockData(std::move(_other.blockData))
	{
	}

	VerifiedBlock& operator=(VerifiedBlock&& _other)
	{
		assert(&_other != this);

		verified = (std::move(_other.verified));
		blockData = (std::move(_other.blockData));
		return *this;
	}

	VerifiedBlockRef verified;				
	bytes blockData;						

private:
	VerifiedBlock(VerifiedBlock const&) = delete;
	VerifiedBlock operator=(VerifiedBlock const&) = delete;
};

using VerifiedBlocks = std::vector<VerifiedBlock>;

}
}
