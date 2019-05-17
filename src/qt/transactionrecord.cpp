





#include "transactionrecord.h"

#include "base58.h"
#include "obfuscation.h"
#include "swifttx.h"
#include "timedata.h"
#include "wallet.h"

#include <stdint.h>

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction(const CWalletTx& wtx)
{
    if (wtx.IsCoinBase()) {
        
        if (!wtx.IsInMainChain()) {
            return false;
        }
    }
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const CWallet* wallet, const CWalletTx& wtx)
{
    QList<TransactionRecord> parts;
    int64_t nTime = wtx.GetComputedTxTime();
    CAmount nCredit = wtx.GetCredit(ISMINE_ALL);
    CAmount nDebit = wtx.GetDebit(ISMINE_ALL);
    CAmount nNet = nCredit - nDebit;
    uint256 hash = wtx.GetHash();
    std::map<std::string, std::string> mapValue = wtx.mapValue;

    if (wtx.IsCoinStake()) {
        TransactionRecord sub(hash, nTime);
        CTxDestination address;
        if (!ExtractDestination(wtx.vout[1].scriptPubKey.HasOpVmHashState() ? wtx.vout[2].scriptPubKey : wtx.vout[1].scriptPubKey, address))
            return parts;

        if (!IsMine(*wallet, address)) {
            
            for (unsigned int i = 1; i < wtx.vout.size(); i++) {
                CTxDestination outAddress;
                if (ExtractDestination(wtx.vout[i].scriptPubKey, outAddress)) {
                    if (IsMine(*wallet, outAddress)) {
                        isminetype mine = wallet->IsMine(wtx.vout[i]);
                        sub.involvesWatchAddress = mine & ISMINE_WATCH_ONLY;
                        sub.type = TransactionRecord::MNReward;
                        sub.address = CBitcoinAddress(outAddress).ToString();
                        sub.credit = wtx.vout[i].nValue;
                    }
                }
            }
        } else {
            
            isminetype mine = wallet->IsMine(wtx.vout[1]);
            sub.involvesWatchAddress = mine & ISMINE_WATCH_ONLY;
            sub.type = TransactionRecord::StakeMint;
            sub.address = CBitcoinAddress(address).ToString();
            sub.credit = nNet;
        }
        parts.append(sub);
    } else if (wtx.IsZerocoinSpend()) {
        
        libzerocoin::CoinSpend zcspend = TxInToZerocoinSpend(wtx.vin[0]);
        bool fSpendFromMe = wallet->IsMyZerocoinSpend(zcspend.getCoinSerialNumber());

        
        bool fFeeAssigned = false;
        for (const CTxOut txout : wtx.vout) {
            
            if (txout.IsZerocoinMint()) {
                
                if (!fSpendFromMe)
                    continue;

                TransactionRecord sub(hash, nTime);
                sub.type = TransactionRecord::ZerocoinSpend_Change_zUlo;
                sub.address = mapValue["zerocoinmint"];
                sub.debit = -txout.nValue;
                if (!fFeeAssigned) {
                    sub.debit -= (wtx.GetZerocoinSpent() - wtx.GetValueOut());
                    fFeeAssigned = true;
                }
                sub.idx = parts.size();
                parts.append(sub);
                continue;
            }

            string strAddress = "";
            CTxDestination address;
            if (ExtractDestination(txout.scriptPubKey, address))
                strAddress = CBitcoinAddress(address).ToString();

            
            isminetype mine = wallet->IsMine(txout);
            if (mine) {
                TransactionRecord sub(hash, nTime);
                sub.type = (fSpendFromMe ? TransactionRecord::ZerocoinSpend_FromMe : TransactionRecord::RecvFromZerocoinSpend);
                sub.debit = txout.nValue;
                sub.address = mapValue["recvzerocoinspend"];
                if (strAddress != "")
                    sub.address = strAddress;
                sub.idx = parts.size();
                parts.append(sub);
                continue;
            }

            
            if (!fSpendFromMe)
                continue;

            
            TransactionRecord sub(hash, nTime);
            sub.debit = -txout.nValue;
            sub.type = TransactionRecord::ZerocoinSpend;
            sub.address = mapValue["zerocoinspend"];
            if (strAddress != "")
                sub.address = strAddress;
            sub.idx = parts.size();
            parts.append(sub);
        }
    } else if (nNet > 0 || wtx.IsCoinBase()) {
        
        
        
        BOOST_FOREACH (const CTxOut& txout, wtx.vout) {
            isminetype mine = wallet->IsMine(txout);
            if (mine) {
                TransactionRecord sub(hash, nTime);
                CTxDestination address;
                sub.idx = parts.size(); 
                sub.credit = txout.nValue;
                sub.involvesWatchAddress = mine & ISMINE_WATCH_ONLY;
                if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*wallet, address)) {
                    
                    sub.type = TransactionRecord::RecvWithAddress;
                    sub.address = CBitcoinAddress(address).ToString();
                } else {
                    
                    sub.type = TransactionRecord::RecvFromOther;
                    sub.address = mapValue["from"];
                }
                if (wtx.IsCoinBase()) {
                    
                    sub.type = TransactionRecord::Generated;
                }

                parts.append(sub);
            }
        }
    } else {
        bool fAllFromMeDenom = true;
        int nFromMe = 0;
        bool involvesWatchAddress = false;
        isminetype fAllFromMe = ISMINE_SPENDABLE;
        BOOST_FOREACH (const CTxIn& txin, wtx.vin) {
            if (wallet->IsMine(txin)) {
                fAllFromMeDenom = fAllFromMeDenom && wallet->IsDenominated(txin);
                nFromMe++;
            }
            isminetype mine = wallet->IsMine(txin);
            if (mine & ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if (fAllFromMe > mine) fAllFromMe = mine;
        }

        isminetype fAllToMe = ISMINE_SPENDABLE;
        bool fAllToMeDenom = true;
        int nToMe = 0;
        BOOST_FOREACH (const CTxOut& txout, wtx.vout) {
            if (wallet->IsMine(txout)) {
                fAllToMeDenom = fAllToMeDenom && wallet->IsDenominatedAmount(txout.nValue);
                nToMe++;
            }
            isminetype mine = wallet->IsMine(txout);
            if (mine & ISMINE_WATCH_ONLY) involvesWatchAddress = true;
            if (fAllToMe > mine) fAllToMe = mine;
        }

        if (fAllFromMeDenom && fAllToMeDenom && nFromMe * nToMe) {
            parts.append(TransactionRecord(hash, nTime, TransactionRecord::ObfuscationDenominate, "", -nDebit, nCredit));
            parts.last().involvesWatchAddress = false; 
        } else if (fAllFromMe && fAllToMe) {
            
            
            

            TransactionRecord sub(hash, nTime);
            
            sub.type = TransactionRecord::SendToSelf;
            sub.address = "";

            if (mapValue["DS"] == "1") {
                sub.type = TransactionRecord::Obfuscated;
                CTxDestination address;
                if (ExtractDestination(wtx.vout[0].scriptPubKey, address)) {
                    
                    sub.address = CBitcoinAddress(address).ToString();
                } else {
                    
                    sub.address = mapValue["to"];
                }
            } else {
                for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++) {
                    const CTxOut& txout = wtx.vout[nOut];
                    sub.idx = parts.size();

                    if (wallet->IsCollateralAmount(txout.nValue)) sub.type = TransactionRecord::ObfuscationMakeCollaterals;
                    if (wallet->IsDenominatedAmount(txout.nValue)) sub.type = TransactionRecord::ObfuscationCreateDenominations;
                    if (nDebit - wtx.GetValueOut() == OBFUSCATION_COLLATERAL) sub.type = TransactionRecord::ObfuscationCollateralPayment;
                }
            }

            CAmount nChange = wtx.GetChange();

            sub.debit = -(nDebit - nChange);
            sub.credit = nCredit - nChange;
            parts.append(sub);
            parts.last().involvesWatchAddress = involvesWatchAddress; 
        } else if (fAllFromMe) {
            
            
            
            CAmount nTxFee = nDebit - wtx.GetValueOut();

            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++) {
                const CTxOut& txout = wtx.vout[nOut];
                TransactionRecord sub(hash, nTime);
                sub.idx = parts.size();
                sub.involvesWatchAddress = involvesWatchAddress;

                if (wallet->IsMine(txout)) {
                    
                    
                    continue;
                }

                CTxDestination address;
                if (ExtractDestination(txout.scriptPubKey, address)) {
                    
                    sub.type = TransactionRecord::SendToAddress;
                    sub.address = CBitcoinAddress(address).ToString();
                } else if (txout.IsZerocoinMint()){
                    sub.type = TransactionRecord::ZerocoinMint;
                    sub.address = mapValue["zerocoinmint"];
                } else {
                    
                    sub.type = TransactionRecord::SendToOther;
                    sub.address = mapValue["to"];
                }

                if (mapValue["DS"] == "1") {
                    sub.type = TransactionRecord::Obfuscated;
                }

                CAmount nValue = txout.nValue;
                
                if (nTxFee > 0) {
                    nValue += nTxFee;
                    nTxFee = 0;
                }
                sub.debit = -nValue;

                parts.append(sub);
            }
        } else {
            
            
            
            parts.append(TransactionRecord(hash, nTime, TransactionRecord::Other, "", nNet, 0));
            parts.last().involvesWatchAddress = involvesWatchAddress;
        }
    }

    return parts;
}

