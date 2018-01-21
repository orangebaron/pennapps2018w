#ifndef CHAIN_H
#define CHAIN_H

#include <map>
#include <exception>
#include <vector>
#include "basic_types.h"

namespace chain {
  const Time acceptTime = 20;
  enum class TxnType { GiveTxn,CallTxn,MakeContractTxn };
  struct TxnPtrWrapper {
    TxnType type;
    void    *txn;
  };
  struct State {
    Key   key;
    Coins money;
    Code  code;
    std::map<int,int> storage;
  };
  typedef std::map<Key,State> States;

  struct Txn {
    Key  send;
    virtual void run(States &states, Hash &difficulty);
    virtual void unrun(States &states);
    virtual bool valid(States &states, Hash &difficulty);
    Txn(Key send):send(send){}
  };

  struct GiveTxn: public Txn {
    Key   recieve;
    Coins amt;
    void run(States &states, Hash &difficulty);
    void unrun(States &states);
    bool valid(States &states, Hash &difficulty);
    GiveTxn(Key send,Key recieve,Coins amt):Txn(send),recieve(recieve),amt(amt){}
  };
  struct CallTxn: public Txn {
    Key   recieve;
    Coins amt;
    char  args[sizeof(GiveTxn)];
    void  *block;
    void run(States &states, Hash &difficulty);
    void unrun(States &states){}
    bool valid(States &states, Hash &difficulty);
    CallTxn(Key send,Key recieve,Coins amt,void *block):Txn(send),recieve(recieve),amt(amt),block(block){}
  };
  struct MakeContractTxn: public Txn {
    Code code;
    void run(States &states, Hash &difficulty);
    void unrun(States &states);
    bool valid(States &states, Hash &difficulty);
    MakeContractTxn(Key send,Code code):Txn(send),code(code){}
    int getSize();
  };
  struct WriteStorageTxn: public Txn {
    int loc;
    int val;
    void run(States &states, Hash &difficulty);
    void unrun(States &states);
    bool valid(States &states, Hash &difficulty) { return true; }
    WriteStorageTxn(Key send,int loc, int val):Txn(send),loc(loc),val(val){}
  private:
    int before;
  };
  struct Block {
    std::vector<Txn> txns;
    Block    *approved[2];
    Time     time;
    long int nonce;
    Hash     beforeHash;
    std::vector<Sig> sigs;
    bool     validityChecked;
    Hash     hash;
    int      numContractsMade;

    bool runCheckValid(States &states);
    void unrun(States &states);
  };
  struct Blockchain {
    std::vector<Block*> blocks;
    States states;
    Blockchain(std::vector<Block*> blocks);
    bool validInitCheck();
    bool addBlock(Block *b);
  };
}

#endif
