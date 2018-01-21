#include "chain.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/hex.h"
#include "cryptopp/osrng.h"
#include <iostream>
#include <string>
#include <ctime>

namespace chain {
  Key keyFromPriv(std::string priv) {
  }
  void signBlock(Block &b,std::string givePriv) {
    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
    CryptoPP::HexDecoder decoder;
    decoder.Put((CryptoPP::byte*)givePriv.data(), givePriv.size());
    decoder.MessageEnd();
    CryptoPP::ECDSA<CryptoPP::ECP,CryptoPP::SHA1>::Signer signer( privateKey );

    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::StringSource s( message, true /*pump all*/,
        new SignerFilter( prng,
            signer,
            new StringSink( signature )
        ) // SignerFilter
    );
  }
  bool give(std::string givePriv,Key recievePub,Coins amt,Blockchain ch) {
    GiveTxn *t = new GiveTxn(keyFromPriv(givePriv),recievePub,amt);
    Block b;
    b.txns.push_back(*t);
    b.approved[0] = *ch.blocks.end()-1;
    b.approved[1] = *ch.blocks.end()-2;
    b.time = time(NULL);
    b.calcBeforeHash();
    for (;Hash(20)>b.getDifficulty();b.nonce++) {}
    signBlock(b,givePriv);
    return ch.addBlock(&b);
  }
}
