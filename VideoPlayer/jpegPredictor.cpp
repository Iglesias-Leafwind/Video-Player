#include <iostream>
#include "opencv2/opencv.hpp"
#include "jpegPredictor.h"
#include <stdlib.h>     /* abs */

using namespace cv;
using namespace std;

void jpegPredictor::changeGolombM(int total){
    if(total <= 0){
        total = 1;
    }
    unsigned int count = (int)log2(total) + 1;
    count = (int)pow(2, count);

    if(count <= 2){
        count = 4;
    }
    if (count != golombo.m) {
        golombo.changeM(count);
    }
    golombo.ReadWrite.writeNbits(count, 8);
}

jpegPredictor::jpegPredictor(int mode) {
    this->mode = mode;
    this->bpp = 0;
    this->totalPixeis = 0;
    double dataLuminance[8][8] = {
            {16, 11, 10, 16, 24, 40, 51, 61},
            {12, 12, 14, 19, 26, 58, 60, 55},
            {14, 13, 16, 24, 40, 57, 69, 56},
            {14, 17, 22, 29, 51, 87, 80, 62},
            {18, 22, 37, 56, 68, 109, 103, 77},
            {24, 35, 55, 64, 81, 104, 113, 92},
            {49, 64, 78, 87, 103, 121, 120, 101},
            {72, 92, 95, 98, 112, 100, 103, 99}
    };

    double dataChrominance[8][8] = {
            {17, 18, 24, 27, 99, 99, 99, 99},
            {18, 21, 26, 66, 99, 99, 99, 99},
            {24, 26, 56, 99, 99, 99, 99, 99},
            {47, 66, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99},
            {99, 99, 99, 99, 99, 99, 99, 99}
    };
    this->luminance = Mat(8,8, CV_64FC1, &dataLuminance);
    this->chrominance = Mat(8,8, CV_64FC1, &dataChrominance);
}

signed int jpegPredictor::decalculator(vector<signed int> numb) {
    int a, b, c, x;
    c = numb.at(0);
    b = numb.at(1);
    a = numb.at(2);
    x = numb.at(3);
    x = x << this->nbits;
    if (mode == 1) {
        x = x + a;
    }
    else if (mode == 2) {
        x = x + b;
    }
    else if (mode == 3) {
        x = x + c;
    }
    else if (mode == 4) {
        x = x + (a + b - c);
    }
    else if (mode == 5) {
        x = x + (a + (b - c) / 2);
    }
    else if (mode == 6) {
        x = x + (b + (a - c) / 2);
    }
    else if (mode == 7) {
        x = x + ((a + b) / 2);
    }
    else if (mode == 8) {
        if (c >= max(a, b)) {
            x = x + min(a, b);
        }
        else if (c <= min(a, b)) {
            x = x + max(a, b);
        }
        else {
            x = x + (a + b - c);
        }
    }
    return x;
}

pair<signed int, signed int> jpegPredictor::calculator(const Mat& numb) {
    int a, b, c, x,decodedValue;
    c = numb.ptr<uchar>(0)[0];
    b = numb.ptr<uchar>(0)[1];
    a = numb.ptr<uchar>(1)[0];
    x = numb.ptr<uchar>(1)[1];
    pair<signed int,signed int> returning;
    if (mode == 1) {
        x = x - a;
    }
    else if (mode == 2) {
        x = x - b;
    }
    else if (mode == 3) {
        x = x - c;
    }
    else if (mode == 4) {
        x = x - (a + b - c);
    }
    else if (mode == 5) {
        x = x - (a + (b - c) / 2);
    }
    else if (mode == 6) {
        x = x - (b + (a - c) / 2);
    }
    else if (mode == 7) {
        x = x - ((a + b) / 2);
    }
    else if (mode == 8) {
        if (c >= max(a, b)) {
            x = x - min(a, b);
        }
        else if (c <= min(a, b)) {
            x = x - max(a, b);
        }
        else {
            x = x - (a + b - c);
        }
    }
    x = x >> this->nbits;
    returning.first = x;
    decodedValue = x << this->nbits;
    if (mode == 1) {
        decodedValue = decodedValue + a;
    }
    else if (mode == 2) {
        decodedValue = decodedValue + b;
    }
    else if (mode == 3) {
        decodedValue = decodedValue + c;
    }
    else if (mode == 4) {
        decodedValue = decodedValue + (a + b - c);
    }
    else if (mode == 5) {
        decodedValue = decodedValue + (a + (b - c) / 2);
    }
    else if (mode == 6) {
        decodedValue = decodedValue + (b + (a - c) / 2);
    }
    else if (mode == 7) {
        decodedValue = decodedValue + ((a + b) / 2);
    }
    else if (mode == 8) {
        if (c >= max(a, b)) {
            decodedValue = decodedValue + min(a, b);
        }
        else if (c <= min(a, b)) {
            decodedValue = decodedValue + max(a, b);
        }
        else {
            decodedValue = decodedValue + (a + b - c);
        }
    }
    returning.second = decodedValue;
    return returning;
}

Mat jpegPredictor::fillZeros(const Mat& channel) {
    Mat returning = Mat::zeros(channel.rows+1, channel.cols+1, CV_8UC1);
    channel.copyTo(returning(Rect(1, 1, channel.cols, channel.rows)));
    return returning;
}

void jpegPredictor::saveInfo(int h, int w, string type, int fps, int frames,int nbits,string path) {
    golombo = Golomb(path, 10, "w");
    rwFlag = 1;
    golombo.encode(mode);
    this->videoHeight = h;
    golombo.encode(h);
    this->videoWidth = w;
    golombo.encode(w);
    this->videoFrames = frames;
    golombo.encode(frames);
    this->type = type;
    if (type == "4:2:2") {
        golombo.ReadWrite.writeNbits(1, 2);
    }
    else if (type == "4:2:0") {
        golombo.ReadWrite.writeNbits(2, 2);
    }
    else {
        golombo.ReadWrite.writeNbits(0, 2);
    }
    this->videoFPS = fps;
    golombo.encode(fps);
    this->nbits = nbits;
    golombo.encode(nbits);
    golombo.changeM(8);
    golombo.ReadWrite.writeNbits(8, 8);
}

