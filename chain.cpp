#include "basic_types.h"
#include "chain.h"
#include "run_code.cpp"
#include "bigint.cpp"
#include <vector>
#include <algorithm>

using namespace chain;

#define addNormDifficulty(a) difficulty-=Hash(65*a)

//TODO: make a new entry in states when a transaction affects an address for the first time

bool GiveTxn::valid(States &states, Hash &difficulty) { return (states[send].money >= amt); }
void GiveTxn::run(States &states, Hash &difficulty) {
  states[send].money -= amt;
  states[recieve].money += amt;
  addNormDifficulty(1);
}
void GiveTxn::unrun(States &states) {
  states[send].money += amt;
  states[recieve].money -= amt;
}
void CallTxn::run(States &states, Hash &difficulty) {
  addNormDifficulty(1);
}
bool MakeContractTxn::valid(States &states, Hash &difficulty) { return true; }
void MakeContractTxn::run(States &states, Hash &difficulty) {
  states[send].code = code;
  addNormDifficulty(getSize());
}
void MakeContractTxn::unrun(States &states) {
  states[send].code = code;
}
void WriteStorageTxn::run(States &states, Hash &difficulty) {
  before=states[send].storage[loc];
  states[send].storage[loc] = val;
}
void WriteStorageTxn::unrun(States &states) {
  states[send].storage[loc] = before;
}
void Block::unrun(States &states) { for (auto i=txns.end()-1; i >= txns.begin(); i--) i->unrun(states); }
bool Block::runCheckValid(States &states) {
  if (validityChecked) return true;
  for (int i=0;i<2;i++) if (
    time-approved[i]->time > acceptTime*2
  ) return false;
  bigint hash(0);
  bigint bignum = bigint(1);
  for (int i=0;i<256;i++) bignum *= 2;
  for (int i=sizeof(beforeHash.bytes)-1;i>=0;i--) {
    hash=hash/255;
    hash+=beforeHash.bytes[i];
  }
  hash = bignum/(hash+1);
  Hash totDiff(0);
  for (int i=sizeof(beforeHash.bytes)/sizeof(int);i>=0;i--) {
    ((int*)totDiff.bytes)[i]=hash.a[0];
    hash/=255;
  }
  std::vector<Key> keys;
  for (auto i = txns.begin(); i < txns.end(); i++) {
    bool valid = i->valid(states,totDiff) && states[i->send].code.code.size() == 0;
    if (valid) {
      i->run(states,totDiff);
      bool notInSigs = true;
      for (auto j = keys.begin();j<keys.end();j++) if (i->send==*j) {notInSigs = false;break;}
      if (notInSigs) keys.push_back(i->send);

      if (totDiff==Hash(0)) {
        for (i--; i >= txns.begin(); i--) i->unrun(states);
        return false;
      }
    } else { //FUCK GO BACK
      for (i--; i >= txns.begin(); i--) i->unrun(states);
      return false;
    }
  }
  if (this->hash>totDiff || sigs.size() != sigs.size()) { //FUCK GO BACK
    for (auto i = txns.end()-1; i >= txns.begin(); i--) i->unrun(states);
    return false;
  }
  for (auto j = sigs.begin();j<sigs.end();j++) {
    bool found = false;
    for (auto k = keys.begin();k<keys.end();k++) {
      if (*k==*j->signer) { found=true;break; }
    }
    if (!found || !(*j).valid(beforeHash.bytes,sizeof(beforeHash))) {
      for (auto i = txns.end()-1; i >= txns.begin(); i--) i->unrun(states);
      return false;
    }
  }
  validityChecked = true;
  return true;
}

Blockchain::Blockchain(std::vector<Block*> blocks):blocks(blocks) {
  std::sort(blocks.begin(),blocks.end(),[](Block *a,Block *b){
    return a->time>b->time;
  });
}
bool Blockchain::validInitCheck() {
  for (auto i=blocks.begin();i<blocks.end();i++)
    if (!(*i)->runCheckValid(states))
      return false;
  return true;
}
bool Blockchain::addBlock(Block *b) {
  States s;
  blocks.push_back(b);
  std::sort(blocks.begin(),blocks.end(),[](Block *a,Block *b){
    return a->time>b->time;
  });
  auto i=blocks.end()-1;
  for (;*i!=b;i--) {
    (*i)->unrun(states);
  }
  auto backupi = i;
  for (;i<blocks.end();i++) {
    if (!(*i)->runCheckValid(states)) {
      for (;i>=backupi;i--) (*i)->unrun(states);
      blocks.erase(i);
      for (;i<blocks.end();i++) (*i)->runCheckValid(states);
      return false;
    }
  }
  return true;
}
