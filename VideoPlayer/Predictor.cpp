#include <iostream>
#include "opencv2/opencv.hpp"
#include "Predictor.h"
#include <stdlib.h>     /* abs */

using namespace cv;
using namespace std;

void Predictor::changeGolombM(int total){
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
Predictor::Predictor(int mode) {
    this->mode = mode;
}

signed int Predictor::decalculator(vector<signed int> numb) {
    int a, b, c, x;
    c = numb.at(0);
    b = numb.at(1);
    a = numb.at(2);
    x = numb.at(3);
    if (mode == 1) {
        return x + a;
    }
    else if (mode == 2) {
        return x + b;
    }
    else if (mode == 3) {
        return x + c;
    }
    else if (mode == 4) {
        return x + (a + b - c);
    }
    else if (mode == 5) {
        return x + (a + (b - c) / 2);
    }
    else if (mode == 6) {
        return x + (b + (a - c) / 2);
    }
    else if (mode == 7) {
        return x + ((a + b) / 2);
    }
    else if (mode == 8) {
        if (c >= max(a, b)) {
            return x + min(a, b);
        }
        else if (c <= min(a, b)) {
            return x + max(a, b);
        }
        else {
            return x + (a + b - c);
        }
    }
    return 0;
}

signed int Predictor::calculator(const Mat& numb) {
    int a, b, c, x;
    c = numb.ptr<uchar>(0)[0];
    b = numb.ptr<uchar>(0)[1];
    a = numb.ptr<uchar>(1)[0];
    x = numb.ptr<uchar>(1)[1];
    if (mode == 1) {
        return x - a;
    }
    else if (mode == 2) {
        return x - b;
    }
    else if (mode == 3) {
        return x - c;
    }
    else if (mode == 4) {
        return x - (a + b - c);
    }
    else if (mode == 5) {
        return x - (a + (b - c) / 2);
    }
    else if (mode == 6) {
        return x - (b + (a - c) / 2);
    }
    else if (mode == 7) {
        return x - ((a + b) / 2);
    }
    else if (mode == 8) {
        if (c >= max(a, b)) {
            return x - min(a, b);
        }
        else if (c <= min(a, b)) {
            return x - max(a, b);
        }
        else {
            return x - (a + b - c);
        }
    }
    return 0;
}

Mat Predictor::fillZeros(const Mat& channel) {
    Mat returning = Mat::zeros(channel.rows+1, channel.cols+1, CV_8UC1);
    channel.copyTo(returning(Rect(1, 1, channel.cols, channel.rows)));
    return returning;
}

void Predictor::saveInfo(int h, int w, string type, int fps, int frames,string path) {
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
    golombo.changeM(8);
    golombo.ReadWrite.writeNbits(8, 8);
}

void Predictor::readInfo(string path) {
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
    golombo.changeM(8);
}

void Predictor::frameWrite3Channel(const Mat& frame) {
    Mat colorChannel[3];
    split(frame, colorChannel);
    Mat currentY = fillZeros(colorChannel[0]);
    Mat currentU = fillZeros(colorChannel[1]);
    Mat currentV = fillZeros(colorChannel[2]);
    Mat square(2, 2, CV_8UC1);
    int total = 0;
    signed int numb = 0;
    for (int linhas = 0; linhas < videoHeight; linhas++)
    {

        for (int colunas = 0; colunas < videoWidth; colunas++)
        {
            //square = currentY(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentY.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentY.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentY.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentY.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            /*if (numb < 0) {
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }*/
            golombo.encode(numb);//G
            //square = currentU(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentU.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentU.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentU.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentU.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            /*if (numb < 0) {
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }*/
            golombo.encode(numb);//GG
            //square = currentV(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentV.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentV.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentV.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentV.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            if (numb < 0) {
                total = total + (numb * (-1));
            }
            else {
                total = total + numb * 1;
            }
            /*if (numb < 0) {
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }*/
            golombo.encode(numb);//GGG
        }
    }
    total = total / (frame.rows * frame.cols * 3);
    changeGolombM(total);
}

