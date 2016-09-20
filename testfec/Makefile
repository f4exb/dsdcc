#CXXFLAGS=-g
CXXFLAGS=-O3

all: qr golay20 golay23 golay24 hamming7 hamming12 hamming16 viterbi viterbi35 crc pn

crc: crc.o crc.cpp
	g++ -o crc crc.o crc.cpp

pn: pn.o pn.cpp
	g++ -o pn pn.o pn.cpp

viterbi: viterbi.o descramble.o viterbi.cpp
	g++ -o viterbi viterbi.o descramble.o viterbi.cpp

viterbi35: viterbi3.o viterbi5.o viterbi.o descramble.o viterbi35.cpp
	g++ -o viterbi35 viterbi3.o viterbi5.o viterbi.o descramble.o viterbi35.cpp

hamming7: fec.o hamming7.cpp
	g++ -o hamming7 fec.o hamming7.cpp

hamming12: fec.o hamming12.cpp
	g++ -o hamming12 fec.o hamming12.cpp

hamming16: fec.o hamming16.cpp
	g++ -o hamming16 fec.o hamming16.cpp

golay20: fec.o golay20.cpp
	g++ -o golay20 fec.o golay20.cpp

golay23: fec.o golay23.cpp
	g++ -o golay23 fec.o golay23.cpp

golay24: fec.o golay24.cpp
	g++ -o golay24 fec.o golay24.cpp

qr: fec.o qr.cpp
	g++ -o qr fec.o qr.cpp

fec.o: ../fec.h ../fec.cpp
	g++ $(CXXLFAGS) -c -o fec.o -I.. ../fec.cpp

crc.o: ../crc.h ../crc.cpp
	g++ $(CXXFLAGS) -c -o crc.o -I.. ../crc.cpp

pn.o: ../pn.h ../pn.cpp
	g++ $(CXXFLAGS) -c -o pn.o -I.. ../pn.cpp

viterbi.o: ../viterbi.h ../viterbi.cpp
	g++ $(CXXFLAGS) -c -o viterbi.o -I.. ../viterbi.cpp

viterbi3.o: ../viterbi3.h ../viterbi3.cpp
	g++ $(CXXFLAGS) -c -o viterbi3.o -I.. ../viterbi3.cpp

viterbi5.o: ../viterbi5.h ../viterbi5.cpp
	g++ $(CXXFLAGS) -c -o viterbi5.o -I.. ../viterbi5.cpp

descramble.o: ../descramble.h ../descramble.cpp
	g++ $(CXXFLAGS) -c -o descramble.o -I.. ../descramble.cpp

clean:
	rm -f *.o qr golay20 golay23 golay24 hamming7 hamming12 hamming16 viterbi viterbi35 crc
	