void jpegPredictor::readInfo(string path) {
    golombo = Golomb(path, 10, "r");
    rwFlag = 0;
    this->mode = golombo.decode();
    this->videoHeight = golombo.decode();
    this->videoWidth = golombo.decode();
    this->videoFrames = golombo.decode();
    int doType = (int)golombo.ReadWrite.readNbits(2);
    if (doType == 1) {
        this->type = "4:2:2";
    }
    else if (doType == 2) {
        this->type = "4:2:0";
    }
    else {
        this->type = "4:4:4";
    }
    this->videoFPS = golombo.decode();
    this->nbits = golombo.decode();
    golombo.changeM(8);
}

Mat jpegPredictor::frameWrite3Channel(Mat frame) {
    Mat colorChannel[3];
    split(frame, colorChannel);
    Mat currentY = fillZeros(colorChannel[0]);
    Mat currentU = fillZeros(colorChannel[1]);
    Mat currentV = fillZeros(colorChannel[2]);
    Mat square(2, 2, CV_8UC1);
    int total = 0;
    signed int numb = 0;
    pair<signed int,signed int> result;
    for (int linhas = 0; linhas < videoHeight; linhas++)
    {

        for (int colunas = 0; colunas < videoWidth; colunas++)
        {
            //square = currentY(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentY.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentY.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentY.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentY.ptr<uchar>(linhas+1)[colunas+1];
            result = calculator(square);
            numb = result.first;
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            bpp += golombo.fakeEncode(numb);
            golombo.encode(numb);
            currentY.ptr<uchar>(linhas+1)[colunas+1] = result.second;
            frame.ptr<Vec3b>(linhas)[colunas][0] = result.second;

            //square = currentU(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentU.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentU.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentU.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentU.ptr<uchar>(linhas+1)[colunas+1];
            result = calculator(square);
            numb = result.first;
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            bpp += golombo.fakeEncode(numb);
            golombo.encode(numb);
            currentU.ptr<uchar>(linhas+1)[colunas+1] = result.second;
            frame.ptr<Vec3b>(linhas)[colunas][1] = result.second;

            //square = currentV(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentV.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentV.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentV.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentV.ptr<uchar>(linhas+1)[colunas+1];
            result = calculator(square);
            numb = result.first;
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            bpp += golombo.fakeEncode(numb);
            golombo.encode(numb);
            currentV.ptr<uchar>(linhas+1)[colunas+1] = result.second;
            frame.ptr<Vec3b>(linhas)[colunas][2] = result.second;
        }
    }
    totalPixeis += bpp / (frame.rows * frame.cols * 3);
    bpp = 0;
    total = total / (frame.rows * frame.cols * 3);
    changeGolombM(total);
    return frame;
}

Mat jpegPredictor::frameWrite1Channel(Mat frame) {
    int lines = frame.rows;
    int cols = frame.cols;
    Mat current = fillZeros(frame);
    Mat square(2, 2, CV_8UC1);
    int total = 0;
    signed int numb = 0;
    pair<signed int,signed int> result;
    for (int linhas = 0; linhas < lines; linhas++)
    {

        for (int colunas = 0; colunas < cols; colunas++)
        {
            //square = current(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = current.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = current.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = current.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = current.ptr<uchar>(linhas+1)[colunas+1];
            result = calculator(square);
            numb = result.first;
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            bpp += golombo.fakeEncode(numb);
            golombo.encode(numb);
            current.ptr<uchar>(linhas+1)[colunas+1] = result.second;
            frame.ptr<uchar>(linhas)[colunas] = result.second;
        }
    }
    totalPixeis += bpp / (lines * cols);
    bpp = 0;
    total = total / (lines * cols);
    changeGolombM(total);
    return frame;
}

Mat jpegPredictor::frameRead3Channel() {
    vector<signed int> tubytu(4);
    Mat result(videoHeight,videoWidth , CV_8UC3);
    for (int linhas = 0; linhas < videoHeight; linhas++)
    {
        for (int colunas = 0; colunas < videoWidth; colunas++)
        {
            tubytu[0] = 0;
            tubytu[1] = 0;
            tubytu[2] = 0;
            tubytu[3] = 0;
            if (linhas == 0) {
                if (colunas == 0) {
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][0] = decalculator(tubytu);
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][1] = decalculator(tubytu);
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][2] = decalculator(tubytu);
                }
                else {
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][0];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][0] = decalculator(tubytu);
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][1];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][1] = decalculator(tubytu);
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][2];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][2] = decalculator(tubytu);
                }
            }
            else {
                if (colunas == 0) {
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][0];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][0] = decalculator(tubytu);
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][1];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][1] = decalculator(tubytu);
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][2];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][2] = decalculator(tubytu);
                }
                else {
                    tubytu[0] = result.ptr<Vec3b>(linhas - 1)[colunas - 1][0];
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][0];
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][0];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][0] = decalculator(tubytu);
                    tubytu[0] = result.ptr<Vec3b>(linhas - 1)[colunas - 1][1];
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][1];
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][1];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][1] = decalculator(tubytu);
                    tubytu[0] = result.ptr<Vec3b>(linhas - 1)[colunas - 1][2];
                    tubytu[1] = result.ptr<Vec3b>(linhas - 1)[colunas][2];
                    tubytu[2] = result.ptr<Vec3b>(linhas)[colunas - 1][2];
                    tubytu[3] = golombo.decode();
                    result.ptr<Vec3b>(linhas)[colunas][2] = decalculator(tubytu);
                }
            }
        }
    }//sV81//GGg*sV8
    return result;
}

