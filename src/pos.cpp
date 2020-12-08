// Copyright (c) 2016-2020 The Vericonomy developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pos.h>

#include <amount.h>
#include <arith_uint256.h>
#include <bignum.h>
#include <chain.h>
#include <index/txindex.h>
#include <net.h>
#include <primitives/block.h>
#include <validation.h>
#include <uint256.h>

double GetDifficulty(const CBlockIndex* blockindex = nullptr);

static int nAverageStakeWeightHeightCached = 0;
static double dAverageStakeWeightCached = 0;

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    arith_uint256 bnTargetLimit = pindexLast->IsProofOfWork() ? params.powLimit : params.posLimit;

    if (pindexLast == nullptr)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, true);
    if (pindexPrev->pprev == nullptr)
        return bnTargetLimit.GetCompact(); // first block

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, true);
    if (pindexPrevPrev->pprev == nullptr)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // Protocol change, NextTargetV2. Split the if to be more readable
    if (pindexLast->nHeight >= params.NextTargetV2Height)
    {
        if (nActualSpacing < 0)
            nActualSpacing = params.nStakeTargetSpacing;
    }

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    int64_t nInterval = params.nTargetTimespan / params.nStakeTargetSpacing;
    bnNew *= ((nInterval - 1) * params.nStakeTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * params.nStakeTargetSpacing);

    // Protocol change, NextTargetV2. Split the if to be more readable
    if (pindexLast->nHeight < params.NextTargetV2Height)
    {
        if (bnNew > CBigNum(ArithToUint256(bnTargetLimit)))
            bnNew = CBigNum(ArithToUint256(bnTargetLimit));
    }
    else
    {
        if (bnNew <= 0 || bnNew > CBigNum(ArithToUint256(bnTargetLimit)))
            bnNew = CBigNum(ArithToUint256(bnTargetLimit));
    }

    return bnNew.GetCompact();
}

double GetPoSKernelPS(CBlockIndex* pindexPrev, const Consensus::Params& params)
{
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    CBlockIndex* pindex = pindexBestHeader;
    CBlockIndex* pindexPrevStake = NULL;

    while (pindex && nStakesHandled < nPoSInterval)
    {

        if (pindexPrev->IsProofOfStake())
        {
            dStakeKernelsTriedAvg += GetDifficulty(pindexPrev) * 4294967296.0;;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindexPrev->nTime) : 0;
            pindexPrevStake = pindexPrev;
            nStakesHandled++;
        }

        pindexPrev = pindexPrev->pprev;
    }

   return nStakesTime ? dStakeKernelsTriedAvg / nStakesTime : 0;
}

double GetPoSKernelPS(CBlockIndex* pindexPrev)
{
    const Consensus::Params& consensusParams = Params().GetConsensus();

    return GetPoSKernelPS(pindexPrev, consensusParams);
}

double GetPoSKernelPS()
{
    const Consensus::Params& consensusParams = Params().GetConsensus();

    return GetPoSKernelPS(pindexBestHeader, consensusParams);
}

// get current inflation rate using average stake weight ~1.5-2.5% (measure of liquidity) PoST
double GetCurrentInflationRate(double nAverageWeight)
{
    double inflationRate = (17*(log(nAverageWeight/20)))/100;

    return inflationRate;
}

// get current interest rate by targeting for network stake dependent inflation rate PoST
double GetCurrentInterestRate(CBlockIndex* pindexPrev, const Consensus::Params& params)
{
    double nAverageWeight = GetAverageStakeWeight(pindexPrev);
    double inflationRate = GetCurrentInflationRate(nAverageWeight)/100;
    double interestRate = ((inflationRate*params.nInitialCoinSupply)/nAverageWeight)*100;

    return interestRate;
}