void Predictor::frameWrite1Channel(const Mat& frame) {
    int lines = frame.rows;
    int cols = frame.cols;
    Mat current = fillZeros(frame);
    Mat square(2, 2, CV_8UC1);
    int total = 0;
    signed int numb = 0;
    for (int linhas = 0; linhas < lines; linhas++)
    {

        for (int colunas = 0; colunas < cols; colunas++)
        {
            //square = current(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = current.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = current.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = current.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = current.ptr<uchar>(linhas+1)[colunas+1];

            numb = calculator(square);
            total += abs(numb);
            golombo.encode(numb);
        }
    }
    total = total / (lines * cols);
    changeGolombM(total);
}

Mat Predictor::frameRead3Channel() {
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

Mat Predictor::frameRead1Channel() {
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

void Predictor::encode(const Mat& frame) {
    if (not rwFlag) {
        golombo.turnaround();
        rwFlag = 1;
    }
    if (type == "4:4:4") {
        frameWrite3Channel(frame);//GGGsV8
    }
    else {
        frameWrite1Channel(frame);//GsV8
    }
}

Mat Predictor::decode() {
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


void Predictor::saveEFInfo(int h, int w, string type, int fps,int frames,int blockSearchRadius, int blockSize,string path) {
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
    golombo.changeM(8);
    golombo.ReadWrite.writeNbits(8, 8);
}

void Predictor::readEFInfo(string path) {
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
    golombo.changeM(8);
}

Mat Predictor::blockFrameRead3Channel(){
    vector<signed int> block(blockSize);
    Mat result(videoHeight,videoWidth , CV_8UC3);
    int shiftL;
    int shiftC;
    for (int linhas = 0; linhas < videoHeight; linhas+=blockSize)
    {
        for (int colunas = 0; colunas < videoWidth; colunas+=blockSize)
        {
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            //creating blocks Y U V
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][0] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][0] + golombo.decode();
                }
            }
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][1] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][1] + golombo.decode();
                }
            }

            shiftL = golombo.decode();
            shiftC = golombo.decode();
            for (int lb = 0; lb < blockSize; lb++) {
                for (int cb = 0; cb < blockSize; cb++) {
                    result.ptr<Vec3b>(linhas+lb)[colunas+cb][2] = prevFrame.ptr<Vec3b>(linhas+lb+shiftL)[colunas+cb+shiftC][2] + golombo.decode();
                }
            }

        }
    }
    return result;
}

Mat Predictor::blockFrameRead1Channel(){
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
    for (int linhas = 0; linhas < lines; linhas+=blockSize)
    {
        for (int colunas = 0; colunas < cols; colunas+=blockSize)
        {
            shiftL = golombo.decode();
            shiftC = golombo.decode();
            //creating blocks
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    result.ptr<uchar>(linhas+lb)[colunas+cb] = prevFrame.ptr<uchar>(linhas+lb+shiftL)[colunas+cb+shiftC] - golombo.decode();
                }
            }

        }
    }
    return result;
}

void Predictor::blockFrameWrite3Channel(const Mat& frame){
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
            //codificar blocos
            golombo.encode(shiftY[0]); //G
            golombo.encode(shiftY[1]); //GG
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    golombo.encode(blockLeastDiffY[lb*blockSize+cb]);
                }
            }
            golombo.encode(shiftU[0]);
            golombo.encode(shiftU[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    golombo.encode(blockLeastDiffU[lb*blockSize+cb]);
                }
            }
            golombo.encode(shiftV[0]);
            golombo.encode(shiftV[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    golombo.encode(blockLeastDiffV[lb*blockSize+cb]);
                }
            }
            total = total + sumLeastDiffY + sumLeastDiffU + sumLeastDiffV;
            sumLeastDiffY = 255*vecSize;
            sumLeastDiffU = 255*vecSize;
            sumLeastDiffV = 255*vecSize;
        }
    }
    total = total / (frame.rows * frame.cols * 3);
    changeGolombM(total);
}

