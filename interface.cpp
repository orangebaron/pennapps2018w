#include "chain.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/hex.h"
#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/oids.h"
#include <iostream>
#include <string>
#include <ctime>


namespace chain {
  Key keyFromPriv(std::string key) {
    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
    CryptoPP::HexDecoder decoder;
    decoder.Put((CryptoPP::byte*)key.data(), key.size());
    decoder.MessageEnd();

    CryptoPP::Integer x;
    x.Decode(decoder, decoder.MaxRetrievable());
        CryptoPP::AutoSeededRandomPool prng;

    privateKey.Initialize(CryptoPP::ASN1::secp256k1(), x);

    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;
    privateKey.MakePublicKey(publicKey);

    Key k = Key();
    //TODO get it into the key
    return k;
  }
  void signBlock(Block &b,std::string key) {
    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
    CryptoPP::HexDecoder decoder;
    decoder.Put((CryptoPP::byte*)key.data(), key.size());
    decoder.MessageEnd();

    CryptoPP::Integer x;
    x.Decode(decoder, decoder.MaxRetrievable());
        CryptoPP::AutoSeededRandomPool prng;

    privateKey.Initialize(CryptoPP::ASN1::secp256k1(), x);


    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Signer signer( privateKey );


    std::string signature;
    b.sigs.push_back(*new Sig());
    Key k = keyFromPriv(key);
    b.sigs.end()->signer = &k; //TODO: can I even do this
    CryptoPP::ArraySource s( b.beforeHash.bytes, true /*pump all*/,
        new CryptoPP::SignerFilter( prng,
            signer,
            new CryptoPP::ArraySink( (CryptoPP::byte*)b.sigs.end()->bytes , 72)
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
    for (;Hash(200)>b.getDifficulty();b.nonce++) {}
    signBlock(b,givePriv);
    return ch.addBlock(&b);
  }
  bool call(std::string givePriv,Key recievePub,Coins amt,char args[sizeof(GiveTxn)],Blockchain ch) {
    Block b;
    CallTxn *t = new CallTxn(keyFromPriv(givePriv),recievePub,amt,&b);
    std::copy(args,args+sizeof(GiveTxn),t->args);
    b.txns.push_back(*t);
    b.approved[0] = *ch.blocks.end()-1;
    b.approved[1] = *ch.blocks.end()-2;
    b.time = time(NULL);
    b.calcBeforeHash();
    for (;Hash(200)>b.getDifficulty();b.nonce++) {}
    signBlock(b,givePriv);
    return ch.addBlock(&b);
  }
  bool make(std::string givePriv,Code code,Blockchain ch) {
    Block b;
    MakeContractTxn *t = new MakeContractTxn(keyFromPriv(givePriv),code);
    b.txns.push_back(*t);
    b.approved[0] = *ch.blocks.end()-1;
    b.approved[1] = *ch.blocks.end()-2;
    b.time = time(NULL);
    b.calcBeforeHash();
    for (;Hash(200)>b.getDifficulty();b.nonce++) {}
    signBlock(b,givePriv);
    return ch.addBlock(&b);
  }
}
