#ifndef RUNCODE_CPP
#define RUNCODE_CPP

#include <vector>
#include <array>
#include "chain.h"

namespace chain {
  #define memsize 65536
  #define checkArgs(argLen) if(location+argLen >= memsize) { stayInLoop = false; break; }
  #define memAt(loc,cl) *(cl*)(memory.data()+location+loc+1)
  #define memAtNonLocal(loc,cl) *(cl*)(memory.data()+loc+1)
  #define ptrAt(loc,cl) memAtNonLocal(memAt(loc,loctype),cl)
  #define checkPtr(loc,cl) checkArgs(loc); if(memAt(loc,short)+sizeof(cl) > memsize) { stayInLoop = false; break; }
  #define applyOp(op,size) checkArgs(size); op; location += size + 1; break;
  #define applyOpNoAdd(op,size) checkArgs(size); op; break;
  #define unaryOp(op,cl) memAt(0,cl) = op memAt(0,cl)
  #define binaryOp(op,cl) memAt(0,cl) = memAt(0,cl) op memAt(sizeof(cl),cl)
  #define setOp(op,cl) checkPtr(0,cl); ptrAt(0,cl) = op
  #define mathOp(op,cl) applyOp(binaryOp(op,cl),sizeof(cl)*2);
  #define copyBytesOp(bytes,size)\
    checkArgs(sizeof(loctype));\
    for (loctype i = 0;i < size;i++) memAtNonLocal(memAt(0,loctype)+i,char) = bytes[i];\
    location += sizeof(loctype) + 1;\
    break;
  #define revert() for (auto i = txns.end()-1;i>=txns.begin();i--) (*i)->unrun(states); txns.resize(0);
  #define subDifficulty(amt) difficulty -= amt; if (difficulty == 0) { revert(); return false; }

