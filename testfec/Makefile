all: qr golay hamming7 hamming12

hamming7: fec.o hamming7.cpp
	g++ -o hamming7 fec.o hamming7.cpp

hamming12: fec.o hamming12.cpp
	g++ -o hamming12 fec.o hamming12.cpp

golay: fec.o golay.cpp
	g++ -o golay fec.o golay.cpp

qr: fec.o qr.cpp
	g++ -o qr fec.o qr.cpp

fec.o: ../fec.h ../fec.cpp
	g++ -c -o fec.o -I.. ../fec.cpp