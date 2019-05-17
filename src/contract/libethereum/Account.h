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
/** @file Account.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>
#include <libdevcore/RLP.h>
#include <libdevcore/TrieDB.h>
#include <libdevcore/SHA3.h>
#include <libethcore/Common.h>

namespace dev
{
namespace eth
{

/**
 * Models the state of a single Ethereum account.
 * Used to cache a portion of the full Ethereum state. State keeps a mapping of Address's to Accounts.
 *
 * Aside from storing the nonce and balance, the account may also be "dead" (where isAlive() returns false).
 * This allows State to explicitly store the notion of a deleted account in it's cache. kill() can be used
 * for this.
 *
 * For the account's storage, the class operates a cache. baseRoot() specifies the base state of the storage
 * given as the Trie root to be looked up in the state database. Alterations beyond this base are specified
 * in the overlay, stored in this class and retrieved with storageOverlay(). setStorage allows the overlay
 * to be altered.
 *
 * The code handling explicitly supports a two-stage commit model needed for contract-creation. When creating
 * a contract (running the initialisation code), the code of the account is considered empty. The attribute
 * of emptiness can be retrieved with codeBearing(). After initialisation one must set the code accordingly;
 * the code of the Account can be set with setCode(). To validate a setCode() call, this class records the
 * state of being in contract-creation (and thus in a state where setCode may validly be called). It can be
 * determined through isFreshCode().
 *
 * The code can be retrieved through code(), and its hash through codeHash(). codeHash() is only valid when
 * the account is not in the contract-creation phase (i.e. when isFreshCode() returns false). This class
 * supports populating code on-demand from the state database. To determine if the code has been prepopulated
 * call codeCacheValid(). To populate the code, look it up with codeHash() and populate with noteCode().
 *
 * @todo: need to make a noteCodeCommitted().
 *
 * The constructor allows you to create an one of a number of "types" of accounts. The default constructor
 * makes a dead account (this is ignored by State when writing out the Trie). Another three allow a basic
 * or contract account to be specified along with an initial balance. The fina two allow either a basic or
 * a contract account to be created with arbitrary values.
 */
class Account
{
public:
	
	enum Changedness
	{
		
		Changed,
		
		Unchanged
	};

	
	Account() {}

	
	
	
	Account(u256 _nonce, u256 _balance, Changedness _c = Changed): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance) {}

	
	Account(u256 _nonce, u256 _balance, h256 _contractRoot, h256 _codeHash, Changedness _c): m_isAlive(true), m_isUnchanged(_c == Unchanged), m_nonce(_nonce), m_balance(_balance), m_storageRoot(_contractRoot), m_codeHash(_codeHash) { assert(_contractRoot); }


	
	void kill() { m_isAlive = false; m_storageOverlay.clear(); m_codeHash = EmptySHA3; m_storageRoot = EmptyTrie; m_balance = 0; m_nonce = 0; changed(); }

	
	
	
	bool isAlive() const { return m_isAlive; }

	
	bool isDirty() const { return !m_isUnchanged; }

	void untouch() { m_isUnchanged = true; }

	
	
	bool isEmpty() const { return nonce() == 0 && balance() == 0 && codeHash() == EmptySHA3; }

	
	u256 const& balance() const { return m_balance; }

	
	void addBalance(u256 _value) { m_balance += _value; changed(); }

	
	u256 nonce() const { return m_nonce; }

	
	void incNonce() { ++m_nonce; changed(); }

	
	
	void setNonce(u256 const& _nonce) { m_nonce = _nonce; changed(); }


	
	
	h256 baseRoot() const { assert(m_storageRoot); return m_storageRoot; }

	
	std::unordered_map<u256, u256> const& storageOverlay() const { return m_storageOverlay; }

	
	
	void setStorage(u256 _p, u256 _v) { m_storageOverlay[_p] = _v; changed(); }

	
	
	void setStorageCache(u256 _p, u256 _v) const { const_cast<decltype(m_storageOverlay)&>(m_storageOverlay)[_p] = _v; }

	
	h256 codeHash() const { return m_codeHash; }

	bool hasNewCode() const { return m_hasNewCode; }

	
	void setNewCode(bytes&& _code);

	
	void resetCode() { m_codeCache.clear(); m_hasNewCode = false; m_codeHash = EmptySHA3; }

	
	
	void noteCode(bytesConstRef _code) { assert(sha3(_code) == m_codeHash); m_codeCache = _code.toBytes(); }

	
	bytes const& code() const { return m_codeCache; }

private:
	
	void changed() { m_isUnchanged = false; }

	
	bool m_isAlive = false;

	
	bool m_isUnchanged = false;

	
	bool m_hasNewCode = false;

	
	u256 m_nonce;

	
	u256 m_balance = 0;

	
	
	h256 m_storageRoot = EmptyTrie;

	/** If c_contractConceptionCodeHash then we're in the limbo where we're running the initialisation code.
	 * We expect a setCode() at some point later.
	 * If EmptySHA3, then m_code, which should be empty, is valid.
	 * If anything else, then m_code is valid iff it's not empty, otherwise, State::ensureCached() needs to
	 * be called with the correct args.
	 */
	h256 m_codeHash = EmptySHA3;

	
	std::unordered_map<u256, u256> m_storageOverlay;

	
	
	bytes m_codeCache;

	
	static const h256 c_contractConceptionCodeHash;
};

class AccountMask
{
public:
	AccountMask(bool _all = false):
		m_hasBalance(_all),
		m_hasNonce(_all),
		m_hasCode(_all),
		m_hasStorage(_all)
	{}

	AccountMask(
		bool _hasBalance,
		bool _hasNonce,
		bool _hasCode,
		bool _hasStorage,
		bool _shouldNotExist = false
	):
		m_hasBalance(_hasBalance),
		m_hasNonce(_hasNonce),
		m_hasCode(_hasCode),
		m_hasStorage(_hasStorage),
		m_shouldNotExist(_shouldNotExist)
	{}

	bool allSet() const { return m_hasBalance && m_hasNonce && m_hasCode && m_hasStorage; }
	bool hasBalance() const { return m_hasBalance; }
	bool hasNonce() const { return m_hasNonce; }
	bool hasCode() const { return m_hasCode; }
	bool hasStorage() const { return m_hasStorage; }
	bool shouldExist() const { return !m_shouldNotExist; }

private:
	bool m_hasBalance;
	bool m_hasNonce;
	bool m_hasCode;
	bool m_hasStorage;
	bool m_shouldNotExist = false;
};

using AccountMap = std::unordered_map<Address, Account>;
using AccountMaskMap = std::unordered_map<Address, AccountMask>;

class PrecompiledContract;
using PrecompiledContractMap = std::unordered_map<Address, PrecompiledContract>;

AccountMap jsonToAccountMap(std::string const& _json, u256 const& _defaultNonce = 0, AccountMaskMap* o_mask = nullptr, PrecompiledContractMap* o_precompiled = nullptr);

}
}
