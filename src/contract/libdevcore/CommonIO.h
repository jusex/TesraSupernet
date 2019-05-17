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
/** @file CommonIO.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * File & stream I/O routines.
 */

#pragma once

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <list>
#include <memory>
#include <vector>
#include <array>
#include <sstream>
#include <string>
#include <iostream>
#include <chrono>
#include "Common.h"
#include "CommonData.h"
#include "Base64.h"

namespace dev
{


std::string getPassword(std::string const& _prompt);



bytes contents(std::string const& _file);

bytesSec contentsSec(std::string const& _file);


std::string contentsString(std::string const& _file);


bytesRef contentsNew(std::string const& _file, bytesRef _dest = bytesRef());





void writeFile(std::string const& _file, bytesConstRef _data, bool _writeDeleteRename = false);

inline void writeFile(std::string const& _file, bytes const& _data, bool _writeDeleteRename = false) { writeFile(_file, bytesConstRef(&_data), _writeDeleteRename); }
inline void writeFile(std::string const& _file, std::string const& _data, bool _writeDeleteRename = false) { writeFile(_file, bytesConstRef(_data), _writeDeleteRename); }



std::string memDump(bytes const& _bytes, unsigned _width = 8, bool _html = false);




template <class S, class T> struct StreamOut { static S& bypass(S& _out, T const& _t) { _out << _t; return _out; } };
template <class S> struct StreamOut<S, uint8_t> { static S& bypass(S& _out, uint8_t const& _t) { _out << (int)_t; return _out; } };

inline std::ostream& operator<<(std::ostream& _out, bytes const& _e) { _out << toHex(_e, 2, HexPrefix::Add); return _out; }
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::vector<T> const& _e);
template <class T, std::size_t Z> inline std::ostream& operator<<(std::ostream& _out, std::array<T, Z> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::pair<T, U> const& _e);
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::list<T> const& _e);
template <class T1, class T2, class T3> inline std::ostream& operator<<(std::ostream& _out, std::tuple<T1, T2, T3> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::map<T, U> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::unordered_map<T, U> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::set<T, U> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::unordered_set<T, U> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::multimap<T, U> const& _e);
template <class _S, class _T> _S& operator<<(_S& _out, std::shared_ptr<_T> const& _p);

#if defined(_WIN32)
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string _format = "%Y-%m-%d %H:%M:%S")
#else
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string _format = "%F %T")
#endif
{
	unsigned long milliSecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(_e.time_since_epoch()).count();
	auto const durationSinceEpoch = std::chrono::milliseconds(milliSecondsSinceEpoch);
	std::chrono::time_point<std::chrono::system_clock> const tpAfterDuration(durationSinceEpoch);

	tm timeValue;
	auto time = std::chrono::system_clock::to_time_t(tpAfterDuration);
#if defined(_WIN32)
	gmtime_s(&timeValue, &time);
#else
	gmtime_r(&time, &timeValue);
#endif

	unsigned const millisRemainder = milliSecondsSinceEpoch % 1000;
	char buffer[1024];
	if (strftime(buffer, sizeof(buffer), _format.c_str(), &timeValue))
		return std::string(buffer) + "." + (millisRemainder < 1 ? "000" : millisRemainder < 10 ? "00" : millisRemainder < 100 ? "0" : "") + std::to_string(millisRemainder) + "Z";
	return std::string();
}

template <class S, class T>
inline S& streamout(S& _out, std::vector<T> const& _e)
{
	_out << "[";
	if (!_e.empty())
	{
		StreamOut<S, T>::bypass(_out, _e.front());
		for (auto i = ++_e.begin(); i != _e.end(); ++i)
			StreamOut<S, T>::bypass(_out << ",", *i);
	}
	_out << "]";
	return _out;
}

template <class T> inline std::ostream& operator<<(std::ostream& _out, std::vector<T> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T, std::size_t Z>
inline S& streamout(S& _out, std::array<T, Z> const& _e)
{
	_out << "[";
	if (!_e.empty())
	{
		StreamOut<S, T>::bypass(_out, _e.front());
		auto i = _e.begin();
		for (++i; i != _e.end(); ++i)
			StreamOut<S, T>::bypass(_out << ",", *i);
	}
	_out << "]";
	return _out;
}
template <class T, std::size_t Z> inline std::ostream& operator<<(std::ostream& _out, std::array<T, Z> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T>
inline S& streamout(S& _out, std::list<T> const& _e)
{
	_out << "[";
	if (!_e.empty())
	{
		_out << _e.front();
		for (auto i = ++_e.begin(); i != _e.end(); ++i)
			_out << "," << *i;
	}
	_out << "]";
	return _out;
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::list<T> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T, class U>
inline S& streamout(S& _out, std::pair<T, U> const& _e)
{
	_out << "(" << _e.first << "," << _e.second << ")";
	return _out;
}
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::pair<T, U> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T1, class T2, class T3>
inline S& streamout(S& _out, std::tuple<T1, T2, T3> const& _t)
{
	_out << "(" << std::get<0>(_t) << "," << std::get<1>(_t) << "," << std::get<2>(_t) << ")";
	return _out;
}
template <class T1, class T2, class T3> inline std::ostream& operator<<(std::ostream& _out, std::tuple<T1, T2, T3> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T, class U>
S& streamout(S& _out, std::map<T, U> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	int i = 0;
	for (auto p: _v)
		_out << (!(i++) ? "{ " : "; ") << p.first << " => " << p.second;
	return _out << " }";
}
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::map<T, U> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T, class U>
S& streamout(S& _out, std::unordered_map<T, U> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	int i = 0;
	for (auto p: _v)
		_out << (!(i++) ? "{ " : "; ") << p.first << " => " << p.second;
	return _out << " }";
}
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::unordered_map<T, U> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T>
S& streamout(S& _out, std::set<T> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	int i = 0;
	for (auto p: _v)
		_out << (!(i++) ? "{ " : ", ") << p;
	return _out << " }";
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::set<T> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T>
S& streamout(S& _out, std::unordered_set<T> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	int i = 0;
	for (auto p: _v)
		_out << (!(i++) ? "{ " : ", ") << p;
	return _out << " }";
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::unordered_set<T> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T>
S& streamout(S& _out, std::multiset<T> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	int i = 0;
	for (auto p: _v)
		_out << (!(i++) ? "{ " : ", ") << p;
	return _out << " }";
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::multiset<T> const& _e) { streamout(_out, _e); return _out; }

template <class S, class T, class U>
S& streamout(S& _out, std::multimap<T, U> const& _v)
{
	if (_v.empty())
		return _out << "{}";
	T l;
	int i = 0;
	for (auto p: _v)
		if (!(i++))
			_out << "{ " << (l = p.first) << " => " << p.second;
		else if (l == p.first)
			_out << ", " << p.second;
		else
			_out << "; " << (l = p.first) << " => " << p.second;
	return _out << " }";
}
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::multimap<T, U> const& _e) { streamout(_out, _e); return _out; }

template <class _S, class _T> _S& operator<<(_S& _out, std::shared_ptr<_T> const& _p) { if (_p) _out << "@" << (*_p); else _out << "nullptr"; return _out; }




template <class _T>
std::string toString(_T const& _t)
{
	std::ostringstream o;
	o << _t;
	return o.str();
}

}
