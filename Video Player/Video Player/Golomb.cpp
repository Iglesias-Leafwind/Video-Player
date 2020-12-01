#include <iostream>
#include <stdio.h>

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "BitStream.h"
#include "Golomb.h"
using namespace std;

Golomb::Golomb() {
}

Golomb::Golomb(string path, int m, string readorwrite) {
	this->path = path;
	this->m = m;
	this->b = ceil(log2(m));
	this->ReadWrite.open(path,readorwrite);
}
void Golomb::changeM(int m) {
	this->m = m;
	this->b = ceil(log2(m));
}
void Golomb::encode(signed int encoding) {
	if (encoding < 0) {
		encoding = (encoding * (-2)) - 1;
	}
	else {
		encoding = (encoding * 2);
	}
	int q = encoding / m;
	int r = encoding - (q * m);
	
	/*here we will add the quotient part of the algorithm*/
	for (int ones = 0; ones < q; ones++) {
		ReadWrite.writebit(1);
	}
	ReadWrite.writebit(0);
	//resto < 2^b -m (b-1 bits) e r = r
	if (r < ((int)pow(2, b) - m)) {
		ReadWrite.writeNbits(r, b-1);
	}
	//resto >= 2^b -m (b bits) e r = r + 2^b-m
	else if (r >= ((int)pow(2, b) - m)) {
		r = (r + (int)pow(2, b) - m);
		ReadWrite.writeNbits(r, b);
	}
	else {
		cout << "You did it!!! BUT HOW?" << endl;
	}
};

signed int Golomb::decode() {
	//if we read bit with position b and it is 1
	// then we are in r and we need to read from 0 to b-2 for r.
	int q = -1;
	signed int retrn = 0;
	//Q analysis
	unsigned char current_bit = -1;
	while(current_bit != 0x00) {
		q++;
		current_bit = ReadWrite.readbit();
	}
	//R analysis
	int r = (int) ReadWrite.readNbits(b - 1);

	if (r >= ((int)pow(2, b) - m)) {
		// then r is formula (R + (int)pow(2, b) - m)
		r = r << 1;
		r = r + ReadWrite.readbit();
		r = r - (int)pow(2, b) + m;
	}

	retrn = q * m + r;
	if ((retrn % 2) == 0) {
		retrn = retrn / 2;
	}
	else {
		retrn = (retrn + 1) / (-2);
	}
	return retrn;
};
void Golomb::turnaround() {
	if (ReadWrite.readwrite) {
		ReadWrite.open(path, "w");
	}
	else {
		ReadWrite.open(path, "r");
	}
};
int testes() {
	cout << "Test with m = 5 from -255 to 255: " << endl;
	int m = 5;
	Golomb a("golomb.bin",m,"w");
	for (signed int i = -255; i < 255; i++) {
		a.encode(i);
		a.turnaround();
		cout << " - " << i << " :decode: " << a.decode() << endl;
		a.turnaround();
	}
	return 0;
}
