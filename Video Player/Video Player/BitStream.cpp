
#include <iostream>
#include <stdio.h>
#include <fstream>

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>

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

    /*! BitStream(String file_path, string ReadorWrite)
    * this is the bitstream constructor where it choses through ReadorWrite if we want to read or write
    * if we want to read we open the file through file_path and initialize ifstream input
    * if we want to write we create a file through file_path and initialize ofstream output
    */
    BitStream(string file_path, string ReadorWrite);
    /*! unsinged char readbit()
    * reads 1 bit from the file and returns it as a uchar
    */
    unsigned char readbit();
    /*! void writebit(unsigned char bit)
    * writes 1 bit to the file from a uchar
    * this uchar if its bits are all 0 then it will write bit 0 if it is anything else it will write 1
    */
    void writebit(unsigned char bit);
    /*! vector<int> readNbits(int Nbits)
    *  reads Nbits from the file using readbit() function and returns a vector with all bits
    */
    vector<int> readNbits(int Nbits);
    /*! void writeNbits(int N, int Nbits)
    * writes N with Nbits on the file
    */
    void writeNbits(int N, int Nbits);
    /*! void write()
    * writes everyrthing that is saved in the buffer
    */
    void write();
    /*! void close()
    * closes input and output streams when the program finishes the usage
    */
    void close();
};
BitStream::BitStream(string file_path, string ReadorWrite) {

    if (ReadorWrite == "w") {
        output.open(file_path, ios::binary);
        if (output.fail()) {
            throw runtime_error("Could not open file");
        }
    }
    else if(ReadorWrite == "r"){
        input.open(file_path, ios::binary);
        if (input.fail()) {
            throw runtime_error("Could not open file");
        }
    }
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
vector<int> BitStream::readNbits(int Nbits) {
    if (input.fail()) {
        throw runtime_error("Could not open file");
    }
    vector<int> bits;
    int i = 0;
    while (i != Nbits) {
        int bit = (int) readbit();
        bits.push_back(bit);
        i++;
    }
    return bits;
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
    return returning;
}
void BitStream::close() {
    if (current_bit != 0 and not output.fail()) {
        write();
    }
    input.close();
    output.close();
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
/*Change the name of this class to main to test
*   after changing the name of video player main
*/
int test()
{
    BitStream bs("binary.bin","w");
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(0);
    bs.writebit(1);

    bs.writebit(0);
    bs.writebit(1);
    bs.write();
    bs.writeNbits(4,8);
    bs.writebit(1);
    bs.writebit(1);
    bs.close();
    BitStream bs2("binary.bin", "r");
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit()<< " ";
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit();
    cout << (int)bs2.readbit()<< endl;
    vector<int> lmao = bs2.readNbits(8);
    cout << "myvector contains:";
    for (int i = 0; i < lmao.size(); i++){
        cout << lmao.at(i);
        if (i == 3)
            cout << " ";
    }
    cout << '\n';

    lmao = bs2.readNbits(8);
    cout << "my2vector contains:";
    for (int i = 0; i < lmao.size(); i++) {
        cout << lmao.at(i);
        if (i == 3)
            cout << " ";
    }
    cout << '\n';

    lmao = bs2.readNbits(8);
    cout << "my3vector contains:";
    for (int i = 0; i < lmao.size(); i++) {
        cout << lmao.at(i);
        if (i == 3)
            cout << " ";
    }
    cout << '\n';
}