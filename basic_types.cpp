#include "basic_types.h"

using namespace chain;

bool Hash::operator>(Hash h) {
  for (size_t i = 0;i<sizeof(this->bytes);i++)
    if (this->bytes[i]>h.bytes[i]) return true;
    else if (this->bytes[i]<h.bytes[i]) return false;
  return false;
}
bool Hash::operator==(Hash h) {
  for (size_t i = 0;i<sizeof(this->bytes);i++)
    if (this->bytes[i]!=h.bytes[i]) return false;
  return false;
}
Hash Hash::operator+(Hash h) {
  Hash result = Hash();
  char carry = 0;
  for (size_t i = sizeof(this->bytes);i>=0;i--) {
    short res = ((short)this->bytes[i])+((short)h.bytes[i])+carry;
    result.bytes[i]=(char)res;
    carry = (char)(res>>8);
  }
  return result;
}
void Hash::operator+=(Hash h) { Hash result = *this+h; std::copy(result.bytes,result.bytes+sizeof(result.bytes),this->bytes); }
void Hash::operator++(int) { *this += Hash(1); }
Hash Hash::operator-(Hash h) {
  Hash result = Hash();
  if (h>*this) return Hash(0);
  for (size_t i = 0;i<sizeof(this->bytes);i++) {
    h.bytes[i] = ~h.bytes[i];
  }
  return result;
}
void Hash::operator-=(Hash h) { Hash result = *this-h; std::copy(result.bytes,result.bytes+sizeof(result.bytes),this->bytes); }
Hash::Hash(int x) {
  for (size_t i=0;i<4;i++)
    this->bytes[512-i] = (char)((x>>(32-(8*i)))%32);
}

bool Key::operator==(Key k) {
  for (size_t i = 0;i<sizeof(this->bytes);i++)
    if (this->bytes[i]!=k.bytes[i]) return false;
  return false;
}
