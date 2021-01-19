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

    this->luminance = Mat(8,8, CV_64FC1);

    for(int l = 0; l<8;l++){
        for(int c = 0; c<8;c++){
            luminance.ptr<double>(l)[c] = dataLuminance[l][c] * 1.0;
        };
    };

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

Mat jpegPredictor::frameRead1Channel() {
    vector<signed int> tubytu(4);
    int lines = videoHeight;
    int cols = videoWidth;
    lines = lines * 3 / 2;
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

void jpegPredictor::saveEFInfo(int h, int w, int fps,int frames,int blockSearchRadius,int nbits,int answer,string path) {
    golombo = Golomb(path, 10, "w");
    golombo.encode(this->mode);
    this->videoHeight = h;
    golombo.encode(h);
    this->videoWidth = w;
    golombo.encode(w);
    this->videoFrames = frames;
    golombo.encode(frames);
    this->videoFPS = fps;
    golombo.encode(fps);
    this->blockSearchRadius = blockSearchRadius;
    golombo.encode(blockSearchRadius);
    this->blockSize = 8;
    this->nbits = nbits;
    golombo.encode(nbits);
    this->quantProblemFlag = answer;
    golombo.ReadWrite.writebit(answer);
    changeGolombM(8);
}

void jpegPredictor::readEFInfo(string path) {
    golombo = Golomb(path, 10, "r");
    this->mode = golombo.decode();
    this->videoHeight = golombo.decode();
    this->videoWidth = golombo.decode();
    this->videoFrames = golombo.decode();
    this->videoFPS = golombo.decode();
    this->blockSearchRadius = golombo.decode();
    this->blockSize = 8;
    this->nbits = golombo.decode();
    this->quantProblemFlag = golombo.ReadWrite.readbit();
}

Mat jpegPredictor::blockFrameRead1Channel(){
    int lines = videoHeight;
    int cols = videoWidth;
    lines = lines * 3 / 2;
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
                    block.ptr<double>(lb)[cb] = golombo.decode();
                }
            }
            if(quantProblemFlag){
                multiply(block,luminance,block);
            }
            idct(block,block);
            add(block,128.0,block);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<double>(lb)[cb];
                    numb = numb << nbits;
                    unsigned char prevNumb = prevFrame.ptr<uchar>(linhas+lb+shiftL)[colunas+cb+shiftC];
                    result.ptr<uchar>(linhas+lb)[colunas+cb] = prevNumb - numb;
                }
            }


        }
    }
    return result;
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
                    block.ptr<double>(lb)[cb] = numb;
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
            if(quantProblemFlag){
                divide( block, luminance, block );
            }
            block.convertTo(block,CV_16SC1);
            block.convertTo(block,CV_64FC1);
            if(quantProblemFlag){
                multiply(block,luminance,returning);
            }else{
                block.copyTo(returning);
            }
            idct(returning,returning);
            add(returning,128.0,returning);
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<double>(lb)[cb];
                    bpp += golombo.fakeEncode(numb);
                    golombo.encode(numb);
                    numb = returning.ptr<double>(lb)[cb];
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

void jpegPredictor::encodeA(const Mat& frame) {
    if(prevFrameF){
        fakePredict1Channel(frame);
        if(bitsForInter <= bitsForIntra){
            golombo.ReadWrite.writebit(1);
            prevFrame = blockFrameWrite1Channel(frame);
        }else{
            golombo.ReadWrite.writebit(0);
            prevFrame = frameWrite1Channel(frame);
        }
    }else{
        golombo.ReadWrite.writebit(0);
        prevFrame = frameWrite1Channel(frame);
        prevFrameF = true;
    }
}

Mat jpegPredictor::decodeA(){
    Mat returning;
    int count = golombo.ReadWrite.readNbits(8);
    golombo.changeM(count);
    if(prevFrameF){
        if(golombo.ReadWrite.readbit()){
            returning = blockFrameRead1Channel();
        }else{
            returning = frameRead1Channel();;
        }
    }else{
        golombo.ReadWrite.readbit();
        prevFrameF = true;
        returning = frameRead1Channel();;
    }
    prevFrame = returning;
    return returning;
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
                    block.ptr<double>(lb)[cb] = numb;
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
            if(quantProblemFlag){
                divide( block, luminance, block );
            }
            for (int lb = 0; lb < blockSize; ++lb) {
                for (int cb = 0; cb < blockSize; ++cb) {
                    numb = block.ptr<double>(lb)[cb];
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