Mat jpegPredictor::frameRead1Channel() {
    vector<signed int> tubytu(4);
    int lines = videoHeight;
    int cols = videoWidth;
    if (type == "4:2:2") {
        lines = lines * 2;
    }
    else {
        lines = lines * 3 / 2;
    }
    Mat result(lines, cols, CV_8UC1);
    for (int linhas = 0; linhas < lines; linhas++)
    {
        for (int colunas = 0; colunas < cols; colunas++)
        {
            tubytu[0] = 0;
            tubytu[1] = 0;
            tubytu[2] = 0;
            tubytu[3] = 0;
            if (linhas == 0) {
                if (colunas == 0) {
                    tubytu[3] = golombo.decode();
                    result.ptr<uchar>(linhas)[colunas] = decalculator(tubytu);
                }
                else {
                    tubytu[2] = result.ptr<uchar>(linhas)[colunas - 1];
                    tubytu[3] = golombo.decode();
                    result.ptr<uchar>(linhas)[colunas] = decalculator(tubytu);
                }
            }
            else {
                if (colunas == 0) {
                    tubytu[1] = result.ptr<uchar>(linhas - 1)[colunas];
                    tubytu[3] = golombo.decode();
                    result.ptr<uchar>(linhas)[colunas] = decalculator(tubytu);
                }
                else {
                    tubytu[0] = result.ptr<uchar>(linhas - 1)[colunas - 1];
                    tubytu[1] = result.ptr<uchar>(linhas - 1)[colunas];
                    tubytu[2] = result.ptr<uchar>(linhas)[colunas - 1];
                    tubytu[3] = golombo.decode();
                    result.ptr<uchar>(linhas)[colunas] = decalculator(tubytu);
                }
            }
        }
    }
    return result;
}

void jpegPredictor::encode(const Mat& frame) {
    if (not rwFlag) {
        golombo.turnaround();
        rwFlag = 1;
    }
    if (type == "4:4:4") {
        prevFrame = frameWrite3Channel(frame);
    }
    else {
        prevFrame = frameWrite1Channel(frame);
    }
}

Mat jpegPredictor::decode() {
    if (rwFlag) {
        golombo.turnaround();
        rwFlag = 0;
    }
    if(not(prevFrameF)){
        int count = golombo.ReadWrite.readNbits(8);
        golombo.changeM(count);
    }
    if (type == "4:4:4") {
        return frameRead3Channel();//GGGsV81//GGg*sV8
    }
    return frameRead1Channel();//GGGsV81//GGg*sV8
}

void jpegPredictor::saveEFInfo(int h, int w, string type, int fps,int frames,int blockSearchRadius,int blockSize,int nbits,string path) {
    golombo = Golomb(path, 10, "w");
    rwFlag = 1;
    golombo.encode(mode);
    this->videoHeight = h;
    golombo.encode(h);
    this->videoWidth = w;
    golombo.encode(w);
    this->videoFrames = frames;
    golombo.encode(frames);
    this->type = type;
    if (type == "4:2:2") {
        golombo.ReadWrite.writeNbits(1, 2);
    }
    else if (type == "4:2:0") {
        golombo.ReadWrite.writeNbits(2, 2);
    }
    else {
        golombo.ReadWrite.writeNbits(0, 2);
    }
    this->videoFPS = fps;
    golombo.encode(fps);
    this->blockSearchRadius = blockSearchRadius;
    golombo.encode(blockSearchRadius);
    this->blockSize = blockSize;
    golombo.encode(blockSize);
    this->nbits = nbits;
    golombo.encode(nbits);
    golombo.changeM(8);
    golombo.ReadWrite.writeNbits(8, 8);
}

void jpegPredictor::readEFInfo(string path) {
    golombo = Golomb(path, 10, "r");
    rwFlag = 0;
    this->mode = golombo.decode();
    this->videoHeight = golombo.decode();
    this->videoWidth = golombo.decode();
    this->videoFrames = golombo.decode();
    int doType = (int)golombo.ReadWrite.readNbits(2);
    if (doType == 1) {
        this->type = "4:2:2";
    }
    else if (doType == 2) {
        this->type = "4:2:0";
    }
    else {
        this->type = "4:4:4";
    }
    this->videoFPS = golombo.decode();
    this->blockSearchRadius = golombo.decode();
    this->blockSize = golombo.decode();
    this->nbits = golombo.decode();
}

Mat jpegPredictor::blockFrameRead3Channel(){
    vector<signed int> block(blockSize);
    Mat result(videoHeight,videoWidth , CV_8UC3);
    int shiftL;
    int shiftC;
    int numb;
    for (int linhas = 0; linhas < videoHeight; linhas+=blockSize)
    {
        for (int colunas = 0; colunas < videoWidth; colunas+=blockSize)
        {
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            //creating blocks Y U V
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    numb = golombo.decode() << nbits;
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][0] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][0] + numb;
                }
            }
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    numb = golombo.decode() << nbits;
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][1] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][1] + numb;
                }
            }

            shiftL = golombo.decode();
            shiftC = golombo.decode();
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    numb = golombo.decode() << nbits;
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][2] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][2] + numb;
                }
            }

        }
    }
    return result;
}

