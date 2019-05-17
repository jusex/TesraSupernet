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
/** @file ExtVMFace.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <set>
#include <functional>
#include <boost/optional.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/RLP.h>
#include <libdevcore/SHA3.h>
#include <libevmcore/Instruction.h>
#include <libethcore/Common.h>
#include <libethcore/BlockHeader.h>
#include <libethcore/ChainOperationParams.h>

namespace dev
{
namespace eth
{
















class owning_bytes_ref: public vector_ref<byte const>
{
public:
	owning_bytes_ref() = default;

	
	
	
	owning_bytes_ref(bytes&& _bytes, size_t _begin, size_t _size):
			m_bytes(std::move(_bytes))
	{
		
		
		retarget(&m_bytes[_begin], _size);
	}

	owning_bytes_ref(owning_bytes_ref const&) = delete;
	owning_bytes_ref(owning_bytes_ref&&) = default;
	owning_bytes_ref& operator=(owning_bytes_ref const&) = delete;
	owning_bytes_ref& operator=(owning_bytes_ref&&) = default;

private:
	bytes m_bytes;
};

enum class BlockPolarity
{
	Unknown,
	Dead,
	Live
};

struct LogEntry
{
	LogEntry() {}
	LogEntry(RLP const& _r) { address = (Address)_r[0]; topics = _r[1].toVector<h256>(); data = _r[2].toBytes(); }
	LogEntry(Address const& _address, h256s const& _ts, bytes&& _d): address(_address), topics(_ts), data(std::move(_d)) {}

	void streamRLP(RLPStream& _s) const { _s.appendList(3) << address << topics << data; }

	LogBloom bloom() const
	{
		LogBloom ret;
		ret.shiftBloom<3>(sha3(address.ref()));
		for (auto t: topics)
			ret.shiftBloom<3>(sha3(t.ref()));
		return ret;
	}

	Address address;
	h256s topics;
	bytes data;
};

using LogEntries = std::vector<LogEntry>;

struct LocalisedLogEntry: public LogEntry
{
	LocalisedLogEntry() {}
	explicit LocalisedLogEntry(LogEntry const& _le): LogEntry(_le) {}

	explicit LocalisedLogEntry(
		LogEntry const& _le,
		h256 _special
	):
		LogEntry(_le),
		isSpecial(true),
		special(_special)
	{}

	explicit LocalisedLogEntry(
		LogEntry const& _le,
		h256 const& _blockHash,
		BlockNumber _blockNumber,
		h256 const& _transactionHash,
		unsigned _transactionIndex,
		unsigned _logIndex,
		BlockPolarity _polarity = BlockPolarity::Unknown
	):
		LogEntry(_le),
		blockHash(_blockHash),
		blockNumber(_blockNumber),
		transactionHash(_transactionHash),
		transactionIndex(_transactionIndex),
		logIndex(_logIndex),
		polarity(_polarity),
		mined(true)
	{}

	h256 blockHash;
	BlockNumber blockNumber = 0;
	h256 transactionHash;
	unsigned transactionIndex = 0;
	unsigned logIndex = 0;
	BlockPolarity polarity = BlockPolarity::Unknown;
	bool mined = false;
	bool isSpecial = false;
	h256 special;
};

using LocalisedLogEntries = std::vector<LocalisedLogEntry>;

inline LogBloom bloom(LogEntries const& _logs)
{
	LogBloom ret;
	for (auto const& l: _logs)
		ret |= l.bloom();
	return ret;
}

struct SubState
{
	std::set<Address> suicides;	
	LogEntries logs;			
	u256 refunds;				

	SubState& operator+=(SubState const& _s)
	{
		suicides += _s.suicides;
		refunds += _s.refunds;
		logs += _s.logs;
		return *this;
	}

	void clear()
	{
		suicides.clear();
		logs.clear();
		refunds = 0;
	}
};

class ExtVMFace;
class VM;

using LastHashes = std::vector<h256>;

using OnOpFunc = std::function<void(uint64_t , uint64_t , Instruction , bigint , bigint , bigint , VM*, ExtVMFace const*)>;

struct CallParameters
{
	Address senderAddress;
	Address codeAddress;
	Address receiveAddress;
	u256 valueTransfer;
	u256 apparentValue;
	u256 gas;
	bytesConstRef data;
	OnOpFunc onOp;
};

class EnvInfo
{
public:
	EnvInfo() {}
	EnvInfo(BlockHeader const& _current, LastHashes const& _lh = LastHashes(), u256 const& _gasUsed = u256()):
		m_number(_current.number()),
		m_author(_current.author()),
		m_timestamp(_current.timestamp()),
		m_difficulty(_current.difficulty()),
		
		
		
		m_gasLimit(_current.gasLimit().convert_to<int64_t>()),
		m_lastHashes(_lh),
		m_gasUsed(_gasUsed)
	{}

	EnvInfo(BlockHeader const& _current, LastHashes&& _lh, u256 const& _gasUsed = u256()):
		m_number(_current.number()),
		m_author(_current.author()),
		m_timestamp(_current.timestamp()),
		m_difficulty(_current.difficulty()),
		
		
		
		m_gasLimit(_current.gasLimit().convert_to<int64_t>()),
		m_lastHashes(_lh),
		m_gasUsed(_gasUsed)
	{}

	u256 const& number() const { return m_number; }
	Address const& author() const { return m_author; }
	u256 const& timestamp() const { return m_timestamp; }
	u256 const& difficulty() const { return m_difficulty; }
	int64_t gasLimit() const { return m_gasLimit; }
	LastHashes const& lastHashes() const { return m_lastHashes; }
	u256 const& gasUsed() const { return m_gasUsed; }

	void setNumber(u256 const& _v) { m_number = _v; }
	void setAuthor(Address const& _v) { m_author = _v; }
	void setTimestamp(u256 const& _v) { m_timestamp = _v; }
	void setDifficulty(u256 const& _v) { m_difficulty = _v; }
	void setGasLimit(int64_t _v) { m_gasLimit = _v; }
	void setLastHashes(LastHashes&& _lh) { m_lastHashes = _lh; }

private:
	u256 m_number;
	Address m_author;
	u256 m_timestamp;
	u256 m_difficulty;
	int64_t m_gasLimit;
	LastHashes m_lastHashes;
	u256 m_gasUsed;
};

/**
 * @brief Interface and null implementation of the class for specifying VM externalities.
 */