void TransactionRecord::updateStatus(const CWalletTx& wtx)
{
    AssertLockHeld(cs_main);
    

    
    CBlockIndex* pindex = NULL;
    BlockMap::iterator mi = mapBlockIndex.find(wtx.hashBlock);
    if (mi != mapBlockIndex.end())
        pindex = (*mi).second;

    
    status.sortKey = strprintf("%010d-%01d-%010u-%03d",
        (pindex ? pindex->nHeight : std::numeric_limits<int>::max()),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived,
        idx);
    status.countsForBalance = wtx.IsTrusted() && !(wtx.GetBlocksToMaturity() > 0);
    status.depth = wtx.GetDepthInMainChain();
    status.cur_num_blocks = chainActive.Height();
    status.cur_num_ix_locks = nCompleteTXLocks;

    if (!IsFinalTx(wtx, chainActive.Height() + 1)) {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD) {
            status.status = TransactionStatus::OpenUntilBlock;
            status.open_for = wtx.nLockTime - chainActive.Height();
        } else {
            status.status = TransactionStatus::OpenUntilDate;
            status.open_for = wtx.nLockTime;
        }
    }
    
    else if (type == TransactionRecord::Generated || type == TransactionRecord::StakeMint || type == TransactionRecord::MNReward) {
        if (wtx.GetBlocksToMaturity() > 0) {
            status.status = TransactionStatus::Immature;

            if (wtx.IsInMainChain()) {
                status.matures_in = wtx.GetBlocksToMaturity();

                
                if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                    status.status = TransactionStatus::MaturesWarning;
            } else {
                status.status = TransactionStatus::NotAccepted;
            }
        } else {
            status.status = TransactionStatus::Confirmed;
        }
    } else {
        if (status.depth < 0) {
            status.status = TransactionStatus::Conflicted;
        } else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0) {
            status.status = TransactionStatus::Offline;
        } else if (status.depth == 0) {
            status.status = TransactionStatus::Unconfirmed;
        } else if (status.depth < RecommendedNumConfirmations) {
            status.status = TransactionStatus::Confirming;
        } else {
            status.status = TransactionStatus::Confirmed;
        }
    }
}

bool TransactionRecord::statusUpdateNeeded()
{
    AssertLockHeld(cs_main);
    return status.cur_num_blocks != chainActive.Height() || status.cur_num_ix_locks != nCompleteTXLocks;
}

QString TransactionRecord::getTxID() const
{
    return QString::fromStdString(hash.ToString());
}

int TransactionRecord::getOutputIndex() const
{
    return idx;
}
