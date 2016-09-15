#CXXFLAGS=-g
CXXFLAGS=-O3

all: qr golay hamming7 hamming12 hamming16 viterbi

viterbi: viterbi.o viterbi.cpp
	g++ -o viterbi viterbi.o viterbi.cpp

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

clean:
	rm -f *.o qr golay hamming7 hamming12 hamming16 viterbi
	