Mat jpegPredictor::blockFrameRead1Channel(){
    int lines = videoHeight;
    int cols = videoWidth;
    if (type == "4:2:2") {
        lines = lines * 2;
    }
    else {
        lines = lines * 3 / 2;
    }
    Mat result(lines,cols , CV_8UC1);
    int shiftL;
    int shiftC;
    int numb;
    Mat block(Size(8, 8), CV_64FC1);
    for (int linhas = 0; linhas < lines; linhas+=blockSize)
    {
        for (int colunas = 0; colunas < cols; colunas+=blockSize)
        {
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            //creating blocks
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    block.ptr<float>(lb)[cb] = golombo.decode();
                }
            }

            subtract(block,128.0,block);
            multiply(block,luminance,block);
            idct(block,block);
            add(block,128.0,block);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<float>(lb)[cb];
                    numb = numb << nbits;
                    unsigned char prevNumb = prevFrame.ptr<uchar>(linhas+lb+shiftL)[colunas+cb+shiftC];
                    result.ptr<uchar>(linhas+lb)[colunas+cb] = prevNumb - numb;
                }
            }


        }
    }
    return result;
}

Mat jpegPredictor::blockFrameWrite3Channel(Mat frame){
    //create vectors to be saved of each channel diff
    //get absolute sum of each vector
    int vecSize = blockSize*blockSize;
    vector<int> blockLeastDiffY(vecSize);
    vector<int> shiftY(2);
    int sumLeastDiffY = 255*vecSize;
    vector<int> blockLeastDiffU(vecSize);
    vector<int> shiftU(2);
    int sumLeastDiffU = 255*vecSize;
    vector<int> blockLeastDiffV(vecSize);
    vector<int> shiftV(2);
    int sumLeastDiffV = 255*vecSize;
    for(int x = 0; x < vecSize; ++x)
        blockLeastDiffV[x] = 255;
    Mat colorChannel[3];
    split(frame, colorChannel);
    //using Y, U, V
    Mat Y = colorChannel[0];
    Mat U = colorChannel[1];
    Mat V = colorChannel[2];
    int total = 0;
    vector<int> currentblockY(vecSize);
    vector<int> currentblockU(vecSize);
    vector<int> currentblockV(vecSize);
    for (int linhas = 0; linhas < videoHeight; linhas+=blockSize) {
        for (int colunas = 0; colunas < videoWidth; colunas+=blockSize) {
            //checkind diff from prev frame on each pixel using search radius
            for (int raioh = -blockSearchRadius; raioh <= blockSearchRadius; raioh++) {
                if(sumLeastDiffY+sumLeastDiffU+sumLeastDiffV == 0){
                    break;
                }
                if (linhas+raioh < 0 or linhas+raioh > videoHeight-blockSize) {
                    continue;
                }
                for (int raiow = -blockSearchRadius; raiow <= blockSearchRadius; raiow++) {
                    if(sumLeastDiffY+sumLeastDiffU+sumLeastDiffV == 0){
                        break;
                    }
                    if (colunas+raiow < 0 or colunas+raiow > videoWidth-blockSize) {
                        continue;
                    }
                    int sumCurrentBlockY = 0;
                    int sumCurrentBlockU = 0;
                    int sumCurrentBlockV = 0;
                    for (int lb = 0; lb < blockSize; lb++) {
                        for (int cb = 0; cb < blockSize; cb++) {
                            if(sumLeastDiffY != 0){
                                currentblockY[lb*blockSize+cb] = Y.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][0];
                                sumCurrentBlockY = sumCurrentBlockY + abs(currentblockY[lb*blockSize+cb]);
                            }
                            if(sumLeastDiffU != 0){
                                currentblockU[lb*blockSize+cb] = U.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][1];
                                sumCurrentBlockU = sumCurrentBlockU + abs(currentblockU[lb*blockSize+cb]);
                            }
                            if(sumLeastDiffV != 0){
                                currentblockV[lb*blockSize+cb] = V.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][2];
                                sumCurrentBlockV = sumCurrentBlockV + abs(currentblockV[lb*blockSize+cb]);
                            }
                        }
                    }

                    if(sumCurrentBlockY < sumLeastDiffY){
                        blockLeastDiffY = currentblockY;
                        sumLeastDiffY = sumCurrentBlockY;
                        shiftY[0] = raioh;
                        shiftY[1] = raiow;
                    }
                    if(sumCurrentBlockU < sumLeastDiffU){
                        blockLeastDiffU = currentblockU;
                        sumLeastDiffU = sumCurrentBlockU;
                        shiftU[0] = raioh;
                        shiftU[1] = raiow;
                    }
                    if(sumCurrentBlockV < sumLeastDiffV){
                        blockLeastDiffV = currentblockV;
                        sumLeastDiffV = sumCurrentBlockV;
                        shiftV[0] = raioh;
                        shiftV[1] = raiow;
                    }
                }
            }
            int numb;
            //codificar blocos
            bpp += golombo.fakeEncode(shiftY[0]);
            golombo.encode(shiftY[0]); //G
            bpp += golombo.fakeEncode(shiftY[1]);
            golombo.encode(shiftY[1]); //GG
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffY[lb*blockSize+cb] >> nbits;
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = numb << nbits;
                    Y.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shiftY[0])[colunas + cb + shiftY[1]] + numb;
                }
            }
            bpp += golombo.fakeEncode(shiftU[0]);
            golombo.encode(shiftU[0]);
            bpp += golombo.fakeEncode(shiftU[1]);
            golombo.encode(shiftU[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffU[lb*blockSize+cb] >> nbits;
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = numb << nbits;
                    U.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shiftU[0])[colunas + cb + shiftU[1]] + numb;
                }
            }
            bpp += golombo.fakeEncode(shiftV[0]);
            golombo.encode(shiftV[0]);
            bpp += golombo.fakeEncode(shiftV[1]);
            golombo.encode(shiftV[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffV[lb*blockSize+cb] >> nbits;
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = numb << nbits;
                    V.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shiftV[0])[colunas + cb + shiftV[1]] + numb;
                }
            }
            total = total + sumLeastDiffY + sumLeastDiffU + sumLeastDiffV;
            sumLeastDiffY = 255*vecSize;
            sumLeastDiffU = 255*vecSize;
            sumLeastDiffV = 255*vecSize;
        }
    }
    totalPixeis += bpp / (frame.rows * frame.cols * 3);
    bpp = 0;
    total = total / (frame.rows * frame.cols * 3);
    changeGolombM(total);
    return frame;
}

