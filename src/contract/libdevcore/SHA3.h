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
/** @file SHA3.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * The FixedHash fixed-size "hash" container type.
 */

#pragma once

#include <string>
#include "FixedHash.h"
#include "vector_ref.h"

namespace dev
{





bool sha3(bytesConstRef _input, bytesRef o_output);


inline h256 sha3(bytesConstRef _input) { h256 ret; sha3(_input, ret.ref()); return ret; }
inline SecureFixedHash<32> sha3Secure(bytesConstRef _input) { SecureFixedHash<32> ret; sha3(_input, ret.writable().ref()); return ret; }


inline h256 sha3(bytes const& _input) { return sha3(bytesConstRef(&_input)); }
inline SecureFixedHash<32> sha3Secure(bytes const& _input) { return sha3Secure(bytesConstRef(&_input)); }


inline h256 sha3(std::string const& _input) { return sha3(bytesConstRef(_input)); }
inline SecureFixedHash<32> sha3Secure(std::string const& _input) { return sha3Secure(bytesConstRef(_input)); }


template<unsigned N> inline h256 sha3(FixedHash<N> const& _input) { return sha3(_input.ref()); }
template<unsigned N> inline SecureFixedHash<32> sha3Secure(FixedHash<N> const& _input) { return sha3Secure(_input.ref()); }


inline SecureFixedHash<32> sha3(bytesSec const& _input) { return sha3Secure(_input.ref()); }
inline SecureFixedHash<32> sha3Secure(bytesSec const& _input) { return sha3Secure(_input.ref()); }
template<unsigned N> inline SecureFixedHash<32> sha3(SecureFixedHash<N> const& _input) { return sha3Secure(_input.ref()); }
template<unsigned N> inline SecureFixedHash<32> sha3Secure(SecureFixedHash<N> const& _input) { return sha3Secure(_input.ref()); }


inline std::string sha3(std::string const& _input, bool _isNibbles) { return asString((_isNibbles ? sha3(fromHex(_input)) : sha3(bytesConstRef(&_input))).asBytes()); }


inline void sha3mac(bytesConstRef _secret, bytesConstRef _plain, bytesRef _output) { sha3(_secret.toBytes() + _plain.toBytes()).ref().populate(_output); }

extern h256 EmptySHA3;

extern h256 EmptyListSHA3;

}