void Predictor::blockFrameWrite1Channel(const Mat& frame){
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
                                currentblock[lb * blockSize + cb] = prevFrame.ptr<uchar>(linhas + lb + raioh)[colunas +
                                                                                                              cb +
                                                                                                              raiow] - frame.ptr<uchar>(linhas + lb)[colunas + cb];
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
            golombo.encode(shift[0]);
            golombo.encode(shift[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    golombo.encode(blockLeastDiff[lb*blockSize+cb]);
                }
            }
            total = total + sumLeastDiff;
            sumLeastDiff = 255*vecSize;
        }
    }
    total = total / (lines*cols);
    changeGolombM(total);
}

void Predictor::encodeEF(const Mat& frame){
    if (not rwFlag) {
        golombo.turnaround();
        rwFlag = 1;
    }
    if (type == "4:4:4") {
        blockFrameWrite3Channel(frame);
    }
    else {
        blockFrameWrite1Channel(frame);
    }
    prevFrame = frame;

}

Mat Predictor::decodeEF(){
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

void Predictor::encodeA(const Mat& frame) {
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
    prevFrame = frame;

}

Mat Predictor::decodeA(){
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

void Predictor::fakePredict3Channel(const Mat& frame){
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
            //codificar blocos
            bitsForInter += golombo.fakeEncode(shiftY[0]); //G
            bitsForInter += golombo.fakeEncode(shiftY[1]); //GG
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    bitsForInter += golombo.fakeEncode(blockLeastDiffY[lb*blockSize+cb]);
                }
            }
            bitsForInter += golombo.fakeEncode(shiftU[0]);
            bitsForInter += golombo.fakeEncode(shiftU[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    bitsForInter += golombo.fakeEncode(blockLeastDiffU[lb*blockSize+cb]);
                }
            }
            bitsForInter += golombo.fakeEncode(shiftV[0]);
            bitsForInter += golombo.fakeEncode(shiftV[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    bitsForInter += golombo.fakeEncode(blockLeastDiffV[lb*blockSize+cb]);
                }
            }
            total = total + sumLeastDiffY + sumLeastDiffU + sumLeastDiffV;
            sumLeastDiffY = 255*vecSize;
            sumLeastDiffU = 255*vecSize;
            sumLeastDiffV = 255*vecSize;
        }
    }
    for (int linhas = 0; linhas < videoHeight; linhas++){

        for (int colunas = 0; colunas < videoWidth; colunas++){
            //square = currentY(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentY.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentY.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentY.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentY.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            bitsForIntra += golombo.fakeEncode(numb);
            //square = currentU(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentU.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentU.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentU.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentU.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            bitsForIntra += golombo.fakeEncode(numb);
            //square = currentV(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentV.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentV.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentV.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentV.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            bitsForIntra += golombo.fakeEncode(numb);
        }
    }
}

void Predictor::fakePredict1Channel(const Mat& frame){
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
                                currentblock[lb * blockSize + cb] = prevFrame.ptr<uchar>(linhas + lb + raioh)[colunas +
                                                                                                              cb +
                                                                                                              raiow] - frame.ptr<uchar>(linhas + lb)[colunas + cb];
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
            bitsForInter += golombo.fakeEncode(shift[0]);
            bitsForInter += golombo.fakeEncode(shift[1]);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    bitsForInter += golombo.fakeEncode(blockLeastDiff[lb*blockSize+cb]);
                }
            }
            sumLeastDiff = 255*vecSize;
        }
    }
    for (int linhas = 0; linhas < lines; linhas++){

        for (int colunas = 0; colunas < cols; colunas++){
            //square = current(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = current.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = current.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = current.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = current.ptr<uchar>(linhas+1)[colunas+1];

            numb = calculator(square);
            bitsForIntra += golombo.fakeEncode(numb);
        }
    }
}