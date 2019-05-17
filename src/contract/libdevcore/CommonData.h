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
/** @file CommonData.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Shared algorithms and data types.
 */

#pragma once

#include <vector>
#include <algorithm>
#include <unordered_set>
#include <type_traits>
#include <cstring>
#include <string>
#include "Common.h"

namespace dev
{



enum class WhenError
{
	DontThrow = 0,
	Throw = 1,
};

enum class HexPrefix
{
	DontAdd = 0,
	Add = 1,
};



template <class T>
std::string toHex(T const& _data, int _w = 2, HexPrefix _prefix = HexPrefix::DontAdd)
{
	std::ostringstream ret;
	unsigned ii = 0;
	for (auto i: _data)
		ret << std::hex << std::setfill('0') << std::setw(ii++ ? 2 : _w) << (int)(typename std::make_unsigned<decltype(i)>::type)i;
	return (_prefix == HexPrefix::Add) ? "0x" + ret.str() : ret.str();
}




bytes fromHex(std::string const& _s, WhenError _throw = WhenError::DontThrow);


bool isHex(std::string const& _s) noexcept;


template <class T> static bool isHash(std::string const& _hash)
{
	return (_hash.size() == T::size * 2 || (_hash.size() == T::size * 2 + 2 && _hash.substr(0, 2) == "0x")) && isHex(_hash);
}



inline std::string asString(bytes const& _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}



inline std::string asString(bytesConstRef _b)
{
	return std::string((char const*)_b.data(), (char const*)(_b.data() + _b.size()));
}


inline bytes asBytes(std::string const& _b)
{
	return bytes((byte const*)_b.data(), (byte const*)(_b.data() + _b.size()));
}



bytes asNibbles(bytesConstRef const& _s);









template <class T, class Out>
inline void toBigEndian(T _val, Out& o_out)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); 
	for (auto i = o_out.size(); i != 0; _val >>= 8, i--)
	{
		T v = _val & (T)0xff;
		o_out[i - 1] = (typename Out::value_type)(uint8_t)v;
	}
}




template <class T, class _In>
inline T fromBigEndian(_In const& _bytes)
{
	T ret = (T)0;
	for (auto i: _bytes)
		ret = (T)((ret << 8) | (byte)(typename std::make_unsigned<decltype(i)>::type)i);
	return ret;
}


inline std::string toBigEndianString(u256 _val) { std::string ret(32, '\0'); toBigEndian(_val, ret); return ret; }
inline std::string toBigEndianString(u160 _val) { std::string ret(20, '\0'); toBigEndian(_val, ret); return ret; }
inline bytes toBigEndian(u256 _val) { bytes ret(32); toBigEndian(_val, ret); return ret; }
inline bytes toBigEndian(u160 _val) { bytes ret(20); toBigEndian(_val, ret); return ret; }



template <class T>
inline bytes toCompactBigEndian(T _val, unsigned _min = 0)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); 
	int i = 0;
	for (T v = _val; v; ++i, v >>= 8) {}
	bytes ret(std::max<unsigned>(_min, i), 0);
	toBigEndian(_val, ret);
	return ret;
}
inline bytes toCompactBigEndian(byte _val, unsigned _min = 0)
{
	return (_min || _val) ? bytes{ _val } : bytes{};
}



template <class T>
inline std::string toCompactBigEndianString(T _val, unsigned _min = 0)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); 
	int i = 0;
	for (T v = _val; v; ++i, v >>= 8) {}
	std::string ret(std::max<unsigned>(_min, i), '\0');
	toBigEndian(_val, ret);
	return ret;
}


inline std::string toHex(u256 val, HexPrefix prefix = HexPrefix::DontAdd)
{
	std::string str = toHex(toBigEndian(val));
	return (prefix == HexPrefix::Add) ? "0x" + str : str;
}

inline std::string toCompactHex(u256 val, HexPrefix prefix = HexPrefix::DontAdd, unsigned _min = 0)
{
	std::string str = toHex(toCompactBigEndian(val, _min));
	return (prefix == HexPrefix::Add) ? "0x" + str : str;
}





std::string escaped(std::string const& _s, bool _all = true);




template <class T, class _U>
unsigned commonPrefix(T const& _t, _U const& _u)
{
	unsigned s = std::min<unsigned>(_t.size(), _u.size());
	for (unsigned i = 0;; ++i)
		if (i == s || _t[i] != _u[i])
			return i;
	return s;
}


std::string randomWord();


