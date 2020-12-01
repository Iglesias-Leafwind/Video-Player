#pragma once
#include <iostream>
#include <stdio.h>

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "BitStream.h"

using namespace std;

/*! class Golomb
* this class is used to encode and decode using Golomb method
*/
class Golomb {
public:
	/*! Golomb();
	*	default empty constructor
	*/
	Golomb();
	/*! int b
	*	b is the value used for the numbewr of bites that we are going to user
	*	on the remainer
	*/
	int b;
	/*! string path
	*	string path file to be used in Bitstream class
	*/
	string path;
	/*!	BitStream ReadWrite
	*	used for reading or writing bits from/to path
	*/
	BitStream ReadWrite;
	/*! int m
	*	m is the value used for the golomb encoding / decoding
	*/
	int m;
	/*! Golomb(string path,int m, string readorwrite);
	*	this constructor is used to specify which value "m" you will use for Golomb encoding / decoding
	*/
	Golomb(string path, int m, string readorwrite);
	/*! void encode(signed int encoding);
	*	this function encodes a signed int (encoding) and writes on file path
	*/
	void encode(signed int encoding);
	/*! signed int decode();
	*	this function reads from file path and returns a signed int (decoded)
	*/
	signed int decode();
	/*! void turnaround()
	*	if reading turns into a write
	*	if writing turns into a read
	*/
	void turnaround();
	/*! void changeM(int m)
	* it is used to change m value when needed
	*/
	void changeM(int m);
};