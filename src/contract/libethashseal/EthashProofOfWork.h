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
/** @file EthashProofOfWork.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2015
 *
 * Determines the PoW algorithm.
 */

#pragma once

#include <libethcore/BlockHeader.h>

namespace dev
{
namespace eth
{


struct EthashProofOfWork
{
	struct Solution
	{
		Nonce nonce;
		h256 mixHash;
	};

	struct Result
	{
		h256 value;
		h256 mixHash;
	};

	struct WorkPackage
	{
		WorkPackage() = default;
		WorkPackage(BlockHeader const& _bh);
		void reset() { headerHash = h256(); }
		operator bool() const { return headerHash != h256(); }

		h256 boundary;
		h256 headerHash;	
		h256 seedHash;
	};

	static const WorkPackage NullWorkPackage;

	
	static const unsigned defaultLocalWorkSize;
	
	static const unsigned defaultGlobalWorkSizeMultiplier;
	
	static const unsigned defaultMSPerBatch;
};

}
}
