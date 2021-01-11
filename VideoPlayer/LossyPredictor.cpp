#include <iostream>
#include "opencv2/opencv.hpp"
#include "LossyPredictor.h"
#include <stdlib.h>     /* abs */

using namespace cv;
using namespace std;

void LossyPredictor::changeGolombM(int total){
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

LossyPredictor::LossyPredictor(int mode) {
    this->mode = mode;
    this->bpp = 0;
    this->totalPixeis = 0;
}

signed int LossyPredictor::decalculator(vector<signed int> numb) {
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

pair<signed int, signed int> LossyPredictor::calculator(const Mat& numb) {
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

Mat LossyPredictor::fillZeros(const Mat& channel) {
    Mat returning = Mat::zeros(channel.rows+1, channel.cols+1, CV_8UC1);
    channel.copyTo(returning(Rect(1, 1, channel.cols, channel.rows)));
    return returning;
}

void LossyPredictor::saveInfo(int h, int w, string type, int fps, int frames,int nbits,string path) {
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

void LossyPredictor::readInfo(string path) {
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

Mat LossyPredictor::frameWrite3Channel(Mat frame) {
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

Mat LossyPredictor::frameWrite1Channel(Mat frame) {
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

Mat LossyPredictor::frameRead3Channel() {
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

Mat LossyPredictor::frameRead1Channel() {
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

void LossyPredictor::encode(const Mat& frame) {
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

Mat LossyPredictor::decode() {
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

void LossyPredictor::saveEFInfo(int h, int w, string type, int fps,int frames,int blockSearchRadius, int blockSize,int nbits,string path) {
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
        while(w%blockSize != 0 or (h*2)%blockSize != 0){
            blockSize = blockSize/2;
        }
        golombo.ReadWrite.writeNbits(1, 2);
    }
    else if (type == "4:2:0") {
        while(w%blockSize != 0 or (h*3/2)%blockSize != 0){
            blockSize = blockSize/2;
        }
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

void LossyPredictor::readEFInfo(string path) {
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

Mat LossyPredictor::blockFrameRead3Channel(){
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

Mat LossyPredictor::blockFrameRead1Channel(){
    int lines = videoHeight;
    int cols = videoWidth;
    if (type == "4:2:2") {
        lines = lines * 2;
    }
    else {
        lines = lines * 3 / 2;
    }
    vector<signed int> block(blockSize);
    Mat result(lines,cols , CV_8UC1);
    int shiftL;
    int shiftC;
    int numb;
    for (int linhas = 0; linhas < lines; linhas+=blockSize)
    {
        for (int colunas = 0; colunas < cols; colunas+=blockSize)
        {
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            //creating blocks
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = golombo.decode()  << nbits;
                    result.ptr<uchar>(linhas+lb)[colunas+cb] = prevFrame.ptr<uchar>(linhas+lb+shiftL)[colunas+cb+shiftC] - numb;
                }
            }

        }
    }
    return result;
}

Mat LossyPredictor::blockFrameWrite3Channel(Mat frame){
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

Mat LossyPredictor::blockFrameWrite1Channel(Mat frame){
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
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
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

void LossyPredictor::encodeEF(const Mat& frame){
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

Mat LossyPredictor::decodeEF(){
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

void LossyPredictor::encodeA(const Mat& frame) {
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
            golombo.ReadWrite.writebit(1);
            encodeEF(frame);
        }else{
            golombo.ReadWrite.writebit(0);
            encode(frame);
        }
    }else{
        golombo.ReadWrite.writebit(0);
        encode(frame);
        prevFrameF = true;
    }
}

Mat LossyPredictor::decodeA(){
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
    return returning;
}

void LossyPredictor::fakePredict3Channel(const Mat& frame){
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

void LossyPredictor::fakePredict1Channel(const Mat& frame){
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

double LossyPredictor::getbpp(){
    double result = (totalPixeis*1.0)/(videoFrames*1.0);
    cout << totalPixeis << " / "<< videoFrames << " = " << result << endl;
    return result;
}