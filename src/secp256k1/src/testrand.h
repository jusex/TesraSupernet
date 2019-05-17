/**********************************************************************
 * Copyright (c) 2013, 2014 Pieter Wuille                             *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http:
 **********************************************************************/

#ifndef SECP256K1_TESTRAND_H
#define SECP256K1_TESTRAND_H

#if defined HAVE_CONFIG_H
#include "libsecp256k1-config.h"
#endif




SECP256K1_INLINE static void secp256k1_rand_seed(const unsigned char *seed16);


static uint32_t secp256k1_rand32(void);

/** Generate a pseudorandom number in the range [0..2**bits-1]. Bits must be 1 or
 *  more. */
static uint32_t secp256k1_rand_bits(int bits);


static uint32_t secp256k1_rand_int(uint32_t range);


static void secp256k1_rand256(unsigned char *b32);


static void secp256k1_rand256_test(unsigned char *b32);


static void secp256k1_rand_bytes_test(unsigned char *bytes, size_t len);

#endif 
