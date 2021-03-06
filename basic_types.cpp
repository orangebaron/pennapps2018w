#include "basic_types.h"
#include "cryptopp/sha.h"
#include "cryptopp/eccrypto.h"
#include <string>

using namespace chain;

bool Hash::operator>(Hash h) {
  for (size_t i = 0;i<sizeof(bytes);i++)
    if (bytes[i]>h.bytes[i]) return true;
    else if (bytes[i]<h.bytes[i]) return false;
  return false;
}
bool Hash::operator==(Hash h) {
  for (size_t i = 0;i<sizeof(bytes);i++)
    if (bytes[i]!=h.bytes[i]) return false;
  return false;
}
Hash Hash::operator+(Hash h) {
  Hash result = Hash();
  char carry = 0;
  for (size_t i = sizeof(bytes);i>=0;i--) {
    short res = ((short)bytes[i])+((short)h.bytes[i])+carry;
    result.bytes[i]=(char)res;
    carry = (char)(res>>8);
  }
  return result;
}
void Hash::operator+=(Hash h) { Hash result = *this+h; std::copy(result.bytes,result.bytes+sizeof(result.bytes),bytes); }
void Hash::operator++(int) { *this += Hash(1); }
Hash Hash::operator-(Hash h) {
  Hash result = Hash();
  if (h>*this) return Hash(0);
  for (size_t i = 0;i<sizeof(bytes);i++) {
    h.bytes[i] = ~h.bytes[i];
  }
  return result;
}
void Hash::operator-=(Hash h) { Hash result = *this-h; std::copy(result.bytes,result.bytes+sizeof(result.bytes),bytes); }
Hash::Hash(int x) {
  for (size_t i=0;i<4;i++)
    bytes[512-i] = (char)((x>>(32-(8*i)))%32);
}
Hash::Hash(char data[],size_t size) {
  CryptoPP::SHA256 c;
  c.Update((CryptoPP::byte*)data,size);
  c.Final((CryptoPP::byte*)bytes);
}

bool Key::operator==(Key k) {
  for (size_t i = 0;i<sizeof(bytes);i++)
    if (bytes[i]!=k.bytes[i]) return false;
  return false;
}

bool Sig::valid(char *data,size_t len) {
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;
  publicKey.Load(
    *new CryptoPP::ArraySource((CryptoPP::byte*)signer->bytes, sizeof(signer->bytes), true /*pumpAll*/)
  );
  CryptoPP::ECDSA<CryptoPP::ECP,CryptoPP::SHA256>::Verifier verifier(publicKey);
  bool result = verifier.VerifyMessage(
    (const CryptoPP::byte*)data, len,
    (const CryptoPP::byte*)bytes, sizeof(bytes)
  );
  return result;
}
