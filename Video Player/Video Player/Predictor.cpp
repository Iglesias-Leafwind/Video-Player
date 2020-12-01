/*TODO LIST
	Verificar predict decode
	Create tests menu in Video Player.cpp
*/


#include <iostream>
#include <stdio.h>
#include <fstream>
#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
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
			square = currentY(Rect(colunas, linhas, 2, 2));
			numb = calculator(square);
			if (numb < 0) {
				total = total + numb * (-1);
			}
			else {
				total = total + numb;
			}
			golombo.encode(numb);
			square = currentU(Rect(colunas, linhas, 2, 2));
			numb = calculator(square);
			if (numb < 0) {
				total = total + numb * (-1);
			}
			else {
				total = total + numb;
			}
			golombo.encode(numb);
			square = currentV(Rect(colunas, linhas, 2, 2));
			numb = calculator(square);
			if (numb < 0) {
				total = total + numb * (-1);
			}
			else {
				total = total + numb;
			}
			golombo.encode(numb);
		}
	}
	total = total / (frame.rows * frame.cols * 3);
	unsigned int count = (int)log2(total) + 1;
	count = (int)pow(2, count);
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
			square = current(Rect(colunas, linhas, 2, 2));
			numb = calculator(square);
			if (numb < 0) {
				total = total + numb * (-1);
			}
			else {
				total = total + numb;
			}
			golombo.encode(numb);
		}
	}
	total = total / (lines * cols);
	unsigned int count = (int)log2(total) + 1;
	count = (int)pow(2, count);
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