class ExtVMFace
{
public:
	
	ExtVMFace() = default;

	
	ExtVMFace(EnvInfo const& _envInfo, Address _myAddress, Address _caller, Address _origin, u256 _value, u256 _gasPrice, bytesConstRef _data, bytes _code, h256 const& _codeHash, unsigned _depth);

	virtual ~ExtVMFace() = default;

	ExtVMFace(ExtVMFace const&) = delete;
	ExtVMFace& operator=(ExtVMFace const&) = delete;

	
	virtual u256 store(u256) { return 0; }

	
	virtual void setStore(u256, u256) {}

	
	virtual u256 balance(Address) { return 0; }

	
	virtual bytes const& codeAt(Address) { return NullBytes; }

	
	virtual size_t codeSizeAt(Address) { return 0; }

	
	virtual bool exists(Address) { return false; }

	
	virtual void suicide(Address) { sub.suicides.insert(myAddress); }

	
	virtual h160 create(u256, u256&, bytesConstRef, OnOpFunc const&) { return h160(); }

	
	virtual boost::optional<owning_bytes_ref> call(CallParameters&) = 0;

	
	virtual void log(h256s&& _topics, bytesConstRef _data) { sub.logs.push_back(LogEntry(myAddress, std::move(_topics), _data.toBytes())); }

	
	h256 blockHash(u256 _number) { return _number < envInfo().number() && _number >= (std::max<u256>(256, envInfo().number()) - 256) ? envInfo().lastHashes()[(unsigned)(envInfo().number() - 1 - _number)] : h256(); }

	
	EnvInfo const& envInfo() const { return m_envInfo; }

	
	virtual EVMSchedule const& evmSchedule() const { return DefaultSchedule; }

private:
	EnvInfo const& m_envInfo;

public:
	
	Address myAddress;			
	Address caller;				
	Address origin;				
	u256 value;					
	u256 gasPrice;				
	bytesConstRef data;			
	bytes code;					
	h256 codeHash;				
	SubState sub;				
	unsigned depth = 0;			
};

}
}
