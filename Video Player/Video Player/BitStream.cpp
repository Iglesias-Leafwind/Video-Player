
#include <iostream>
#include <stdio.h>
#include <fstream>

#include "BitStream.h"
#include <bitset>

using namespace std;

BitStream::BitStream() {
}
void BitStream::writebit(unsigned char bit) {
    if (output.fail()) {
        throw runtime_error("Could not open file");
    }
    if (bit != 0) {
        currentBitStream = currentBitStream << 1;
        currentBitStream |= 0x01;
    }
    else {
        currentBitStream = currentBitStream << 1;
        currentBitStream |= 0x00;
    }
    current_bit += 1;
    if (current_bit == 8) {
        unsigned char writing = (unsigned char) currentBitStream.to_ulong();
        output.put((char)writing);
        current_bit = 0;
        currentBitStream = 0;
    }
    
}
unsigned char BitStream::readNbits(int Nbits) {
    if (input.fail()) {
        throw runtime_error("Could not open file");
    }
    unsigned long result = (unsigned long)readbit();
    int i = 1;
    while (i != Nbits) {
        result = result << 1;
        unsigned long bit = (unsigned long) readbit();
        result = result + bit;
        i++;
    }
    return (unsigned char) result;
}
void BitStream::writeNbits(int N, int Nbits) {
    if (output.fail()) {
        throw runtime_error("Could not open file");
    }
    unsigned char number = (unsigned char) N;
    int i = Nbits-1;
    while (i >= 0) {
        writebit((number >> i) & 0x01);
        i--;
    }
}
unsigned char BitStream::readbit() {
    if (input.fail()) {
        throw runtime_error("Could not open file");
    }

    if (current_bit == 0){
        char reading;
        if(input.get(reading)){
            for (int i = 7; 0 <= i; i--){
                currentBitStream[i] = ((unsigned char)reading >> i) & 0x01;
            }
        }
        else {
            throw runtime_error("Empty File");
        }
    }
    unsigned char returning = (unsigned char) currentBitStream[7-current_bit++];
    if(current_bit == 8){
        current_bit = 0;
        currentBitStream = 0;
    }
    cout << (int) returning;
    return returning;
}
void BitStream::close() {
    if (current_bit != 0 and not output.fail()) {
        write();
    }
    input.close();
    output.close();
}
void BitStream::open(string file_path,string ReadorWrite) {
    if (input.is_open() or output.is_open()) {
        close();
    }
    if (ReadorWrite == "w") {
        output.open(file_path, ios::binary);
        if (output.fail()) {
            throw runtime_error("Could not open file");
        }
        readwrite = 0;
    }
    else if (ReadorWrite == "r") {
        input.open(file_path, ios::binary);
        if (input.fail()) {
            throw runtime_error("Could not open file");
        }
        readwrite = 1;
    }
}
void BitStream::write() {
    if (output.fail()) {
        throw runtime_error("Could not open file");
    }
    while(current_bit != 8){
        currentBitStream = currentBitStream << 1;
        currentBitStream |= 0x00;
        current_bit += 1;
    }
    unsigned char writing = (unsigned char)currentBitStream.to_ulong();
    output.put((char)writing);
    current_bit = 0;
    currentBitStream = 0;
}


/*Change the name of this file to "main" to test
*   after changing the name of video player file "main"
*/
int tests()
{
    BitStream bs;
    bs.open("binary.bin", "w");
    /*result1*/
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(1);
    /*result2*/
    bs.writeNbits(255,16);
    /*result3*/
    bs.writeNbits(254,10);
    bs.write();
    /*result4*/
    bs.writebit(1);
    bs.writebit(1);
    bs.close();

    BitStream bs2;
    bs2.open("binary.bin", "r");
    /*results*/
    cout << "result1:";
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit()<< " ";
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit()<< endl;
    cout << "0000 0000 1111 1111 = " << (int)bs2.readNbits(16) << endl;
    cout << "0011 1111 10 = " << (int)bs2.readNbits(10) << endl;
    cout << "0000 00 = " << (int)bs2.readNbits(6) << endl;
    cout << "1100 0000 = " << (int)bs2.readNbits(8) << endl;
    return 0;
}