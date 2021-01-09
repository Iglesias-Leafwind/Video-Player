
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "Golomb.h"
#include <stdlib.h>     /* abs */

using namespace cv;
using namespace std;



/*! class Predictor
* this class is used to convert the data from a frame / image to a more repeatable pattern
*/
class LossyPredictor {
private:

    /*!void changeGolombM(int total)
     * this will use a total number of bits and calculate the best m for golomb to use
     * */
    void changeGolombM(int total);
    /*! bool rwFlag
    *   Tells us if it is reading or writing
    *  starting with 1 == writing
    */
    bool rwFlag = 1;
    /*! Mat fillZeros(const Mat& channel)
    * given a Mat with a channel it will fill the 1st row and 1st column with zeros
    * for easier future calculation
    */
    Mat fillZeros(const Mat& channel);
    /*! pair<signed int, signed int> calculator(const Mat& numb)
    * this function simply gets the numbers from the mat and depending on the mode we are using it will calculate the correct precdictor
    * and will shiftnbits >> depending on nbits determined previously
     * after that it will predict what the decoder will do and return both values
     * */
    pair<signed int, signed int> calculator(const Mat& numb);
    /*! signed int decalculator(vector<signed int> numb)
    * this function simply gets the vector numbers and depending on the mode we are using it will calculate the original number
    * and will shiftnbits << dependin on nbits determined previously
     * */
    signed int decalculator(vector<signed int> numb);
    /*! Mat frameRead3Channel()
    * will read a 3channels frame from bin file
    */
    Mat frameRead3Channel();
    /*! Mat frameRead1Channel()
    * will read a 1channel frame from bin file
    */
    Mat frameRead1Channel();
    /*! Mat frameWrite3Channel(const Mat& frame)
    * will write a 3channels frame into the bin file
    */
    Mat frameWrite3Channel(Mat frame);
    /*! Mat frameWrite1Channel(const Mat& frame)
    * will write a 1channel frame into the bin file
    */
    Mat frameWrite1Channel(Mat frame);

    /*! Mat blockFrameRead3Channel()
    * will read a 3channels frame from bin file using blocks
    */
    Mat blockFrameRead3Channel();
    /*! Mat blockFrameRead1Channel()
    * will read a 1channel frame from bin file using blocks
    */
    Mat blockFrameRead1Channel();
    /*! Mat blockFrameWrite3Channel(const Mat& frame)
    * will write a 3channels frame into the bin file using blocks
    */
    Mat blockFrameWrite3Channel(Mat frame);
    /*! Mat blockFrameWrite1Channel(const Mat& frame)
    * will write a 1channel frame into the bin file using blocks
    */
    Mat blockFrameWrite1Channel(Mat frame);
    /*! Mat fakePredict3Channel(const Mat& frame)
     * will check how many bits it is going to use for inter and intra frames
     * */
    void fakePredict3Channel(const Mat& frame);
    /*! Mat fakePredict1Channel(const Mat& frame)
     * will check how many bits it is going to use for inter and intra frames
     * */
    void fakePredict1Channel(const Mat& frame);
    /*! signed int bitsForIntra
     * this var is going to tell us how many bits we need for intra frames
     * */
    signed int bitsForIntra;
    /*! signed int bitsForInter
     * this var is going to tell us how many bits we need for inter frames
     * */
    signed int bitsForInter;
public:

