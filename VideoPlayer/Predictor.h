
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "Golomb.h"

using namespace cv;
using namespace std;



/*! class Predictor
* this class is used to convert the data from a frame / image to a more repeatable pattern
*/
class Predictor {
private:
    /*! bool rwFlag
    *   Tells us if it is reading or writing
    *  starting with 1 == writing
    */
    bool rwFlag = 1;
    /*! Mat fillZeros(Mat channel)
    * given a Mat with a channel it will fill the 1st row and 1st column with zeros
    * for easier future calculation
    */
    Mat fillZeros(Mat channel);
    /*! signed int calculator(Mat numb)
    * this function simply gets the numbers from the mat and depending on the mode we are using it will calculate the correct precdictor
    */
    signed int calculator(Mat numb);
    /*! signed int decalculator(vector<signed int> numb)
    * this function simply gets the vector numbers and depending on the mode we are using it will calculate the original number
    */
    signed int decalculator(vector<signed int> numb);
    /*! Mat frameRead3Channel()
    * will read a 3channels frame from bin file
    */
    Mat frameRead3Channel();
    /*! Mat frameRead1Channel()
    * will read a 1channel frame from bin file
    */
    Mat frameRead1Channel();
    /*! Mat frameWrite3Channel(Mat frame)
    * will write a 3channels frame into the bin file
    */
    void frameWrite3Channel(Mat frame);
    /*! Mat frameWrite1Channel(Mat frame)
    * will write a 1channel frame into the bin file
    */
    void frameWrite1Channel(Mat frame);

public:
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
    /*! Predictor(int mode,string path)
    * this is the contructor that will accept the mode we want to use to "encode" / "convert" / "decode"
    * and uses path where it will save the encoded file or read the encoded file
    */
    Predictor(int mode);
    /*! void saveInfo(int h, int w, string type, int fps)
    * will save info into the bin file
    * and save it on local variables in this class
    */
    void saveInfo(int h, int w, string type, int fps,int frames,string path);
    /*! void readInfo()
    * will read info from the bin file
    * and save it on local variables in this class
    */
    void readInfo(string path);
    /*! void encode(Mat frame)
    * here we have the encode function that with a frame it will choose if it has 3 channels or 1 and convert each channel using the selected mode and write in the bin file
    */
    void encode(Mat frame);
    /*! Mat decode(void)
    * here we have the decode function that with the bin file it will read a frame and convert into a single Mat
    */
    Mat decode();
    
};