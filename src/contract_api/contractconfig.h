



#ifndef SUPERBITCOIN_CONTRACTCONFIG_H
#define SUPERBITCOIN_CONTRACTCONFIG_H

#include <uint256.h>
#include <amount.h>



static const uint64_t STANDARD_MINIMUM_GAS_LIMIT = 10000;


static const uint64_t STANDARD_MINIMUM_GAS_PRICE = 1;

static const uint64_t DEFAULT_GAS_LIMIT_OP_CREATE = 2500000;
static const uint64_t DEFAULT_GAS_LIMIT_OP_SEND = 250000;
static const CAmount DEFAULT_GAS_PRICE = 0.00000040 * COIN;
static const CAmount MAX_RPC_GAS_PRICE = 0.00000100 * COIN;



static const uint64_t MINIMUM_GAS_LIMIT = 10000;

static const uint64_t MEMPOOL_MIN_GAS_LIMIT = 22000;

#define CONTRACT_STATE_DIR "stateContract"

static const uint256 DEFAULT_HASH_STATE_ROOT = uint256S("0x21b463e3b52f6201c0ad6c991be0485b6ef8c092e64583ffa655cc1b171fe856");
static const uint256 DEFAULT_HASH_UTXO_ROOT = uint256S("0x21b463e3b52f6201c0ad6c991be0485b6ef8c092e64583ffa655cc1b171fe856");

#endif 
