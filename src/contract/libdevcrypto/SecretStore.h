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
/** @file SecretStore.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <functional>
#include <mutex>
#include <libdevcore/FixedHash.h>
#include <libdevcore/FileSystem.h>
#include "Common.h"

namespace dev
{

enum class KDF {
	PBKDF2_SHA256,
	Scrypt,
};

/**
 * Manages encrypted keys stored in a certain directory on disk. The keys are read into memory
 * and changes to the keys are automatically synced to the directory.
 * Each file stores exactly one key in a specific JSON format whose file name is derived from the
 * UUID of the key.
 * @note that most of the functions here affect the filesystem and throw exceptions on failure,
 * and they also throw exceptions upon rare malfunction in the cryptographic functions.
 */
class SecretStore
{
public:
	struct EncryptedKey
	{
		std::string encryptedKey;
		std::string filename;
		Address address;
	};

	
	
	SecretStore() = default;

	
	SecretStore(std::string const& _path);

	
	void setPath(std::string const& _path);

	
	
	
	bytesSec secret(h128 const& _uuid, std::function<std::string()> const& _pass, bool _useCache = true) const;
	
	
	static bytesSec secret(std::string const& _content, std::string const& _pass);
	
	
	bytesSec secret(Address const& _address, std::function<std::string()> const& _pass) const;
	
	h128 importKey(std::string const& _file) { auto ret = readKey(_file, false); if (ret) save(); return ret; }
	
	
	h128 importKeyContent(std::string const& _content) { auto ret = readKeyContent(_content, std::string()); if (ret) save(); return ret; }
	
	
	h128 importSecret(bytesSec const& _s, std::string const& _pass);
	h128 importSecret(bytesConstRef _s, std::string const& _pass);
	
	bool recode(h128 const& _uuid, std::string const& _newPass, std::function<std::string()> const& _pass, KDF _kdf = KDF::Scrypt);
	
	bool recode(Address const& _address, std::string const& _newPass, std::function<std::string()> const& _pass, KDF _kdf = KDF::Scrypt);
	
	void kill(h128 const& _uuid);

	
	std::vector<h128> keys() const { return keysOf(m_keys); }

	
	bool contains(h128 const& _k) const { return m_keys.count(_k); }

	
	
	void clearCache() const;

	
	
	
	h128 readKey(std::string const& _file, bool _takeFileOwnership);
	
	
	
	
	h128 readKeyContent(std::string const& _content, std::string const& _file = std::string());

	
	void save(std::string const& _keysPath);
	
	void save() { save(m_path); }
	
	bool noteAddress(h128 const& _uuid, Address const& _address);
	
	Address address(h128 const& _uuid) const { return m_keys.at(_uuid).address; }

	
	static std::string defaultPath() { return getDataDir("web3") + "/keys"; }

private:
	
	void load(std::string const& _keysPath);
	void load() { load(m_path); }
	
	static std::string encrypt(bytesConstRef _v, std::string const& _pass, KDF _kdf = KDF::Scrypt);
	
	static bytesSec decrypt(std::string const& _v, std::string const& _pass);
	
	std::pair<h128 const, EncryptedKey> const* key(Address const& _address) const;
	std::pair<h128 const, EncryptedKey>* key(Address const& _address);
	
	mutable std::unordered_map<h128, bytesSec> m_cached;
	
	std::unordered_map<h128, EncryptedKey> m_keys;

	std::string m_path;
};

}