int test() {
	Predictor predict8(8);

	//String videoPath = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\UnusAnnus.mp4";
	//String videoPath = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\ducks_take_off_420_720p50.y4m";
	String videoPath = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\park_joy_444_720p50.y4m";
	// Create a VideoCapture object and open the input file
	VideoCapture inputVideo(videoPath);
	// Check if camera opened successfully
	if (!inputVideo.isOpened())
	{
		cout << "Could not open the input video: " << videoPath << endl;
	}
	else {
		int fps = inputVideo.get(CAP_PROP_FPS);
		int frame_width = inputVideo.get(CAP_PROP_FRAME_WIDTH);
		int frame_height = inputVideo.get(CAP_PROP_FRAME_HEIGHT);
		int frames = inputVideo.get(CAP_PROP_FRAME_COUNT);
		int delay = 1000 / fps;
		// When everything done, release the video capture object
		Mat frame;
		int numframes = 0;
		predict8.saveInfo(frame_height, frame_width, "4:2:2", fps,frames, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict8.bin");
		cout << predict8.videoWidth << endl;
		cout << predict8.videoHeight << endl;
		cout << predict8.type << endl;
		cout << predict8.videoFPS << endl;
		predict8.readInfo("D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict8.bin");
		cout << predict8.videoWidth << endl;
		cout << predict8.videoHeight << endl;
		cout << predict8.type << endl;
		cout << predict8.videoFPS << endl;
		while (1) {
			// Capture frame-by-frame
			inputVideo >> frame;
			// If the frame is empty, break immediately
			if (frame.empty())
				break;
			cout << ++numframes << " - ";
			predict8.encode(frame);
			// Press  ESC on keyboard to exit
			char c = (char)waitKey(25);
			if (c == 27)
				break;
			if (waitKey(delay)) {
				continue;
			}
		}
		inputVideo.release();
		// Closes all the frames
		void destroyAllWindows();
	}
		/*
	Predictor predict1(1, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict1.bin");
	Predictor predict2(2, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict2.bin");
	Predictor predict3(3, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict3.bin");
	Predictor predict4(4, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict4.bin");
	Predictor predict5(5, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict5.bin");
	Predictor predict6(6, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict6.bin");
	Predictor predict7(7, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict7.bin");
	Predictor predict8(8, "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\predict8.bin");
	predict2.encode(img);
	cout << "DONE2" << endl;
	predict3.encode(img);
	cout << "DONE3" << endl;
	predict4.encode(img);
	cout << "DONE4" << endl;
	predict5.encode(img);
	cout << "DONE5" << endl;
	predict6.encode(img);
	cout << "DONE6" << endl;
	predict7.encode(img);
	cout << "DONE7" << endl;
	predict8.encode(img);
	cout << "DONE8" << endl;*//*
	predict1.encode(img);
	cout << "DONE" << endl;
	Mat frame = predict1.decode();
	cout << "li" << endl;
	for(int i = 0; i < 10; i++){
		predict1.encode(frame);
		cout << "DONE" << i << endl;
		frame = predict1.decode();
	}
	predict1.encode(frame);
	imshow("Img", predict1.decode());
	waitKey(0);*/
	return 0;
}
/*int imageMain(){

	Predictor predict1(1);
	Predictor predict2(2);
	Predictor predict3(3);
	Predictor predict4(4);
	Predictor predict5(5);
	Predictor predict6(6);
	Predictor predict7(7);
	Predictor predict8(8);
	String videoPath = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\UnusAnnus.mp4";
	//String videoPath = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\ducks_take_off_420_720p50.y4m";
	// Create a VideoCapture object and open the input file
    VideoCapture inputVideo(videoPath);
    // Check if camera opened successfully
    if (!inputVideo.isOpened())
    {
		cout << "Could not open the input video: " << videoPath << endl;
    }
    else {
		int fps = inputVideo.get(CAP_PROP_FPS);
        int delay = 1000 / fps;
        // When everything done, release the video capture object
		Mat frame;
		int numframes = 0;
		while (1) {
			// Capture frame-by-frame
			inputVideo >> frame;
			// If the frame is empty, break immediately
			if (frame.empty())
				break;
			cout << ++numframes << endl;
			vector<signed int> lmao1 = predict1.encode(frame);
			vector<signed int> lmao2 = predict2.encode(frame);
			vector<signed int> lmao3 = predict3.encode(frame);
			vector<signed int> lmao4 = predict4.encode(frame);
			vector<signed int> lmao5 = predict5.encode(frame);
			vector<signed int> lmao6 = predict6.encode(frame);
			vector<signed int> lmao7 = predict7.encode(frame);
			vector<signed int> lmao8 = predict8.encode(frame);
			int pos = 2;
			cout << lmao1.at(0) << " / " << lmao1.at(1) << endl;
			cout << lmao1.at(pos) << " " << lmao1.at(pos+3) << " " << lmao1.at(pos+6) << " " << lmao1.at(pos+9) << " " << lmao1.at(pos+12) << endl;
			cout << lmao1.at(pos + 1) << " " << lmao1.at(pos + 4) << " " << lmao1.at(pos + 7) << " " << lmao1.at(pos + 10) << " " << lmao1.at(pos + 13) << endl;
			cout << lmao1.at(pos + 2) << " " << lmao1.at(pos + 5) << " " << lmao1.at(pos + 8) << " " << lmao1.at(pos + 11) << " " << lmao1.at(pos + 14) << endl;
			cout << endl;
			cout << lmao2.at(pos) << " " << lmao2.at(pos + 3) << " " << lmao2.at(pos + 6) << " " << lmao2.at(pos + 9) << " " << lmao2.at(pos + 12) << endl;
			cout << lmao2.at(pos + 1) << " " << lmao2.at(pos + 4) << " " << lmao2.at(pos + 7) << " " << lmao2.at(pos + 10) << " " << lmao2.at(pos + 13) << endl;
			cout << lmao2.at(pos + 2) << " " << lmao2.at(pos + 5) << " " << lmao2.at(pos + 8) << " " << lmao2.at(pos + 11) << " " << lmao2.at(pos + 14) << endl;
			cout << endl;
			cout << lmao3.at(pos) << " " << lmao3.at(pos + 3) << " " << lmao3.at(pos + 6) << " " << lmao3.at(pos + 9) << " " << lmao3.at(pos + 12) << endl;
			cout << lmao3.at(pos + 1) << " " << lmao3.at(pos + 4) << " " << lmao3.at(pos + 7) << " " << lmao3.at(pos + 10) << " " << lmao3.at(pos + 13) << endl;
			cout << lmao3.at(pos + 2) << " " << lmao3.at(pos + 5) << " " << lmao3.at(pos + 8) << " " << lmao3.at(pos + 11) << " " << lmao3.at(pos + 14) << endl;
			cout << endl;
			cout << lmao4.at(pos) << " " << lmao4.at(pos + 3) << " " << lmao4.at(pos + 6) << " " << lmao4.at(pos + 9) << " " << lmao4.at(pos + 12) << endl;
			cout << lmao4.at(pos + 1) << " " << lmao4.at(pos + 4) << " " << lmao4.at(pos + 7) << " " << lmao4.at(pos + 10) << " " << lmao4.at(pos + 13) << endl;
			cout << lmao4.at(pos + 2) << " " << lmao4.at(pos + 5) << " " << lmao4.at(pos + 8) << " " << lmao4.at(pos + 11) << " " << lmao4.at(pos + 14) << endl;
			cout << endl;
			cout << lmao5.at(pos) << " " << lmao5.at(pos + 3) << " " << lmao5.at(pos + 6) << " " << lmao5.at(pos + 9) << " " << lmao5.at(pos + 12) << endl;
			cout << lmao5.at(pos + 1) << " " << lmao5.at(pos + 4) << " " << lmao5.at(pos + 7) << " " << lmao5.at(pos + 10) << " " << lmao5.at(pos + 13) << endl;
			cout << lmao5.at(pos + 2) << " " << lmao5.at(pos + 5) << " " << lmao5.at(pos + 8) << " " << lmao5.at(pos + 11) << " " << lmao5.at(pos + 14) << endl;
			cout << endl;
			cout << lmao6.at(pos) << " " << lmao6.at(pos + 3) << " " << lmao6.at(pos + 6) << " " << lmao6.at(pos + 9) << " " << lmao6.at(pos + 12) << endl;
			cout << lmao6.at(pos + 1) << " " << lmao6.at(pos + 4) << " " << lmao6.at(pos + 7) << " " << lmao6.at(pos + 10) << " " << lmao6.at(pos + 13) << endl;
			cout << lmao6.at(pos + 2) << " " << lmao6.at(pos + 5) << " " << lmao6.at(pos + 8) << " " << lmao6.at(pos + 11) << " " << lmao6.at(pos + 14) << endl;
			cout << endl;
			cout << lmao7.at(pos) << " " << lmao7.at(pos + 3) << " " << lmao7.at(pos + 6) << " " << lmao7.at(pos + 9) << " " << lmao7.at(pos + 12) << endl;
			cout << lmao7.at(pos + 1) << " " << lmao7.at(pos + 4) << " " << lmao7.at(pos + 7) << " " << lmao7.at(pos + 10) << " " << lmao7.at(pos + 13) << endl;
			cout << lmao7.at(pos + 2) << " " << lmao7.at(pos + 5) << " " << lmao7.at(pos + 8) << " " << lmao7.at(pos + 11) << " " << lmao7.at(pos + 14) << endl;
			cout << endl;
			cout << lmao8.at(pos) << " " << lmao8.at(pos + 3) << " " << lmao8.at(pos + 6) << " " << lmao8.at(pos + 9) << " " << lmao8.at(pos + 12) << endl;
			cout << lmao8.at(pos + 1) << " " << lmao8.at(pos + 4) << " " << lmao8.at(pos + 7) << " " << lmao8.at(pos + 10) << " " << lmao8.at(pos + 13) << endl;
			cout << lmao8.at(pos + 2) << " " << lmao8.at(pos + 5) << " " << lmao8.at(pos + 8) << " " << lmao8.at(pos + 11) << " " << lmao8.at(pos + 14) << endl;
			cout << endl;
			Mat newFrame = predict1.decode(lmao1);
			//bool isEqual = (sum(frame != newFrame) == Scalar(0, 0, 0, 0));
			//cout << isEqual << endl;
			imshow("FramePredict1", newFrame);
			imshow("FramePredict2", predict2.decode(lmao2));
			imshow("FramePredict3", predict3.decode(lmao3));
			imshow("FramePredict4", predict4.decode(lmao4));
			imshow("FramePredict5", predict5.decode(lmao5));
			imshow("FramePredict6", predict6.decode(lmao6));
			imshow("FramePredict7", predict7.decode(lmao7));
			imshow("FramePredict8", predict8.decode(lmao8));
			// Press  ESC on keyboard to exit
			char c = (char)waitKey(25);
			if (c == 27)
				break;
			if (waitKey(delay)) {
				continue;
			}
		}
        inputVideo.release();
        // Closes all the frames
        void destroyAllWindows();
    }
}*/