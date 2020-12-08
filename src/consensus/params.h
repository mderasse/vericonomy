// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include <arith_uint256.h>
#include <uint256.h>
#include <limits>
#include <map>
#include <string>

namespace Consensus {

/**
 * Parameters that influence chain consensus.
 */
struct Params {
    // XXX: Probably cleanup to do here
    uint256 hashGenesisBlock;
    /** Block height */
    int BIP34Height;
    /** Block height at which BIP65 becomes active */
    int BIP65Height;
    /** Block height at which BIP66 becomes active */
    int BIP66Height;
    /** Block height at which CSV (BIP68, BIP112 and BIP113) becomes active */
    int CSVHeight;

    /** Vericoin Params **/
    arith_uint256 posLimit;
    int NextTargetV2Height;
    int PoSTHeight;
    int PoSHeight;
    int nStakeTargetSpacing;
    int nStakeMinAge;
    int nModifierInterval;

    /** Proof of work parameters */
    arith_uint256 powLimit;
    bool fPowNoRetargeting;
    int64_t nPowTargetTimespan;
    int64_t nPowTargetSpacing;
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** **/
    int nTargetTimespan;
    int nCoinbaseMaturity;
    int nInitialCoinSupply;

    /** VIP */
    int VIP1Height;
};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H