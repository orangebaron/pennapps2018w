#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <vector>
#include <cstddef>

namespace chain {
  struct Key  { char bytes[256]; };
  struct Hash { char bytes[256]; };
  struct Sig  { char bytes[72]; Key *signer; }; //TODO: signer is gonna be sitting in the heap, make a handy little destructor function when no one is using it
  struct Code { char bytes[]; size_t size; };
  typedef unsigned long Coins;
  typedef unsigned long Time;
  template<class c> c add(c a,c b);
  template<class c> c intToBytes(int a);
  template<class c> bool greater(c a,c b);
  Hash hashBytes(std::vector<char> c);
}

#endif
