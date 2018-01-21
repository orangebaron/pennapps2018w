cryptopp:
	git clone https://github.com/weidai11/cryptopp.git
bigint.cpp:
	git clone https://github.com/indy256/codelibrary.git
	mv codelibrary/cpp/bigint-full.cpp bigint.cpp
	rm codelibrary
all: cryptopp bigint.cpp
