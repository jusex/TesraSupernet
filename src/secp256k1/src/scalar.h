/**********************************************************************
 * Copyright (c) 2014 Pieter Wuille                                   *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http:
 **********************************************************************/

#ifndef SECP256K1_SCALAR_H
#define SECP256K1_SCALAR_H

#include "num.h"

#if defined HAVE_CONFIG_H
#include "libsecp256k1-config.h"
#endif

#if defined(EXHAUSTIVE_TEST_ORDER)
#include "scalar_low.h"
#elif defined(USE_SCALAR_4X64)
#include "scalar_4x64.h"
#elif defined(USE_SCALAR_8X32)
#include "scalar_8x32.h"
#else
#error "Please select scalar implementation"
#endif


static void secp256k1_scalar_clear(secp256k1_scalar *r);


static unsigned int secp256k1_scalar_get_bits(const secp256k1_scalar *a, unsigned int offset, unsigned int count);


static unsigned int secp256k1_scalar_get_bits_var(const secp256k1_scalar *a, unsigned int offset, unsigned int count);


static void secp256k1_scalar_set_b32(secp256k1_scalar *r, const unsigned char *bin, int *overflow);


static void secp256k1_scalar_set_int(secp256k1_scalar *r, unsigned int v);


static void secp256k1_scalar_get_b32(unsigned char *bin, const secp256k1_scalar* a);


static int secp256k1_scalar_add(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b);


static void secp256k1_scalar_cadd_bit(secp256k1_scalar *r, unsigned int bit, int flag);


static void secp256k1_scalar_mul(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b);

/** Shift a scalar right by some amount strictly between 0 and 16, returning
 *  the low bits that were shifted off */
static int secp256k1_scalar_shr_int(secp256k1_scalar *r, int n);


static void secp256k1_scalar_sqr(secp256k1_scalar *r, const secp256k1_scalar *a);


static void secp256k1_scalar_inverse(secp256k1_scalar *r, const secp256k1_scalar *a);


static void secp256k1_scalar_inverse_var(secp256k1_scalar *r, const secp256k1_scalar *a);


static void secp256k1_scalar_negate(secp256k1_scalar *r, const secp256k1_scalar *a);


static int secp256k1_scalar_is_zero(const secp256k1_scalar *a);


static int secp256k1_scalar_is_one(const secp256k1_scalar *a);


static int secp256k1_scalar_is_even(const secp256k1_scalar *a);


static int secp256k1_scalar_is_high(const secp256k1_scalar *a);

/** Conditionally negate a number, in constant time.
 * Returns -1 if the number was negated, 1 otherwise */
static int secp256k1_scalar_cond_negate(secp256k1_scalar *a, int flag);

#ifndef USE_NUM_NONE

static void secp256k1_scalar_get_num(secp256k1_num *r, const secp256k1_scalar *a);


static void secp256k1_scalar_order_get_num(secp256k1_num *r);
#endif


static int secp256k1_scalar_eq(const secp256k1_scalar *a, const secp256k1_scalar *b);

#ifdef USE_ENDOMORPHISM

static void secp256k1_scalar_split_128(secp256k1_scalar *r1, secp256k1_scalar *r2, const secp256k1_scalar *a);

static void secp256k1_scalar_split_lambda(secp256k1_scalar *r1, secp256k1_scalar *r2, const secp256k1_scalar *a);
#endif


static void secp256k1_scalar_mul_shift_var(secp256k1_scalar *r, const secp256k1_scalar *a, const secp256k1_scalar *b, unsigned int shift);

#endif 
