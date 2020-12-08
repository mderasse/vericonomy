// Copyright (c) 2016-2020 The Vericonomy developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_POS_H
#define BITCOIN_POS_H

#include <consensus/params.h>

class CBlockHeader;
class CBlockIndex;
class CBlock;
class CWallet;
class CTransaction;
class CCoinsViewCache;

/** Get next required staking work **/
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, const Consensus::Params& params);

double GetPoSKernelPS(CBlockIndex* pindexPrev, const Consensus::Params& params);
double GetPoSKernelPS(CBlockIndex* pindexPrev);
double GetPoSKernelPS();

double GetCurrentInflationRate(double nAverageWeight);
double GetCurrentInterestRate(CBlockIndex* pindexPrev, const Consensus::Params& params);
double GetAverageStakeWeight(CBlockIndex* pindexPrev);
int64_t GetStakeTimeFactoredWeight(int64_t timeWeight, int64_t bnCoinDayWeight, CBlockIndex* pindexPrev);

/** Get reward amount for a solved work **/
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, CBlockIndex* pindex, const Consensus::Params& params);

bool GetCoinAge(const CTransaction& tx, const CCoinsViewCache &view, uint64_t& nCoinAge, CBlockIndex* pindexPre);
bool SignBlock(CBlock& block, const CWallet& keystore);
bool CheckBlockSignature(const CBlock& block);

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx);


static const double PI = 3.1415926535;


#endif // BITCOIN_POS_H