Mat jpegPredictor::blockFrameWrite1Channel(Mat frame){
    //create vectors to be saved of each channel diff
    //get absolute sum of each vector
    int lines = frame.rows;
    int cols = frame.cols;
    int vecSize = blockSize*blockSize;
    vector<int> blockLeastDiff(vecSize);
    vector<int> shift(2);
    int sumLeastDiff = 255*vecSize;
    int total = 0;
    vector<int> currentblock(vecSize);
    Mat block(Size(8, 8), CV_64FC1),returning(Size(8, 8), CV_64FC1);
    for (int linhas = 0; linhas < lines; linhas+=blockSize) {
        for (int colunas = 0; colunas < cols; colunas+=blockSize) {
            //checkind diff from prev frame on each pixel using search radius
            for (int raioh = -blockSearchRadius; raioh <= blockSearchRadius; raioh++) {
                if(sumLeastDiff == 0){
                    break;
                }
                if (linhas+raioh < 0 or linhas+raioh > lines-blockSize) {
                    continue;
                }
                for (int raiow = -blockSearchRadius; raiow <= blockSearchRadius; raiow++) {
                    if(sumLeastDiff == 0){
                        break;
                    }
                    if (colunas+raiow < 0 or colunas+raiow > cols-blockSize) {
                        continue;
                    }
                    int sumCurrentBlock = 0;
                    for (int lb = 0; lb < blockSize; ++lb) {
                        for (int cb = 0; cb < blockSize; ++cb) {
                            if (sumLeastDiff != 0) {
                                currentblock[lb * blockSize + cb] = prevFrame.ptr<uchar>(linhas + lb + raioh)[colunas + cb + raiow] - frame.ptr<uchar>(linhas + lb)[colunas + cb];
                                sumCurrentBlock = sumCurrentBlock + abs(currentblock[lb * blockSize + cb]);
                            }
                        }
                    }

                    if(sumCurrentBlock < sumLeastDiff){
                        blockLeastDiff = currentblock;
                        sumLeastDiff = sumCurrentBlock;
                        shift[0] = raioh;
                        shift[1] = raiow;
                    }
                }
            }
            //codificar blocos
            int numb;
            bpp += golombo.fakeEncode(shift[0]);
            golombo.encode(shift[0]);
            bpp += golombo.fakeEncode(shift[1]);
            golombo.encode(shift[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiff[lb*blockSize+cb] >> nbits;
                    block.ptr<float>(lb)[cb] = numb;
                    /*
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = numb << nbits;
                    frame.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shift[0])[colunas + cb + shift[1]] - numb;
                     */
                }
            }
            subtract(block,128.0,block);
            dct(block,block);
            divide( block, luminance, block );
            multiply(block,luminance,returning);
            idct(returning,returning);
            add(block,128.0,block);
            add(returning,128.0,returning);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<float>(lb)[cb];
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = returning.ptr<float>(lb)[cb];
                    numb = numb << nbits;
                    frame.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shift[0])[colunas + cb + shift[1]] - numb;
                }
            }
            total = total + sumLeastDiff;
            sumLeastDiff = 255*vecSize;
        }
    }
    totalPixeis += bpp / (lines * cols);
    bpp = 0;
    total = total / (lines*cols);
    changeGolombM(total);
    return frame;
}

void jpegPredictor::encodeEF(const Mat& frame){
    if (not rwFlag) {
        golombo.turnaround();
        rwFlag = 1;
    }
    if (type == "4:4:4") {
        prevFrame = blockFrameWrite3Channel(frame);
    }
    else {
        prevFrame = blockFrameWrite1Channel(frame);
    }

}

Mat jpegPredictor::decodeEF(){
    if (rwFlag) {
        golombo.turnaround();
        rwFlag = 0;
    }
    Mat returning;
    if (type == "4:4:4") {
        returning = blockFrameRead3Channel();
    }else{
        returning = blockFrameRead1Channel();
    }
    prevFrame = returning;
    return returning;
}

