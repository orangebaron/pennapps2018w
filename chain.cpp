#include "basic_types.h"
#include "chain.h"
#include "run_code.cpp"

using namespace chain;

#define addNormDifficulty(a) difficulty-=Hash(65*a)

//TODO: make a new entry in states when a transaction affects an address for the first time

bool GiveTxn::valid(States &states, Hash &difficulty) { return (states[this->send].money >= this->amt); }
void GiveTxn::run(States &states, Hash &difficulty) {
  states[this->send].money -= this->amt;
  states[this->recieve].money += this->amt;
  addNormDifficulty(1);
}
void GiveTxn::unrun(States &states) {
  states[this->send].money += this->amt;
  states[this->recieve].money -= this->amt;
}
void CallTxn::run(States &states, Hash &difficulty) {
  addNormDifficulty(1);
}
bool MakeContractTxn::valid(States &states, Hash &difficulty) { return true; }
void MakeContractTxn::run(States &states, Hash &difficulty) {
  states[this->send].code = this->code;
  addNormDifficulty(this->getSize());
}
void MakeContractTxn::unrun(States &states) {
  states[this->send].code = this->code;
}
void WriteStorageTxn::run(States &states, Hash &difficulty) {
  this->before=states[this->send].storage[this->loc];
  states[this->send].storage[this->loc] = this->val;
}
void WriteStorageTxn::unrun(States &states) {
  states[this->send].storage[this->loc] = this->before;
}
void Block::unrun(States &states) { for (auto i=this->txns.end()-1; i >= this->txns.begin(); i--) i->unrun(states); }
bool Block::runCheckValid(States &states) {
  if (this->validityChecked) return true;
  for (int i=0;i<2;i++) if (!this->approved[i]->runCheckValid(states)) { this->approved[i]->unrun(states); return false; }
  Hash totDiff = this->hash;
  std::vector<Key> keys;
  for (auto i = this->txns.begin(); i < this->txns.end(); i++) {
    bool valid = i->valid(states,totDiff) && states[i->send].code.code.size() == 0;
    if (valid) {
      i->run(states,totDiff);
      bool notInSigs = true;
      for (auto j = keys.begin();j<keys.end();j++) if (i->send==*j) {notInSigs = false;break;}
      if (notInSigs) keys.push_back(i->send);

      if (totDiff==Hash(0)) {
        for (i--; i >= this->txns.begin(); i--) i->unrun(states);
        return false;
      }
    } else { //FUCK GO BACK
      for (i--; i >= this->txns.begin(); i--) i->unrun(states);
      return false;
    }
  }
  if (this->hash>totDiff || sigs.size() != this->sigs.size()) { //FUCK GO BACK
    for (auto i = this->txns.end()-1; i >= this->txns.begin(); i--) i->unrun(states);
    return false;
  }
  for (auto j = this->sigs.begin();j<sigs.end();j++) {
    bool found = false;
    for (auto k = keys.begin();k<keys.end();k++) {
      if (*k==*j->signer) { found=true;break; }
    }
    if (!found || !(*j).valid()) {
      for (auto i = this->txns.end()-1; i >= this->txns.begin(); i--) i->unrun(states);
      return false;
    }
  }
  this->validityChecked = true;
  return true;
}