template <class T>
inline unsigned bytesRequired(T _i)
{
	static_assert(std::is_same<bigint, T>::value || !std::numeric_limits<T>::is_signed, "only unsigned types or bigint supported"); 
	unsigned i = 0;
	for (; _i != 0; ++i, _i >>= 8) {}
	return i;
}



template <class T>
void trimFront(T& _t, unsigned _elements)
{
	static_assert(std::is_pod<typename T::value_type>::value, "");
	memmove(_t.data(), _t.data() + _elements, (_t.size() - _elements) * sizeof(_t[0]));
	_t.resize(_t.size() - _elements);
}



template <class T, class _U>
void pushFront(T& _t, _U _e)
{
	static_assert(std::is_pod<typename T::value_type>::value, "");
	_t.push_back(_e);
	memmove(_t.data() + 1, _t.data(), (_t.size() - 1) * sizeof(_e));
	_t[0] = _e;
}


template <class T>
inline std::vector<T>& operator+=(std::vector<typename std::enable_if<std::is_pod<T>::value, T>::type>& _a, std::vector<T> const& _b)
{
	auto s = _a.size();
	_a.resize(_a.size() + _b.size());
	memcpy(_a.data() + s, _b.data(), _b.size() * sizeof(T));
	return _a;

}


template <class T>
inline std::vector<T>& operator+=(std::vector<typename std::enable_if<!std::is_pod<T>::value, T>::type>& _a, std::vector<T> const& _b)
{
	_a.reserve(_a.size() + _b.size());
	for (auto& i: _b)
		_a.push_back(i);
	return _a;
}


template <class T, class U> std::set<T>& operator+=(std::set<T>& _a, U const& _b)
{
	for (auto const& i: _b)
		_a.insert(i);
	return _a;
}


template <class T, class U> std::unordered_set<T>& operator+=(std::unordered_set<T>& _a, U const& _b)
{
	for (auto const& i: _b)
		_a.insert(i);
	return _a;
}


template <class T, class U> std::vector<T>& operator+=(std::vector<T>& _a, U const& _b)
{
	for (auto const& i: _b)
		_a.push_back(i);
	return _a;
}


template <class T, class U> std::set<T> operator+(std::set<T> _a, U const& _b)
{
	return _a += _b;
}


template <class T, class U> std::unordered_set<T> operator+(std::unordered_set<T> _a, U const& _b)
{
	return _a += _b;
}


template <class T, class U> std::vector<T> operator+(std::vector<T> _a, U const& _b)
{
	return _a += _b;
}


template <class T>
inline std::vector<T> operator+(std::vector<T> const& _a, std::vector<T> const& _b)
{
	std::vector<T> ret(_a);
	return ret += _b;
}


template <class T>
inline std::set<T>& operator+=(std::set<T>& _a, std::set<T> const& _b)
{
	for (auto& i: _b)
		_a.insert(i);
	return _a;
}


template <class T>
inline std::set<T> operator+(std::set<T> const& _a, std::set<T> const& _b)
{
	std::set<T> ret(_a);
	return ret += _b;
}

template <class A, class B>
std::unordered_map<A, B>& operator+=(std::unordered_map<A, B>& _x, std::unordered_map<A, B> const& _y)
{
	for (auto const& i: _y)
		_x.insert(i);
	return _x;
}

template <class A, class B>
std::unordered_map<A, B> operator+(std::unordered_map<A, B> const& _x, std::unordered_map<A, B> const& _y)
{
	std::unordered_map<A, B> ret(_x);
	return ret += _y;
}


std::string toString(string32 const& _s);

template<class T, class U>
std::vector<T> keysOf(std::map<T, U> const& _m)
{
	std::vector<T> ret;
	for (auto const& i: _m)
		ret.push_back(i.first);
	return ret;
}

template<class T, class U>
std::vector<T> keysOf(std::unordered_map<T, U> const& _m)
{
	std::vector<T> ret;
	for (auto const& i: _m)
		ret.push_back(i.first);
	return ret;
}

template<class T, class U>
std::vector<U> valuesOf(std::map<T, U> const& _m)
{
	std::vector<U> ret;
	ret.reserve(_m.size());
	for (auto const& i: _m)
		ret.push_back(i.second);
	return ret;
}

template<class T, class U>
std::vector<U> valuesOf(std::unordered_map<T, U> const& _m)
{
	std::vector<U> ret;
	ret.reserve(_m.size());
	for (auto const& i: _m)
		ret.push_back(i.second);
	return ret;
}

template <class T, class V>
bool contains(T const& _t, V const& _v)
{
	return std::end(_t) != std::find(std::begin(_t), std::end(_t), _v);
}

}
