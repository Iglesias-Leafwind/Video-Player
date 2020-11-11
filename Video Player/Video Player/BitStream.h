#pragma once

#include <iostream>
#include <stdio.h>
#include <fstream>


#include <bitset>

using namespace std;


/*! class BitStream
* this class is used to write or read bits from a file
*/
class BitStream {
public:
    /*! int current_bit
    *  this is used to track which bit we are editing on the buffer
    */
    int current_bit = 0;
    /*! bitset<8> currentBitStream
    * this is the bit buffer
    */
    bitset<8> currentBitStream;
    /*! ifstream input
    * if we are reading we are using input stream to read a file
    */
    ifstream input;
    /*! ofstream output
    * if we are writing we are using output stream to write on a file
    */
    ofstream output;
    /*! bool readwrite
    *   1 = read
    *   0 = write
    */
    bool readwrite = 0;
    /*! BitStream()
    * use function open to : chose through ReadorWrite if we want to read or write
    * "r" - if we want to read we open the file through file_path and initialize ifstream input
    * "w" - if we want to write we create a file through file_path and initialize ofstream output
    */
    BitStream();
    /*! unsinged char readbit()
    * reads 1 bit from the file and returns it as a uchar
    */
    unsigned char readbit();
    /*! void writebit(unsigned char bit)
    * writes 1 bit to the file from a uchar
    * this uchar if its bits are all 0 then it will write bit 0 if it is anything else it will write 1
    */
    void writebit(unsigned char bit);
    /*! unsigned char  readNbits(int Nbits)
    *  reads Nbits from the file using readbit() function and returns a unsigned char with the result
    */
    unsigned char readNbits(int Nbits);
    /*! void writeNbits(int N, int Nbits)
    * writes N with Nbits on the file
    */
    void writeNbits(int N, int Nbits);
    /*! void write()
    * writes everyrthing that is saved in the buffer
    *  This isn't needed it is only a formality.
    */
    void write();
    /*! void close()
    * closes input and output streams when the program finishes the usage
    * this is needed always after finishing using.
    */
    void close();
    /*! void open(String file_path, string ReadorWrite)
    * this is the bitstream opener where it choses through ReadorWrite if we want to read or write
    * "r" - if we want to read we open the file through file_path and initialize ifstream input
    * "w" - if we want to write we create a file through file_path and initialize ofstream output
    */
    void open(string file_path, string ReadorWrite);
};