// get average stake weight of last 60 blocks PoST
double GetAverageStakeWeight(CBlockIndex* pindexPrev)
{
    double weightSum = 0, weightAve = 0;
    // XXX: Need to retrieve connman
    //if (g_connman.GetBestHeight() < 1)
    //    return weightAve;

    // Use cached weight if it's still valid
    if (pindexPrev->nHeight == nAverageStakeWeightHeightCached)
    {
        return dAverageStakeWeightCached;
    }
    nAverageStakeWeightHeightCached = pindexPrev->nHeight;


    CBlockIndex* currentBlockIndex = pindexPrev;
    int i;
    for (i = 0; currentBlockIndex && i < 60; i++)
    {
        double tempWeight = GetPoSKernelPS(currentBlockIndex);
        weightSum += tempWeight;
        currentBlockIndex = currentBlockIndex->pprev;
    }
    weightAve = (weightSum/i)+21;

    // Cache the stake weight value
    dAverageStakeWeightCached = weightAve;

    return weightAve;
}

// get stake time factored weight for reward and hash PoST
int64_t GetStakeTimeFactoredWeight(int64_t timeWeight, int64_t bnCoinDayWeight, CBlockIndex* pindexPrev)
{
    int64_t factoredTimeWeight;
    double weightFraction = (bnCoinDayWeight+1) / (GetAverageStakeWeight(pindexPrev));
    if (weightFraction*100 > 45)
    {
        factoredTimeWeight =  Params().GetConsensus().nStakeMinAge + 1;
    }
    else
    {
        double stakeTimeFactor = pow(cos((PI*weightFraction)),2.0);
        factoredTimeWeight = (stakeTimeFactor*timeWeight);
    }
    return factoredTimeWeight;
}


// miner's coin stake reward based on coin age spent (coin-days)
int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, CBlockIndex* pindex, const Consensus::Params& params)
{
    int64_t nSubsidy;

    // PoST
    if (pindex->nHeight+1 > params.PoSTHeight )
    {
        int64_t nInterestRate = GetCurrentInterestRate(pindex, params) * CENT;
        nSubsidy = params.nStakeMinAge * nInterestRate * 33 / (365 * 33 + 8);
    }
    else
    {
        double nNetworkWeight = GetPoSKernelPS(pindex);
        if(nNetworkWeight < 21)
        {
            nSubsidy = 0;
        }
        else
        {
            int64_t nInterestRate = ((17*(log(nNetworkWeight/20)))*10000);
            nSubsidy = (nCoinAge * (nInterestRate) * 33 / (365 * 33 + 8));
        }
    }
    if (gArgs.GetBoolArg("-printcreation", false))
        LogPrintf("%s: create=%s nCoinAge=%lld\n", __func__, FormatMoney(nSubsidy), nCoinAge);

    return nSubsidy + nFees;
}

