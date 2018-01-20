#include "basic_types.h"
#include "chain.h"

using namespace chain;

#define addNormDifficulty(a) difficulty = add<Hash>(difficulty, intToBytes<Hash>(65*a))

//TODO: make a new entry in states when a transaction affects an address for the first time

bool GiveTxn::valid(States &states) { return (states[this->send].money >= this->amt); }
void GiveTxn::run(States &states, Hash &difficulty) {
  if (states[this->send].money < this->amt) throw TxnFailException();
  states[this->send].money -= this->amt;
  states[this->recieve].money += this->amt;
  addNormDifficulty(1);
}
bool CallTxn::valid(States &states) { return true; }
void CallTxn::run(States &states, Hash &difficulty) {
  addNormDifficulty(1);
}
bool MakeContractTxn::valid(States &states) { return true; }
void MakeContractTxn::run(States &states, Hash &difficulty) {
  states[this->send].code = this->code;
  addNormDifficulty(this->getSize());
}
bool Block::runCheckValid(States &states) {
  if (this->validityChecked) return true;
  for (int i=0;i<2;i++) if (!this->approved[i]->runCheckValid(states)) { this->approved[i]->unrun(states); return false; }
  Hash totDiff = intToBytes<Hash>(0);
  for (auto i = this->txns.begin(); i < this->txns.end(); i++) {
    bool valid;
    switch(i->type) {
    case TxnType::GiveTxn:
      valid = ((GiveTxn*)(i->txn))->valid(states);
      valid = valid && states[((GiveTxn*)(i->txn))->send].code.size == 0;
      break;
    case TxnType::CallTxn:
      valid = ((CallTxn*)(i->txn))->valid(states);
      break;
    case TxnType::MakeContractTxn:
      valid = ((MakeContractTxn*)(i->txn))->valid(states);
      break;
    }
    if (valid) {
      switch(i->type) {
      case TxnType::GiveTxn:
        ((GiveTxn*)(i->txn))->run(states,totDiff);
        break;
      case TxnType::CallTxn:
        ((CallTxn*)(i->txn))->run(states,totDiff);
        break;
      case TxnType::MakeContractTxn:
        ((MakeContractTxn*)(i->txn))->run(states,totDiff);
        break;
      }
    } else { //FUCK GO BACK
      for (;i >= this->txns.begin();i--) {
        switch(i->type) {
        case TxnType::GiveTxn:
          ((GiveTxn*)(i->txn))->unrun(states,totDiff);
          break;
        case TxnType::CallTxn:
          ((CallTxn*)(i->txn))->unrun(states,totDiff);
          break;
        case TxnType::MakeContractTxn:
          ((MakeContractTxn*)(i->txn))->unrun(states,totDiff);
          break;
        }
      }
      return false;
    }
  } //TODO: check signatures
  if (greater<Hash>(totDiff,totDiff)) { //FUCK GO BACK
    for (auto i = this->txns.end()-1;i >= this->txns.begin();i--) {
      switch(i->type) {
      case TxnType::GiveTxn:
        ((GiveTxn*)(i->txn))->unrun(states,totDiff);
        break;
      case TxnType::CallTxn:
        ((CallTxn*)(i->txn))->unrun(states,totDiff);
        break;
      case TxnType::MakeContractTxn:
        ((MakeContractTxn*)(i->txn))->unrun(states,totDiff);
        break;
      }
    }
    return false;
  }
  this->validityChecked = true;
  return true;
}
