#include <iostream>
#include "opencv2/opencv.hpp"
#include "Predictor.h"

using namespace cv;
using namespace std;

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

signed int Predictor::calculator(Mat numb) {
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

Mat Predictor::fillZeros(Mat channel) {
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
}

void Predictor::frameWrite3Channel(Mat frame) {
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
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }
			golombo.encode(numb);
			//square = currentU(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentU.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentU.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentU.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentU.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            if (numb < 0) {
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }
			golombo.encode(numb);
			//square = currentV(Rect(colunas, linhas, 2, 2));
            square.ptr<uchar>(0)[0] = currentV.ptr<uchar>(linhas)[colunas];
            square.ptr<uchar>(0)[1] = currentV.ptr<uchar>(linhas)[colunas+1];
            square.ptr<uchar>(1)[0] = currentV.ptr<uchar>(linhas+1)[colunas];
            square.ptr<uchar>(1)[1] = currentV.ptr<uchar>(linhas+1)[colunas+1];
            numb = calculator(square);
            if (numb < 0) {
                total = total + (numb * (-2) -1);
            }
            else {
                total = total + numb * 2;
            }
			golombo.encode(numb);
		}
	}
	total = total / (frame.rows * frame.cols * 3);
	unsigned int count = (int)log2(total) + 1;
	//count = (int)pow(2, count);
	cout << count << endl;
	if (count != golombo.m) {
		golombo.changeM(count);
	}
	golombo.ReadWrite.writeNbits(count, 8);
}

void Predictor::frameWrite1Channel(Mat frame) {
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
			if (numb < 0) {
				total = total + (numb * (-2) -1);
			}
			else {
				total = total + numb * 2;
			}
			golombo.encode(numb);
		}
	}
	total = total / (lines * cols);
	unsigned int count = (int)log2(total) + 1;
	//count = (int)pow(2, count);
    cout << count << endl;
	if (count != golombo.m) {
		golombo.changeM(count);
	}
	golombo.ReadWrite.writeNbits(count, 8);
}

Mat Predictor::frameRead3Channel() {
	vector<signed int> tubytu(4);
	Mat result(videoHeight,videoWidth , CV_8UC3);
	int count = golombo.ReadWrite.readNbits(8);
	golombo.changeM(count);
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
	}
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
	int count = golombo.ReadWrite.readNbits(8);
	golombo.changeM(count);
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

void Predictor::encode(Mat frame) {
	if (not rwFlag) {
		golombo.turnaround();
		rwFlag = 1;
	}
	if (type == "4:4:4") {
		frameWrite3Channel(frame);
	}
	else {
		frameWrite1Channel(frame);
	}
}

Mat Predictor::decode() {
	if (rwFlag) {
		golombo.turnaround();
		rwFlag = 0;
	}
	if (type == "4:4:4") {
		return frameRead3Channel();
	}
	return frameRead1Channel();
}