Mat customYUV2BGRtest(Mat YUV, String mode = "4:4:4") {
    Mat result;
    if (mode == "4:2:2") {
        Mat bmat(YUV.rows / 2, YUV.cols, CV_8UC1);
        Mat gmat(YUV.rows / 2, YUV.cols, CV_8UC1);
        Mat rmat(YUV.rows / 2, YUV.cols, CV_8UC1);

        int u_row = YUV.rows / 2;
        int u_col = 0;
        int v_row = YUV.rows * 3 / 4;
        int v_col = 0;
        unsigned char Y;
        unsigned char U;
        unsigned char V;
        for (int i = 0; i < YUV.rows / 2; i++)
        {
            if (u_col == YUV.cols) {
                u_row++;
                u_col = 0;
            }
            if (v_col == YUV.cols) {
                v_row++;
                v_col = 0;
            }

            for (int j = 0; j < YUV.cols; j++)
            {
                Y = YUV.ptr<uchar>(i)[j];
                if (j % 2 == 0) {
                    U = YUV.ptr<uchar>(u_row)[u_col++];
                    V = YUV.ptr<uchar>(v_row)[v_col++];
                }

                int B = Y + 1.772 * U - 226.816;
                int G = Y - 0.344136 * U - 0.714136 * V + 135.459;
                int R = Y + 1.402 * V - 179.456;

                if (B < 0) B = 0;
                if (G < 0) G = 0;
                if (R < 0) R = 0;

                if (B > 255) B = 255;
                if (G > 255) G = 255;
                if (R > 255) R = 255;

                bmat.ptr<uchar>(i)[j] = B;
                gmat.ptr<uchar>(i)[j] = G;
                rmat.ptr<uchar>(i)[j] = R;
            }
        }
        std::vector<Mat> channels{ bmat, gmat, rmat };

        // Create the output matrix
        merge(channels, result);
        return result;
    }
    else if (mode == "4:2:0") {
        int rows = YUV.rows;
        int cols = YUV.cols;

        int u_begin = rows * 2 / 3;
        int v_begin = u_begin * 5 / 4;

        int u_cols = 0;
        int v_cols = 0;

        Mat b_array(u_begin, cols, CV_8UC1);
        Mat g_array(u_begin, cols, CV_8UC1);
        Mat r_array(u_begin, cols, CV_8UC1);

        for (int i = 0; i < rows*2/3; i += 2) {

            if (u_cols == cols) {
                u_begin++;
                u_cols = 0;
            }
            if (v_cols == cols) {
                v_begin++;
                v_cols = 0;
            }
            unsigned  char Y1;
            unsigned char Y2;
            unsigned char U;
            unsigned char V;
            for (int j = 0; j < cols; j++) {

                Y1 = YUV.ptr<uchar>(i)[j];
                Y2 = YUV.ptr<uchar>(i + 1)[j];
                if (v_begin == rows) break;

                if (j % 2 == 0) {
                    U = YUV.ptr<uchar>(u_begin)[u_cols++];
                    V = YUV.ptr<uchar>(v_begin)[v_cols++];
                }
                int B1 = Y1 + 1.772 * U - 226.816;
                int G1 = Y1 - 0.344136 * U - 0.714136 * V + 135.459;
                int R1 = Y1 + 1.402 * V - 179.456;

                if (B1 < 0) B1 = 0;
                if (G1 < 0) G1 = 0;
                if (R1 < 0) R1 = 0;

                if (B1 > 255) B1 = 255;
                if (G1 > 255) G1 = 255;
                if (R1 > 255) R1 = 255;

                int B2 = Y2 + 1.772 * U - 226.816;
                int G2 = Y2 - 0.344136 * U - 0.714136 * V + 135.459;
                int R2 = Y2 + 1.402 * V - 179.456;

                if (B2 < 0) B2 = 0;
                if (G2 < 0) G2 = 0;
                if (R2 < 0) R2 = 0;

                if (B2 > 255) B2 = 255;
                if (G2 > 255) G2 = 255;
                if (R2 > 255) R2 = 255;

                b_array.ptr<uchar>(i)[j] = B1;
                g_array.ptr<uchar>(i)[j] = G1;
                r_array.ptr<uchar>(i)[j] = R1;

                b_array.ptr<uchar>(i + 1)[j] = B2;
                g_array.ptr<uchar>(i + 1)[j] = G2;
                r_array.ptr<uchar>(i + 1)[j] = R2;
            }

        }
        Mat result;
        vector<Mat> channels{ b_array, g_array, r_array };
        merge(channels, result);
        return result;
    }
    else {

        Mat result(YUV.rows, YUV.cols, CV_8UC3);

        for (int i = 0; i < YUV.rows; i++)
        {

            for (int j = 0; j < YUV.cols; j++)
            {
                unsigned char Y = YUV.ptr<Vec3b>(i)[j][0];
                unsigned char U = YUV.ptr<Vec3b>(i)[j][1];
                unsigned char V = YUV.ptr<Vec3b>(i)[j][2];

                int B = Y + 1.772                 * U - 226.816;
                int G = Y - 0.344136 * U - 0.714136 * V + 135.459;
                int R = Y + 1.402 * V - 179.456;

                if (B < 0) B = 0;
                if (G < 0) G = 0;
                if (R < 0) R = 0;

                if (B > 255) B = 255;
                if (G > 255) G = 255;
                if (R > 255) R = 255;

                result.ptr<Vec3b>(i)[j][0] = B;
                result.ptr<Vec3b>(i)[j][1] = G;
                result.ptr<Vec3b>(i)[j][2] = R;
            }
        }
        return result;
    }
}

void jpegPredictor::encodeA(const Mat& frame) {
    if(prevFrameF){
        /*
        //testing some code
        Mat diffImage;
        absdiff(prevFrame, frame, diffImage);

        Mat foregroundMask = cv::Mat::zeros(diffImage.rows, diffImage.cols, CV_8UC1);

        float threshold = 30.0f;
        float dist;

        for (int j = 0; j < diffImage.rows; ++j)
            for (int i = 0; i < diffImage.cols; ++i) {
                if(type != "4:4:4"){
                    uchar pix = diffImage.at<uchar>(j, i);

                    dist = (pix * pix);
                    dist = sqrt(dist);
                    if (dist > threshold) {
                        foregroundMask.at<unsigned char>(j, i) = 255;
                    }

                }else{
                    Vec3b pix = diffImage.at<Vec3b>(j, i);

                    dist = (pix[0] * pix[0] + pix[1] * pix[1] + pix[2] * pix[2]);
                    dist = sqrt(dist);
                    if (dist < threshold) {
                        foregroundMask.at<unsigned char>(j, i) = 255;
                    }

                }

            }
            */
        //if(sum(foregroundMask)[0] <= 10000000){}
        if(type == "4:4:4"){
            fakePredict3Channel(frame);
        }else{
            fakePredict1Channel(frame);
        }
        if(bitsForInter <= bitsForIntra){
            cout << "E";
            golombo.ReadWrite.writebit(1);
            encodeEF(frame);
        }else{
            cout << "I";
            golombo.ReadWrite.writebit(0);
            encode(frame);
        }
    }else{
        cout << "I";
        golombo.ReadWrite.writebit(0);
        encode(frame);
        prevFrameF = true;
    }
    imshow("Frame",customYUV2BGRtest(prevFrame,"4:2:0"));
    waitKey(1);
}

