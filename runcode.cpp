#ifndef RUNCODE_CPP
#define RUNCODE_CPP

#include <vector>
#include "chain.h"

namespace chain {
  #define checkArgs(argLen) if(location+argLen >= memory.size()) { stayInLoop = false; break; }
  #define applyOp1() *(int*)(memory.data()+location+1) = *(int*)(memory.data()+location+1)
  #define applyOp2(size) *(int*)(memory.data()+location+1+size)

  void RunCode(States &states, Code code) {
    std::vector<char> memory(code.bytes,code.bytes+code.size);
    memory.insert(memory.end(),0,65536-memory.size());
    size_t location = 0;
    bool stayInLoop = true;
    while (stayInLoop) { //TODO: break if too much difficulty used
      switch (memory[location]) {
      case 0: //STOP
        stayInLoop = false;
        break;
      case 1: //ADD
        checkArgs(8);
        applyOp1()+applyOp2(4);
        break;
      }
    }
  }
}

#endif
