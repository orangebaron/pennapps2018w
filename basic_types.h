#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <vector>
#include <cstddef>

namespace chain {
  struct Hash {
    char bytes[256]; //TODO.......
    bool operator>(Hash h);
    bool operator==(Hash h);
    Hash operator-(Hash h);
    Hash operator+(Hash h);
    void operator+=(Hash h);
    void operator-=(Hash h);
    void operator++(int);
    Hash(){}
    Hash(int x);
  };
  struct Key  {
    char bytes[256];
    Key(Hash h); //TODO
    Key();
  };
  struct Sig  { char bytes[72]; Key *signer; bool check(); }; //TODO: signer is gonna be sitting in the heap, make a handy little destructor function when no one is using it
  struct Code { std::vector<char> code; };
  typedef unsigned long Coins;
  typedef unsigned long Time;
}

#endif