    /*! int bpp
     * this is used to know how many bits per pixel we are using encoding a video
     */
     int bpp;
     /*! int totalPixeis
      * total number of pixeis that will be used to calculate bpp
      */
     int totalPixeis;
    /*! int nbits
     * this is used to save the number of bits that will be used in lossy encoding and decoding
     * */
    int nbits;
    /*! int videoHeight
    * this is used for readInfo when extracting video info
    * for saving or reading a frame height
    */
    int videoHeight;
    /*! int videoWidth
    * this is used for readInfo when extracting video info
    * for saving or reading a frame width
    */
    int videoWidth;
    /*! int videoFPS
    * this is used for readInfo when extracting video info
    * for saving or reading a video fps
    */
    int videoFPS;
    /*! int videoFrames
    * this is used for readInfo when extracting video info
    * for saving or reading quantity of frames
    */
    int videoFrames;
    /*! string type
    * this is used for readInfo when extracting video info
    * for saving or reading a frame type
    */
    string type;
    /*! Mat prevFrame
     * this is to save the previous frame for intra frame coding and decoding
     */
    Mat prevFrame;
    /*! int blockSize
    * this is the block size
    */
    int blockSize;
    /*! int blockSearchRadius
     * this is to determine the radius when searching previous frame
     * this block size is usually recommended suing 2^n block size
     * */
    int blockSearchRadius;
    /*! Golomb golombo
    * this golomb will be used to write and read into and from the bit stream
    */
    Golomb golombo;
    /*! int mode
    *  this is used to track which mode we are using
    *  we currently have 8 modes
    * 1- a
    * 2- b
    * 3- c
    * 4- a+b-c
    * 5- a+(b-c)/2
    * 6- b+(a-c)/2
    * 7- (a+b)/2
    * 8- non linear mode -
    *       if c >= Max(a,b) then min(a,b)
    *       if c <= min(a,b) then Max(a,b)
    *           else         then a+b-c
    */
    int mode;
    /*! bool prevFrameF = false
     *  prevFrameF is a flag that will tell predictor if there is a prevFrame or not when decoding or encoding
     */
     bool prevFrameF = false;
    /*! LossyPredictor(int mode,string path)
    * this is the contructor that will accept the mode we want to use to "encode" / "convert" / "decode"
    * and uses path where it will save the encoded file or read the encoded file
    */
    LossyPredictor(int mode);
    /*! void saveInfo(int h, int w, string type, int fps,int frames,int nbits,string path)
    * will save info into the bin file
    * and save it on local variables in this class
    */
    void saveInfo(int h, int w, string type, int fps,int frames,int nbits,string path);
    /*! void readInfo(string path)
    * will read info from the bin file
    * and save it on local variables in this class
    */
    void readInfo(string path);
    /*! void encode(const Mat& frame)
    * here we have the encode function that with a frame it will choose if it has 3 channels or 1 and convert each channel using the selected mode and write in the bin file
    */
    void encode(const Mat& frame);
    /*! Mat decode(void)
    * here we have the decode function that with the bin file it will read a frame and convert into a single Mat
    */
    Mat decode();

    /*! void saveEFInfo(int h, int w, string type, int fps,int frames,int blockSearchRadius, int blockSize,int nbits,string path)
    * will save info into the bin file
    * and save it on local variables in this class
    */
    void saveEFInfo(int h, int w, string type, int fps,int frames,int blockSearchRadius, int blockSize,int nbits,string path);
    /*! void readInfo()
    * will read info from the bin file
    * and save it on local variables in this class
    */
    void readEFInfo(string path);
    /*! void encodeA(const Mat& frame)
    * here we have the encode function that with a frame it will check which function is better
    * encode or encodeEF and if it has 3 channels or 1 and convert each channel using the selected mode and write in the bin file
    */
    void encodeA(const Mat& frame);
    /*! Mat decode()
    * here we have the decode function that with the bin file it will read what type of coding was used and decode it
    * using the correct method decode or decodeEF and convert into a single Mat
    */
    Mat decodeA();


    /*! void encodeEF(const Mat& frame)
    * here we have the encode function that with a frame it will divide into blocks
    * and using previous frame it will find the block difference with the least value
    * and code that diff block
    */
    void encodeEF(const Mat& frame);
    /*! Mat decodeEF()
    * here we have the decode function that with the bin file it will read a frame through its blocks and convert into a single Mat
    */
    Mat decodeEF();
    /*! double getbpp()
     * this will calculate bits per pixel used on this coded video
     */
     double getbpp();
};