#CXXFLAGS=-g
CXXFLAGS=-O3

all: qr golay hamming7 hamming12 hamming16 viterbi viterbi3

viterbi: viterbi.o descramble.o viterbi.cpp
	g++ -o viterbi viterbi.o descramble.o viterbi.cpp

viterbi3: viterbi3.o viterbi.o descramble.o viterbi3.cpp
	g++ -o viterbi3 viterbi3.o viterbi.o descramble.o viterbi3.cpp

hamming7: fec.o hamming7.cpp
	g++ -o hamming7 fec.o hamming7.cpp

hamming12: fec.o hamming12.cpp
	g++ -o hamming12 fec.o hamming12.cpp

hamming16: fec.o hamming16.cpp
	g++ -o hamming16 fec.o hamming16.cpp

golay: fec.o golay.cpp
	g++ -o golay fec.o golay.cpp

qr: fec.o qr.cpp
	g++ -o qr fec.o qr.cpp

fec.o: ../fec.h ../fec.cpp
	g++ $(CXXLFAGS) -c -o fec.o -I.. ../fec.cpp

viterbi.o: ../viterbi.h ../viterbi.cpp
	g++ $(CXXFLAGS) -c -o viterbi.o -I.. ../viterbi.cpp

viterbi3.o: ../viterbi3.h ../viterbi3.cpp
	g++ $(CXXFLAGS) -c -o viterbi3.o -I.. ../viterbi3.cpp

descramble.o: ../descramble.h ../descramble.cpp
	g++ $(CXXFLAGS) -c -o descramble.o -I.. ../descramble.cpp

clean:
	rm -f *.o qr golay hamming7 hamming12 hamming16 viterbi viterbi3
	