  bool CallTxn::valid(States &states, Hash &difficulty) {
    Code code = states[this->send].code;
    Block block = *(Block*)this->block;

    std::vector<char> memory(code.code.data(),code.code.data()+code.code.size());
    memory.insert(memory.end(),0,memsize-memory.size());
    typedef unsigned short loctype;
    loctype location = 0;
    loctype jumpedfrom = 0;
    bool stayInLoop = true;

    std::vector<Txn*> txns; //to avoid crosses initialization error
    Hash h = Hash();
    std::copy(block.hash.bytes,block.hash.bytes+sizeof(Hash),h.bytes);
    while (stayInLoop && location < memsize) { //TODO: break if too much difficulty used
      switch (memory[location]) {
      case 0: //STOP
        stayInLoop = false;
        break;
      case 1: //ADD
        mathOp(+,int);
      case 2: //SUB
        mathOp(-,int);
      case 3: //MULT
        mathOp(*,int);
      case 4: //DIV
        mathOp(*,int);
      case 5: //MOD
        mathOp(%,int);
      case 6: //NEG
        applyOp(unaryOp(-,int),sizeof(int));
      case 7: //MOV
        checkPtr(2,int);
        applyOp(setOp(ptrAt(2,int),int),4);
      case 8: //NOT
        applyOp(unaryOp(!,int),sizeof(int));
      case 9: //AND
        mathOp(&&,int);
      case 10: //OR
        mathOp(||,int);
      case 11: //NAND
        applyOp(binaryOp(&&,int);unaryOp(!,int);,sizeof(int)*2);
      case 12: //NOR
        applyOp(binaryOp(||,int);unaryOp(!,int);,sizeof(int)*2);
      case 13: //XOR
        applyOp(memAt(0,int) = !memAt(0,int) != !memAt(4,int);,sizeof(int)*2);
      case 14: //XNOR
        applyOp(memAt(0,int) = !memAt(0,int) == !memAt(4,int);,sizeof(int)*2);
      case 15: //BNOT
        applyOp(unaryOp(~,int),sizeof(int));
      case 16: //BAND
        mathOp(&,int);
      case 17: //BOR
        mathOp(|,int);
      case 18: //BNAND
        applyOp(binaryOp(&,int);unaryOp(!,int);,sizeof(int)*2);
      case 19: //BNOR
        applyOp(binaryOp(|,int);unaryOp(!,int);,sizeof(int)*2);
      case 20: //BXOR
        mathOp(^,int);
      case 21: //BXNOR
        applyOp(binaryOp(|,int);unaryOp(~,int);,sizeof(int)*2);
      case 22: //JMP
        applyOpNoAdd(jumpedfrom=location;location=memAt(0,loctype);,sizeof(loctype));
      case 23: //JG
        applyOpNoAdd(if(memAt(0,int)>0){jumpedfrom=location;location=memAt(0,loctype);}else location += sizeof(loctype)+sizeof(int)+1,sizeof(short)+sizeof(int));
      case 24: //JL
        applyOpNoAdd(if(memAt(0,int)<0){jumpedfrom=location;location=memAt(0,loctype);}else location += sizeof(loctype)+sizeof(int)+1,sizeof(short)+sizeof(int));
      case 25: //JE
        applyOpNoAdd(if(memAt(0,int)==0){jumpedfrom=location;location=memAt(0,loctype);}else location += sizeof(loctype)+sizeof(int)+1,sizeof(short)+sizeof(int));
      case 26: //JUMPEDFROM
        applyOp(setOp(jumpedfrom,loctype),sizeof(loctype));
      case 27: //REVERT
        applyOp(revert();,0);
      case 28: //ADDR
        copyBytesOp(this->recieve.bytes,sizeof(this->recieve.bytes));
      case 29: //CALLADDR
        copyBytesOp(this->send.bytes,sizeof(this->send.bytes));
      case 30: //CALLVALUE
        copyBytesOp(this->args,sizeof(this->args));
      case 31: //MONEY
        applyOp(setOp(states[this->recieve].money,Coins),sizeof(loctype));
      case 32: //CODE
        copyBytesOp(states[this->recieve].code.code.data(),states[this->recieve].code.code.size());
      case 33: //TIMESTAMP
        applyOp(setOp(block.time,Time),sizeof(loctype));
      case 34: //GASLEFT
        copyBytesOp(difficulty.bytes,sizeof(this->recieve.bytes));
      case 35: //GIVE
        checkArgs(sizeof(loctype)*2+sizeof(Coins));
        checkPtr(0,Key);
        checkPtr(sizeof(loctype)+sizeof(Coins),int);
        txns.push_back(new GiveTxn(this->recieve,Key(),memAt(sizeof(loctype),Coins)));
        std::copy(&ptrAt(0,char),&ptrAt(0,char)+sizeof(Key),((CallTxn*)(txns.back()))->recieve.bytes);
        if (txns.back()->valid(states,difficulty)) {
          ptrAt(sizeof(loctype)+sizeof(Coins),int) = 0;
          txns.pop_back();
        } else {
          ptrAt(sizeof(loctype)+sizeof(Coins),int) = 1;
          txns.back()->run(states,difficulty);
        }
        break;
      case 36: //MAKECTRCT
        checkArgs(sizeof(loctype)*3);
        checkPtr(0,char);
        checkPtr(sizeof(loctype),char);
        checkPtr(sizeof(loctype)*2,Hash);
        if (memAt(0,char)>=memAt(sizeof(loctype),char)) { stayInLoop = false; break; }
        txns.push_back(new MakeContractTxn(Key(),Code()));
        while (&states[Key(h)] != NULL) h++;
        std::copy(&h,&h+sizeof(Hash),((CallTxn*)(txns.back()))->recieve.bytes);
        std::copy(h.bytes,h.bytes+sizeof(Hash),((CallTxn*)(txns.back()))->send.bytes);
        if (txns.back()->valid(states,difficulty)) {
          for (size_t i = 0;i<sizeof(Hash);i++)
            *(&ptrAt(sizeof(loctype)+sizeof(Coins),char)+i) = 0;
          txns.pop_back();
        } else {
          std::copy(h.bytes,h.bytes+sizeof(Hash),&ptrAt(sizeof(loctype)*2,char));
          txns.back()->run(states,difficulty);
          h = h+Hash(1);
        }
        break;
      case 37: //CALLCTRCT
        checkArgs(sizeof(loctype)*2+sizeof(Coins));
        checkPtr(0,Key);
        checkPtr(sizeof(loctype)+sizeof(Coins),int);
        txns.push_back(new CallTxn(this->recieve,Key(),memAt(sizeof(loctype),Coins),&block));
        std::copy(&ptrAt(0,char),&ptrAt(0,char)+sizeof(Key),((CallTxn*)(txns.back()))->recieve.bytes);
        std::copy(&ptrAt(sizeof(Key)+sizeof(Coins),char),&ptrAt(sizeof(Key)+sizeof(Coins),char)+sizeof(GiveTxn),((CallTxn*)(txns.back()))->args);
        if (txns.back()->valid(states,difficulty)) {
          ptrAt(sizeof(loctype)+sizeof(Coins),int) = 0;
          txns.pop_back();
        } else {
          ptrAt(sizeof(loctype)+sizeof(Coins),int) = 1;
          txns.back()->run(states,difficulty);
        }
        break;
      case 38: //RSTORAGE
        checkArgs(sizeof(loctype)*2);
        checkPtr(0,int);
        applyOp(setOp(states[this->recieve].storage[ptrAt(sizeof(loctype),int)],int),sizeof(loctype)*2);
      case 39: //WSTORAGE
        checkArgs(sizeof(loctype)*2);
        checkPtr(0,int);
        checkPtr(sizeof(loctype),int);
        txns.push_back(new WriteStorageTxn(this->send,ptrAt(0,int),ptrAt(0,int)));
        txns.back()->run(states,difficulty);
        states[this->recieve].storage[ptrAt(0,int)] = ptrAt(sizeof(loctype),int);
        break;
      }
    }
    return true;
  }
}

#endif
