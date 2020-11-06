#include <iostream>
#include <stdio.h>

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>

using namespace std;

/*! class Golomb
* this class is used to encode and decode using Golomb method
*/
class Golomb {
public:
	/*! int b
	*	b is the value used for the numbewr of bites that we are going to user
	*	on the remainer
	*/
	int b;
	/*! int m
	*	m is the value used for the golomb encoding / decoding
	*/
	int m;
	/*! Golomb(int m);
	*	this constructor is used to specify which value "m" you will use for Golomb encoding / decoding
	*/
	Golomb(int m);
	/*! signed int encode(signed int encoding);
	*	this function encodes a signed int (encoding) and returns a signed int (encoded)
	*/
	signed int encode(signed int encoding);
	/*! signed int decode(signed int decoding);
	*	this function decodes a signed int (decoding) and returns a signed int (decoded)
	*/
	signed int decode(signed int decoding);
};

Golomb::Golomb(int m) {
	this->m = m;
	this->b = ceil(log2(m));
}

signed int Golomb::encode(signed int encoding) {
	int q = floor(encoding / m);
	int r = encoding - q * m;
	/*this is the value we are going to return*/
	signed int retrn = 0;
	/*here we will add the quotient part of the algorithm*/
	for (int ones = 0; ones < q; ones++) {
		retrn = retrn << 1;
		retrn |= 0x01;
	}
	retrn = retrn << 1;
	retrn |= 0x00;

	if (r < ((int)pow(2, b) - m)) {
		for (int bit = (b - 2); bit >= 0 ;bit--) {
			retrn = retrn << 1;
			retrn |= ((r >> bit) & 0x01);
		}
	}
	else if (r >= ((int)pow(2, b) - m)) {
		for (int bit = (b-1); bit >= 0 ;bit--) {
			retrn = retrn << 1;
			retrn |= (((r + (int)pow(2, b) - m) >> bit) & 0x01);
		}
	}
	else {
		cout << "You did it!!! BUT HOW?" << endl;
	}
	return retrn;
};

signed int Golomb::decode(signed int decoding) {
	//if we read bit with position b and it is 1
	// then we are in r and we need to read from 0 to b-2 for r.
	signed int q = 0;
	signed int r = 0;
	signed int retrn;
	if (((decoding >> b) & 0x01) or decoding < b){
		//R analysis
		for (int bit = b - 2; bit >= 0; bit--) {
			r = r << 1;
			r |= ((decoding >> bit) & 0x01);
		}
		//Q analysis
		if (decoding >= b) {
			q++;
		}
		while (((decoding >> b + q) & 0x01)) {
			q++;
		}
		q = m * q;
	}
	
	//if we read bit with position b and it is 0
	// then we are in (r+2^b-m)	and we need to read from 0 to b-1.
	else {
		//R analysis
		for (int bit = b - 1; bit >= 0; bit--) {
			r = r << 1;
			r |= ((decoding >> bit) & 0x01);
		}
		r = (r - (int)pow(2, b) + m);
		//Q analysis
		while (((decoding >> b+1 + q) & 0x01)) {
			q++;
		}
		q = m * q;
	}
	retrn = q + r;
	return (signed int) retrn;
};

int main() {
	cout << "Test with m = 5 from 0 to 100: " << endl;
	int m = 5;
	Golomb a(m);
	for (signed int i = 0; i < 100; i++) {
		cout << i << " :encode: " << a.encode(i) << " :decode: " << a.decode(a.encode(i)) << endl;
	}
	cout << "Test with m = 15 from 0 to 255: " << endl;
	m = 15;
	Golomb b(m);
	for (signed int i = 0; i < 255; i++) {
		cout << i << " :encode: " << b.encode(i) << " :decode: " << b.decode(b.encode(i)) << endl;
	}
}