// VeriCoin: total stake time spent in transaction that is accepted by the network, in the unit of coin-days.
// Only those coins meeting minimum age requirement counts. As those
// transactions not in main chain are not currently indexed so we
// might not find out about their coin age. Older transactions are
// guaranteed to be in main chain by sync-checkpoint. This rule is
// introduced to help nodes establish a consistent view of the coin
// age (trust score) of competing branches. PoSTistent view of the coin
// age (trust score) of competing branches
bool GetCoinAge(const CTransaction& tx, const CCoinsViewCache &view, uint64_t& nCoinAge, CBlockIndex* pindexPrev)
{
    arith_uint256 bnCentSecond = 0;  // coin age in the unit of cent-seconds
    arith_uint256 bnCoinDay = 0;
    nCoinAge = 0;

    if (tx.IsCoinBase())
        return true;

    // Transaction index is required to get to block header
    if (!g_txindex)
        return false;  // Transaction index not available

    for (const auto& txin : tx.vin)
    {
        // First try finding the previous transaction in database
        const COutPoint &prevout = txin.prevout;
        Coin coin;

        if (!view.GetCoin(prevout, coin))
            continue;  // previous transaction not in main chain
        if (tx.nTime < coin.nTime)
            return false;  // Transaction timestamp violation

        CDiskTxPos postx;
        CTransactionRef txPrev;
        if (g_txindex->FindTxPosition(prevout.hash, postx))
        {
            CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
            CBlockHeader header;
            try {
                file >> header;
                fseek(file.Get(), postx.nTxOffset, SEEK_CUR);
                file >> txPrev;
            } catch (std::exception &e) {
                return error("%s() : deserialize or I/O error in GetCoinAge()", __PRETTY_FUNCTION__);
            }
            if (txPrev->GetHash() != prevout.hash)
                return error("%s() : txid mismatch in GetCoinAge()", __PRETTY_FUNCTION__);

            if (header.GetBlockTime() + Params().GetConsensus().nStakeMinAge > tx.nTime)
                continue; // only count coins meeting min age requirement

            int64_t nValueIn = txPrev->vout[txin.prevout.n].nValue;
            int timeWeight = tx.nTime-txPrev->nTime;

            if (pindexPrev->nHeight+1 > Params().GetConsensus().PoSTHeight )
            {
                int64_t CoinDay = nValueIn * timeWeight / COIN / (24 * 60 * 60);
                int64_t factoredTimeWeight = GetStakeTimeFactoredWeight(timeWeight, CoinDay, pindexPrev);
                bnCoinDay += arith_uint256(nValueIn) * factoredTimeWeight / COIN / (24 * 60 * 60);
            }
            else
            {
                bnCentSecond += arith_uint256(nValueIn) * timeWeight / CENT;
            }

            if (gArgs.GetBoolArg("-printcoinage", false))
                LogPrintf("coin age nValueIn=%-12lld nTimeDiff=%d bnCentSecond=%s\n", nValueIn, timeWeight, bnCentSecond.ToString());
        }
        else
            return error("%s() : tx missing in tx index in GetCoinAge()", __PRETTY_FUNCTION__);
    }

    if ( pindexPrev->nHeight+1 <= Params().GetConsensus().PoSTHeight )
        bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);

    if (gArgs.GetBoolArg("-printcoinage", false))
        LogPrintf("coin age bnCoinDay=%s\n", bnCoinDay.ToString());

    nCoinAge = bnCoinDay.GetLow64();
    return true;
}

// ppcoin: sign block
typedef std::vector<unsigned char> valtype;
bool SignBlock(CBlock& block, const CWallet& keystore)
{
    std::vector<valtype> vSolutions;
    const CTxOut& txout = block.IsProofOfStake()? block.vtx[1]->vout[1] : block.vtx[0]->vout[0];

    if (Solver(txout.scriptPubKey, vSolutions) != TX_PUBKEY)
        return false;

    // Sign
    const valtype& vchPubKey = vSolutions[0];
    CKey key;
    if (!keystore.GetLegacyScriptPubKeyMan()->GetKey(CKeyID(Hash160(vchPubKey)), key))
        return false;
    if (key.GetPubKey() != CPubKey(vchPubKey))
        return false;
    return key.Sign(block.GetHash(), block.vchBlockSig, 0);
}

// ppcoin: check block signature
bool CheckBlockSignature(const CBlock& block)
{
    if (block.GetHash() == Params().GetConsensus().hashGenesisBlock)
        return block.vchBlockSig.empty();

    std::vector<valtype> vSolutions;
    const CTxOut& txout = block.IsProofOfStake()? block.vtx[1]->vout[1] : block.vtx[0]->vout[0];

    if (Solver(txout.scriptPubKey, vSolutions) != TX_PUBKEY)
        return false;

    const valtype& vchPubKey = vSolutions[0];
    CPubKey key(vchPubKey);
    if (block.vchBlockSig.empty())
        return false;
    return key.Verify(block.GetHash(), block.vchBlockSig);
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    // v0.3 protocol
    return (nTimeBlock == nTimeTx);
}
