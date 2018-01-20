#ifndef CHAIN_H
#define CHAIN_H

#include <map>
#include <exception>
#include <vector>
#include "basic_types.h"

namespace chain {
  enum class TxnType { GiveTxn,CallTxn,MakeContractTxn };
  struct TxnPtrWrapper {
    TxnType type;
    void    *txn;
  };
  struct State {
    Key   key;
    Coins money;
    Code  code;
    int   storage[];
  };
  typedef std::map<Key,State> States;

  struct GiveTxn {
    Key   send;
    Key   recieve;
    Coins amt;
    void run(States &states, Hash &difficulty);
    void unrun(States &states, Hash &difficulty);
    bool valid(States &states);
  };
  struct CallTxn {
    GiveTxn giveTxn;
    char    args[sizeof(GiveTxn)];
    void run(States &states, Hash &difficulty);
    void unrun(States &states, Hash &difficulty);
    bool valid(States &states);
  };
  struct MakeContractTxn {
    Key  send;
    Code code;
    void run(States &states, Hash &difficulty);
    void unrun(States &states, Hash &difficulty);
    bool valid(States &states);
    int getSize();
  };
  struct Block {
    std::vector<TxnPtrWrapper> txns;
    Block    *approved[2];
    Time     time;
    long int nonce;
    Sig      sigs[];
    bool     validityChecked;

    bool runCheckValid(States &states);
    void unrun(States &states);
    std::vector<char> getBytes();
  };

  struct TxnFailException: public std::exception {};
}

#endif