Mat jpegPredictor::decodeA(){
    Mat returning;
    int count = golombo.ReadWrite.readNbits(8);
    golombo.changeM(count);
    if(prevFrameF){
        if(golombo.ReadWrite.readbit()){
            returning = decodeEF();
        }else{
            returning = decode();
        }
    }else{
        golombo.ReadWrite.readbit();
        prevFrameF = true;
        returning = decode();
    }
    prevFrame = returning;
    imshow("Frame",customYUV2BGRtest(prevFrame,"4:2:0"));
    waitKey(2000);
    return returning;
}

void jpegPredictor::fakePredict3Channel(const Mat& frame){
    bitsForIntra = 0;
    bitsForInter = 0;

    //create vectors to be saved of each channel diff
    //get absolute sum of each vector
    int vecSize = blockSize*blockSize;
    vector<int> blockLeastDiffY(vecSize);
    vector<int> shiftY(2);
    int sumLeastDiffY = 255*vecSize;
    vector<int> blockLeastDiffU(vecSize);
    vector<int> shiftU(2);
    int sumLeastDiffU = 255*vecSize;
    vector<int> blockLeastDiffV(vecSize);
    vector<int> shiftV(2);
    int sumLeastDiffV = 255*vecSize;
    for(int x = 0; x < vecSize; ++x)
        blockLeastDiffV[x] = 255;
    Mat colorChannel[3];
    split(frame, colorChannel);
    //using Y, U, V
    Mat Y = colorChannel[0];
    Mat U = colorChannel[1];
    Mat V = colorChannel[2];
    int total = 0;
    vector<int> currentblockY(vecSize);
    vector<int> currentblockU(vecSize);
    vector<int> currentblockV(vecSize);
    split(frame, colorChannel);
    Mat currentY = fillZeros(colorChannel[0]);
    Mat currentU = fillZeros(colorChannel[1]);
    Mat currentV = fillZeros(colorChannel[2]);
    Mat square(2, 2, CV_8UC1);
    signed int numb = 0;
    for (int linhas = 0; linhas < videoHeight; linhas+=blockSize) {
        for (int colunas = 0; colunas < videoWidth; colunas+=blockSize) {
            //checkind diff from prev frame on each pixel using search radius
            for (int raioh = -blockSearchRadius; raioh <= blockSearchRadius; raioh++) {
                if(sumLeastDiffY+sumLeastDiffU+sumLeastDiffV == 0){
                    break;
                }
                if (linhas+raioh < 0 or linhas+raioh > videoHeight-blockSize) {
                    continue;
                }
                for (int raiow = -blockSearchRadius; raiow <= blockSearchRadius; raiow++) {
                    if(sumLeastDiffY+sumLeastDiffU+sumLeastDiffV == 0){
                        break;
                    }
                    if (colunas+raiow < 0 or colunas+raiow > videoWidth-blockSize) {
                        continue;
                    }
                    int sumCurrentBlockY = 0;
                    int sumCurrentBlockU = 0;
                    int sumCurrentBlockV = 0;
                    for (int lb = 0; lb < blockSize; lb++) {
                        for (int cb = 0; cb < blockSize; cb++) {
                            if(sumLeastDiffY != 0){
                                currentblockY[lb*blockSize+cb] = Y.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][0];
                                sumCurrentBlockY = sumCurrentBlockY + abs(currentblockY[lb*blockSize+cb]);
                            }
                            if(sumLeastDiffU != 0){
                                currentblockU[lb*blockSize+cb] = U.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][1];
                                sumCurrentBlockU = sumCurrentBlockU + abs(currentblockU[lb*blockSize+cb]);
                            }
                            if(sumLeastDiffV != 0){
                                currentblockV[lb*blockSize+cb] = V.ptr<uchar>(linhas + lb)[colunas + cb] - prevFrame.ptr<Vec3b>(linhas + lb+raioh)[colunas + cb+raiow][2];
                                sumCurrentBlockV = sumCurrentBlockV + abs(currentblockV[lb*blockSize+cb]);
                            }
                        }
                    }

                    if(sumCurrentBlockY < sumLeastDiffY){
                        blockLeastDiffY = currentblockY;
                        sumLeastDiffY = sumCurrentBlockY;
                        shiftY[0] = raioh;
                        shiftY[1] = raiow;
                    }
                    if(sumCurrentBlockU < sumLeastDiffU){
                        blockLeastDiffU = currentblockU;
                        sumLeastDiffU = sumCurrentBlockU;
                        shiftU[0] = raioh;
                        shiftU[1] = raiow;
                    }
                    if(sumCurrentBlockV < sumLeastDiffV){
                        blockLeastDiffV = currentblockV;
                        sumLeastDiffV = sumCurrentBlockV;
                        shiftV[0] = raioh;
                        shiftV[1] = raiow;
                    }
                }
            }
            int numb;
            //codificar blocos
            bitsForInter += golombo.fakeEncode(shiftY[0]);
            bitsForInter += golombo.fakeEncode(shiftY[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffY[lb*blockSize+cb] >> nbits;
                    bitsForInter += golombo.fakeEncode(numb);
                }
            }
            bitsForInter += golombo.fakeEncode(shiftU[0]);
            bitsForInter += golombo.fakeEncode(shiftU[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffU[lb*blockSize+cb] >> nbits;
                    bitsForInter += golombo.fakeEncode(numb);
                }
            }
            bitsForInter += golombo.fakeEncode(shiftV[0]);
            bitsForInter += golombo.fakeEncode(shiftV[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiffV[lb*blockSize+cb] >> nbits;
                    bitsForInter += golombo.fakeEncode(numb);
                }
            }
        }
    }
    pair<signed int,signed int> result;
    for (int linhas = 0; linhas < videoHeight; linhas++) {

        for (int colunas = 0; colunas < videoWidth; colunas++) {
            //square = currentY(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentY.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentY.ptr<uchar>(linhas)[colunas + 1];
            square.ptr<uchar>(1)[0] = currentY.ptr<uchar>(linhas + 1)[colunas];
            square.ptr<uchar>(1)[1] = currentY.ptr<uchar>(linhas + 1)[colunas + 1];
            result = calculator(square);
            numb = result.first;
            bitsForIntra += golombo.fakeEncode(numb);
            currentY.ptr<uchar>(linhas + 1)[colunas + 1] = result.second;

            //square = currentU(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentU.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentU.ptr<uchar>(linhas)[colunas + 1];
            square.ptr<uchar>(1)[0] = currentU.ptr<uchar>(linhas + 1)[colunas];
            square.ptr<uchar>(1)[1] = currentU.ptr<uchar>(linhas + 1)[colunas + 1];
            result = calculator(square);
            numb = result.first;
            bitsForIntra += golombo.fakeEncode(numb);
            currentU.ptr<uchar>(linhas + 1)[colunas + 1] = result.second;

            //square = currentV(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentV.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentV.ptr<uchar>(linhas)[colunas + 1];
            square.ptr<uchar>(1)[0] = currentV.ptr<uchar>(linhas + 1)[colunas];
            square.ptr<uchar>(1)[1] = currentV.ptr<uchar>(linhas + 1)[colunas + 1];
            result = calculator(square);
            numb = result.first;
            bitsForIntra += golombo.fakeEncode(numb);
            currentV.ptr<uchar>(linhas + 1)[colunas + 1] = result.second;
        }
    }
}

