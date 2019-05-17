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
/** @file KeyManager.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <functional>
#include <mutex>
#include <libdevcore/FileSystem.h>
#include <libdevcore/CommonData.h>
#include <libdevcrypto/SecretStore.h>

namespace dev
{
namespace eth
{
class PasswordUnknown: public Exception {};

struct KeyInfo
{
	KeyInfo() = default;
	KeyInfo(h256 const& _passHash, std::string const& _accountName, std::string const& _passwordHint = std::string()): passHash(_passHash), accountName(_accountName), passwordHint(_passwordHint) {}

	
	h256 passHash;
	
	std::string accountName;
	
	std::string passwordHint;
};

static h256 const UnknownPassword;

static auto const DontKnowThrow = [](){ throw PasswordUnknown(); return std::string(); };

enum class SemanticPassword
{
	Existing,
	Master
};



/**
 * @brief High-level manager of password-encrypted keys for Ethereum.
 * Usage:
 *
 * Call exists() to check whether there is already a database. If so, get the master password from
 * the user and call load() with it. If not, get a new master password from the user (get them to type
 * it twice and keep some hint around!) and call create() with it.
 *
 * Uses a "key file" (and a corresponding .salt file) that contains encrypted information about the keys and
 * a directory called "secrets path" that contains a file for each key.
 */
class KeyManager
{
public:
	enum class NewKeyType { DirectICAP = 0, NoVanity, FirstTwo, FirstTwoNextTwo, FirstThree, FirstFour };

	KeyManager(std::string const& _keysFile = defaultPath(), std::string const& _secretsPath = SecretStore::defaultPath());
	~KeyManager();

	void setSecretsPath(std::string const& _secretsPath) { m_store.setPath(_secretsPath); }
	void setKeysFile(std::string const& _keysFile) { m_keysFile = _keysFile; }
	std::string const& keysFile() const { return m_keysFile; }

	bool exists() const;
	void create(std::string const& _pass);
	bool load(std::string const& _pass);
	void save(std::string const& _pass) const { write(_pass, m_keysFile); }

	void notePassword(std::string const& _pass) { m_cachedPasswords[hashPassword(_pass)] = _pass; }
	void noteHint(std::string const& _pass, std::string const& _hint) { if (!_hint.empty()) m_passwordHint[hashPassword(_pass)] = _hint; }
	bool haveHint(std::string const& _pass) const { auto h = hashPassword(_pass); return m_cachedPasswords.count(h) && !m_cachedPasswords.at(h).empty(); }

	
	Addresses accounts() const;
	
	AddressHash accountsHash() const { return AddressHash() + accounts(); }
	bool hasAccount(Address const& _address) const;
	
	std::string const& accountName(Address const& _address) const;
	
	std::string const& passwordHint(Address const& _address) const;
	
	void changeName(Address const& _address, std::string const& _name);

	
	
	bool haveKey(Address const& _a) const { return m_addrLookup.count(_a); }
	
	h128 uuid(Address const& _a) const;
	
	Address address(h128 const& _uuid) const;

	h128 import(Secret const& _s, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	h128 import(Secret const& _s, std::string const& _accountName) { return import(_s, _accountName, defaultPassword(), std::string()); }
	Address importBrain(std::string const& _seed, std::string const& _accountName, std::string const& _seedHint);
	void importExistingBrain(Address const& _a, std::string const& _accountName, std::string const& _seedHint);

	SecretStore& store() { return m_store; }
	void importExisting(h128 const& _uuid, std::string const& _accountName, std::string const& _pass, std::string const& _passwordHint);
	void importExisting(h128 const& _uuid, std::string const& _accountName) { importExisting(_uuid, _accountName, defaultPassword(), std::string()); }
	void importExisting(h128 const& _uuid, std::string const& _accountName, Address const& _addr, h256 const& _passHash = h256(), std::string const& _passwordHint = std::string());

	
	
	Secret secret(Address const& _address, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;
	
	
	Secret secret(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow, bool _usePasswordCache = true) const;

	bool recode(Address const& _address, SemanticPassword _newPass, std::function<std::string()> const& _pass = DontKnowThrow, KDF _kdf = KDF::Scrypt);
	bool recode(Address const& _address, std::string const& _newPass, std::string const& _hint, std::function<std::string()> const& _pass = DontKnowThrow, KDF _kdf = KDF::Scrypt);

	void kill(h128 const& _id) { kill(address(_id)); }
	void kill(Address const& _a);

	static std::string defaultPath() { return getDataDir("ethereum") + "/keys.info"; }

	
	static KeyPair presaleSecret(std::string const& _json, std::function<std::string(bool)> const& _password);

	
	static Secret brain(std::string const& _seed);

	
	static Secret subkey(Secret const& _s, unsigned _index);

	
	static  KeyPair newKeyPair(NewKeyType _type);
private:
	std::string getPassword(h128 const& _uuid, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string getPassword(h256 const& _passHash, std::function<std::string()> const& _pass = DontKnowThrow) const;
	std::string defaultPassword(std::function<std::string()> const& _pass = DontKnowThrow) const { return getPassword(m_master, _pass); }
	h256 hashPassword(std::string const& _pass) const;

	
	void cachePassword(std::string const& _password) const;

	
	
	bool write() const { return write(m_keysFile); }
	bool write(std::string const& _keysFile) const;
	void write(std::string const& _pass, std::string const& _keysFile) const;	
	void write(SecureFixedHash<16> const& _key, std::string const& _keysFile) const;

	

	
	std::unordered_map<h128, Address> m_uuidLookup;
	
	std::unordered_map<Address, h128> m_addrLookup;
	
	std::unordered_map<Address, KeyInfo> m_keyInfo;
	
	std::unordered_map<h256, std::string> m_passwordHint;

	
	mutable std::unordered_map<h256, std::string> m_cachedPasswords;

	
	
	
	
	
	std::string m_defaultPasswordDeprecated;

	mutable std::string m_keysFile;
	mutable SecureFixedHash<16> m_keysFileKey;
	mutable h256 m_master;
	SecretStore m_store;
};

}
}
