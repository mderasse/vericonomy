// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <arith_uint256.h>
#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <clientversion.h>
#include <tinyformat.h>
#include <util/system.h>
#include <util/strencodings.h>

#include <assert.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);

    if (IsVericoin())
        txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(42) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    else
        txNew.vin[0].scriptSig = CScript() << 0 << CScriptNum(999) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));

    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    if (IsVericoin())
        txNew.nTime = nTime; /// Vericoin magic constant.
    else
        txNew.nTime = nTime; /// Verium magic constant.

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "9 May 2014 US politicians can accept bitcoin donations";
    if (IsVerium())
        pszTimestamp = "VeriCoin block 1340292";

    const CScript genesisOutputScript = CScript();
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;

        // COMMON SETTING FOR VERIUM & VERICOIN

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000"); // 623950

        /**
        * The message start string is designed to be unlikely to occur in normal data.
        * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        * a large 32-bit integer with any alignment.
        */
        pchMessageStart[0] = 0x70;
        pchMessageStart[1] = 0x35;
        pchMessageStart[2] = 0x22;
        pchMessageStart[3] = 0x05;

        // Address configure
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,70);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,132);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128+70);
        base58Prefixes[EXT_PUBLIC_KEY] = {0xE3, 0xCC, 0xBB, 0x92};
        base58Prefixes[EXT_SECRET_KEY] = {0xE3, 0xCC, 0xAE, 0x01};
        // human readable prefix to bench32 address
        bech32_hrp = "vry";

        // We are on main net
        consensus.fPowNoRetargeting = false;
        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        /** VERICOIN **/
        if (IsVericoin())
        {
            consensus.BIP34Height = 227931;
            consensus.BIP65Height = 4000000;
            consensus.BIP66Height = 4000000;
            consensus.CSVHeight = 4000000;

            consensus.NextTargetV2Height = 38424; // Moving From GetNextTargetRequiredV1 to V2
            consensus.PoSTHeight = 608100; // Start PoST
            consensus.PoSHeight = 20160; // Start PoS for Vericoin

            consensus.VIP1Height = 0; // VIP1 is for Verium

            consensus.nTargetTimespan = 16 * 60;
            consensus.nCoinbaseMaturity = 500;
            consensus.nInitialCoinSupply = 26751452;

            // PoW Setting
            consensus.powLimit = arith_uint256(~arith_uint256(0) >> 20);
            consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
            consensus.nPowTargetSpacing = 60;

            // PoS Setting
            consensus.posLimit = ~arith_uint256(0) >> 20;
            consensus.nStakeTargetSpacing = 60;
            consensus.nStakeMinAge = 8 * 60 * 60; // 8 hours
            consensus.nModifierInterval = 10 * 60;

            nDefaultPort = 58684;
            m_assumed_blockchain_size = 3;
            m_assumed_chain_state_size = 10;

            genesis = CreateGenesisBlock(1399690945, 612416, consensus.powLimit.GetCompact(), 1, 2500 * COIN);
            consensus.hashGenesisBlock = genesis.GetHash();
            assert(consensus.hashGenesisBlock == uint256S("0x000004da58a02be894a6c916d349fe23cc29e21972cafb86b5d3f07c4b8e6bb8"));
            assert(genesis.hashMerkleRoot == uint256S("0x60424046d38de827de0ed1a20a351aa7f3557e3e1d3df6bfb34a94bc6161ec68"));

            vSeeds.emplace_back("seed.vrc.vericonomy.com");

            vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_vrc_main, pnSeed6_vrc_main + ARRAYLEN(pnSeed6_vrc_main));

            // XXX: Add checkpoint and chainTxData before release
            checkpointData = {
                {
                }
            };

            chainTxData = ChainTxData{
                // Data as of block ff65454ebdf1d89174bec10a3c016db92f7b1d9a4759603472842f254be8d7b3 (height 504051).
                1591618067, // * UNIX timestamp of last known number of transactions
                1797921,    // * total number of transactions between genesis and that timestamp
                            //   (the tx=... number in the ChainStateFlushed debug.log lines)
                0.00730216  // * estimated number of transactions per second after that timestamp
                            // 1797921/(1591618067-1345400356) = 0.00730216
            };
        }
        if (IsVerium())
        {
            consensus.BIP34Height = 0;
            consensus.BIP65Height = 550000;
            consensus.BIP66Height = 550000;
            consensus.CSVHeight = 550000;

            consensus.NextTargetV2Height = 0; // No use for verium
            consensus.PoSTHeight = 0; // No use for verium
            consensus.PoSHeight = 0; // No use for verium

            consensus.VIP1Height = 520000; // Change Min Fee

            consensus.nTargetTimespan = 0; // No use for verium
            consensus.nCoinbaseMaturity = 100;

            // PoW Setting
            consensus.powLimit = ~arith_uint256(0) >> 11;
            consensus.nPowTargetTimespan = 2 * 24 * 60 * 60; // two days
            consensus.nPowTargetSpacing =  5 * 60;  // not used for consensus in Verium as it's variable, but used to indicate age of data

            // PoS Setting
            consensus.posLimit = ~arith_uint256(0) >> 20; // No use for verium
            consensus.nStakeTargetSpacing = 0; // No use for verium
            consensus.nStakeMinAge = 0; // No use for verium
            consensus.nModifierInterval = 0; // No use for verium

            nDefaultPort = 36988;
            m_assumed_blockchain_size = 1;
            m_assumed_chain_state_size = 4;

            genesis = CreateGenesisBlock(1472669240, 233180, consensus.powLimit.GetCompact(), 1, 2500 * COIN);
            consensus.hashGenesisBlock = genesis.GetHash();
            assert(consensus.hashGenesisBlock == uint256S("0x8232c0cf3bd7e05546e3d7aaaaf89fed8bc97c4df1a8c95e9249e13a2734932b"));
            assert(genesis.hashMerkleRoot == uint256S("0x925e430072a1f39b530fc79db162e29433ab0ea266a99c8cab4f03001dc9faa9"));

            vSeeds.emplace_back("seed.vrm.vericonomy.com");

            vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_vrm_main, pnSeed6_vrm_main + ARRAYLEN(pnSeed6_vrm_main));

            // XXX: Add checkpoint and chainTxData before release
            checkpointData = {
                {
                    {     1, uint256S("0x3f2566fc0abcc9b2e26c737d905ff3e639a49d44cd5d11d260df3cfb62663012")},
                    {  1500, uint256S("0x0458cc7c7093cea6e78eed03a8f57d0eed200aaf5171eea82e63b8e643891cce")},
                    {100000, uint256S("0x0510c6cb8c5a2a5437fb893853f10e298654361a05cf611b1c54c1750dfbdad6")},
                }
            };

            chainTxData = ChainTxData{
                /* nTime    */ 1499513240,
                /* nTxCount */ 36540,
                /* dTxRate  */ 0.0013,
            };
        }
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {};
};
/**
 * Regression test
 */

class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {}
};

static std::unique_ptr<const CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<const CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    // else if (chain == CBaseChainParams::TESTNET)
    //     return std::unique_ptr<CChainParams>(new CTestNetParams());
    // else if (chain == CBaseChainParams::REGTEST)
    //     return std::unique_ptr<CChainParams>(new CRegTestParams(gArgs));
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}