void jpegPredictor::fakePredict1Channel(const Mat& frame){
    bitsForIntra = 0;
    bitsForInter = 0;

    int lines = frame.rows;
    int cols = frame.cols;
    //create vectors to be saved of each channel diff
    //get absolute sum of each vector
    int vecSize = blockSize*blockSize;
    vector<int> blockLeastDiff(vecSize);
    for(int x = 0; x < vecSize; ++x)
        blockLeastDiff[x] = 255;
    vector<int> shift(2);
    int sumLeastDiff = 255*vecSize;
    int total = 0;
    vector<int> currentblock(vecSize);
    Mat current = fillZeros(frame);
    Mat square(2, 2, CV_8UC1);
    Mat block(Size(8, 8), CV_64FC1);
    signed int numb = 0;
    for (int linhas = 0; linhas < lines; linhas+=blockSize) {
        for (int colunas = 0; colunas < cols; colunas+=blockSize) {
            //checkind diff from prev frame on each pixel using search radius
            for (int raioh = -blockSearchRadius; raioh <= blockSearchRadius; raioh++) {
                if(sumLeastDiff == 0){
                    break;
                }
                if (linhas+raioh < 0 or linhas+raioh > lines-blockSize) {
                    continue;
                }
                for (int raiow = -blockSearchRadius; raiow <= blockSearchRadius; raiow++) {
                    if(sumLeastDiff == 0){
                        break;
                    }
                    if (colunas+raiow < 0 or colunas+raiow > cols-blockSize) {
                        continue;
                    }
                    int sumCurrentBlock = 0;
                    for (int lb = 0; lb < blockSize; ++lb) {
                        for (int cb = 0; cb < blockSize; ++cb) {
                            if (sumLeastDiff != 0) {
                                currentblock[lb * blockSize + cb] = prevFrame.ptr<uchar>(linhas + lb + raioh)[colunas + cb + raiow] - frame.ptr<uchar>(linhas + lb)[colunas + cb];
                                sumCurrentBlock = sumCurrentBlock + abs(currentblock[lb * blockSize + cb]);
                            }
                        }
                    }

                    if(sumCurrentBlock < sumLeastDiff){
                        blockLeastDiff = currentblock;
                        sumLeastDiff = sumCurrentBlock;
                        shift[0] = raioh;
                        shift[1] = raiow;
                    }
                }
            }
            //codificar blocos
            int numb;
            bitsForInter += golombo.fakeEncode(shift[0]);
            bitsForInter += golombo.fakeEncode(shift[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = blockLeastDiff[lb*blockSize+cb] >> nbits;
                    block.ptr<float>(lb)[cb] = numb;
                    /*
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = numb << nbits;
                    frame.ptr<uchar>(linhas + lb)[colunas + cb] = prevFrame.ptr<uchar>(linhas + lb + shift[0])[colunas + cb + shift[1]] - numb;
                     */
                }
            }
            subtract(block,128.0,block);
            dct(block,block);
            divide( block, luminance, block );
            add(block,128.0,block);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<float>(lb)[cb];
                    bitsForInter += golombo.fakeEncode(numb);
                }
            }
        }
    }
    pair<signed int,signed int> result;
    for (int linhas = 0; linhas < lines; linhas++)
    {

        for (int colunas = 0; colunas < cols; colunas++)
        {
            //square = current(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = current.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = current.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = current.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = current.ptr<uchar>(linhas+1)[colunas+1];
            result = calculator(square);
            numb = result.first;
            bitsForIntra += golombo.fakeEncode(numb);
            current.ptr<uchar>(linhas+1)[colunas+1] = result.second;
        }
    }
}

double jpegPredictor::getbpp(){
    double result = (totalPixeis*1.0)/(videoFrames*1.0);
    cout << totalPixeis << " / "<< videoFrames << " = " << result << endl;
    return result;
}