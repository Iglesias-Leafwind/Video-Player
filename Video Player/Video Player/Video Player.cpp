/*! \File Video Player.cpp
 *	\This is a Video player
 *	File Video Player.cpp is a program to show and edit video files.
 */

#include "opencv2/opencv.hpp"
#include <opencv2/videoio.hpp>
#include "Predictor.h"
#include <iostream>
#include <stdio.h>
#include <time.h>

using namespace cv;
using namespace std;

/*! \var string videoPath
 *
 *  This contains the current loaded video path
 */
String videoPath = "Null";
/*! \var time_t startTime
*   Starting time;
*/
time_t startTime;
/*! \var time_t endTime
*   Starting time;
*/
time_t endTime;

/*! \var bool watermarkFlag 
 *
 *  This is a flag which tells us if we have watermark or not.
 *  Default is false.
 */
bool watermarkFlag = false;

/*! \var string watermarkContent
 *
 *  This contains the imagefile / text that will be used as
 *  a watermark.
 */
String watermarkContent;

/*! \var bool colorspacesFlag
 *
 *  This is a flag which tells us if we have colorspaces or not.
 *  Default is false.
 */
bool colorspacesFlag = false;

/*! \var bool histogramsFlag
 *
 *  This is a flag which tells us if we want histograms or not.
 *  Default is false.
 */
bool histogramsFlag = false;

/*! \var bool grayscaleFlag
 *
 *  This is a flag which tells us if we want to grayscale 
 *  and apply histogram equalization or not.
 *  Default is false.
 */
bool grayscaleFlag = false;

/*! \var bool gbkfiltersFlag
 *
 *  This is a flag which tells us if we have gaussian 
 *  or blur or kernels filters.
 *  Default is false.
 */
bool gbkfiltersFlag = false;

/*! \var bool segmentationalgorithmFlag
 *
 *  This is a flag which tells us if we are using 
 *  segmentation algorithms or not.
 *  Default is false.
 */
bool segmentationalgorithmFlag = false;

/*! \var bool morphologicaloperatorsFlag
 *
 *  This is a flag which tells us if we have morphological operators or not.
 *  Default is false.
 */
bool morphologicaloperatorsFlag = false;

/*! \var bool gradientsFlag
 *
 *  This is a flag which tells us if we want to check image gradients.
 *  Default is false.
 */
bool gradientsFlag = false;

/*! \var bool YUVBGRFlag
 *
 *  This is a flag that tells us if we want to edit with our own yuv bgr
 */
bool YUVBGRFlag = false;

/*! \var bool cannyedgeFlag
 *
 *  This is a flag which tells us if we want to apply
 *  canny edge detection algorthm.
 *  Default is false.
 */
bool cannyedgeFlag = false;

/*! \var int alphaSliderMax
 *
 *  This is sets the trackbar max value
 */
const int alphaSliderMax = 100;

/*! \var int alphaSlider
 *  This is the value inside the slider
 */
int alphaSlider;

/*! \var double alpha
 * Is one of the values that will be used on the watermark
 */
double alpha;

/*! \var double beta
 * Is one of the values that will be used on the watermark
 */
double beta;

/*! \var Mat oldFrame,edit,frame
 *  initializing variables just easier future use
 */
Mat oldFrame,edit, frame;

/*! \function alpha_trackbar
* this tracks the watermark intensity
*/
static void alpha_trackbar(int, void*)
{
    alpha = (double)alphaSlider / alphaSliderMax;
    beta = (1.0 - alpha);
    addWeighted(oldFrame, alpha, edit, beta, 0.0, frame);
}

float xCoord, yCoord;
int xSlider, ySlider;
int xSliderMax, ySliderMax;
/*! \function xCoord_trackbar
* this tracks the watermark intensity
*/
static void xCoord_trackbar(int, void*)
{
    xCoord = (float)xSlider;
}

static void ycoord_trackbar(int, void*)
{
    yCoord = (float)ySlider;
}

int selection;
int selectionSlider;
int selectionSliderMax;
static void selection_trackbar(int, void*)
{
    selection = selectionSlider;
}

int general;
int generalSlider;
int generalSliderMax;
static void general_trackbar(int, void*)
{
    general = generalSlider;
}

int general2;
int general2Slider;
int general2SliderMax;
static void general2_trackbar(int, void*)
{
    general2 = general2Slider;
}

int general3;
int general3Slider;
int general3SliderMax;
static void general3_trackbar(int, void*)
{
    general3 = general3Slider;
}

void resetTrackbars() {
    yCoord = 0;
    xCoord = 0;
    selection = 0;
    general = 0;
    general2 = 0;
    general3 = 0;
}
/*! \function customBGR2YUV
*   
*   this function accepts a image and the mode and converts
*   the image from BGR to YUV with the following modes
*   4:4:4 , 4:2:2 , 4:2:0
* 
*   Thanks to 
*            Wei 
*               for helping me out with this function
*/
Mat customBGR2YUV(Mat BGR, String mode = "4:4:4") {
    if (mode == "4:2:2") {

        Mat result(BGR.rows * 2, BGR.cols, CV_8UC1);

        int u_row = BGR.rows;
        int u_col = 0;
        int v_row = BGR.rows*3/2;
        int v_col = 0;

        for (int i = 0; i < BGR.rows; i++)
        {
            if (u_col == BGR.cols) {
                u_row++;
                u_col = 0;
            }
            if (v_col == BGR.cols) {
                v_row++;
                v_col = 0;
            }

            for (int j = 0; j < BGR.cols; j++)
            {
                unsigned char B = BGR.ptr<Vec3b>(i)[j][0];
                unsigned char G = BGR.ptr<Vec3b>(i)[j][1];
                unsigned char R = BGR.ptr<Vec3b>(i)[j][2];

                unsigned char Y = 0.299 * R + 0.587 * G + 0.114 * B;
                unsigned char U = 128 - 0.168736 * R - 0.331264 * G + 0.5 * B;
                unsigned char V = 128 + 0.5 * R - 0.418688 * G - 0.081312 * B;

                result.ptr<uchar>(i)[j] = Y;
                if (j % 2 == 0) {
                    result.ptr<uchar>(u_row)[u_col++] = U;
                    result.ptr<uchar>(v_row)[v_col++] = V;
                }
            }
        }
        return result;
    }
    else if (mode == "4:2:0") {

        Mat result(BGR.rows * 3 / 2, BGR.cols, CV_8UC1);

        int u_row = BGR.rows;
        int u_col = 0;
        int v_row = BGR.rows*5/4;
        int v_col = 0;
        for (int i = 0; i < BGR.rows; i++)
        {
            if (u_col == BGR.cols) {
                u_row++;
                u_col = 0;
            }
            if (v_col == BGR.cols) {
                v_row++;
                v_col = 0;
            }

            for (int j = 0; j < BGR.cols; j++)
            {
                unsigned char B = BGR.ptr<Vec3b>(i)[j][0];
                unsigned char G = BGR.ptr<Vec3b>(i)[j][1];
                unsigned char R = BGR.ptr<Vec3b>(i)[j][2];

                unsigned char Y = 0.299 * R + 0.587 * G + 0.114 * B;
                unsigned char U = 128 - 0.168736 * R - 0.331264 * G + 0.5 * B;
                unsigned char V = 128 + 0.5 * R - 0.418688 * G - 0.081312 * B;

                result.ptr<uchar>(i)[j] = Y;
                if (i % 2 == 0 && j % 2 == 0) {
                    result.ptr<uchar>(u_row)[u_col++] = U;
                    result.ptr<uchar>(v_row)[v_col++] = V;
                }
            }
        }
        return result;
    }
    else {

        Mat ymat(BGR.rows, BGR.cols, CV_8UC1);
        Mat umat(BGR.rows, BGR.cols, CV_8UC1);
        Mat vmat(BGR.rows, BGR.cols, CV_8UC1);

        for (int i = 0; i < BGR.rows; i++)
        {
            for (int j = 0; j < BGR.cols; j++)
            {  
                unsigned char B = BGR.ptr<Vec3b>(i)[j][0];
                unsigned char G = BGR.ptr<Vec3b>(i)[j][1];
                unsigned char R = BGR.ptr<Vec3b>(i)[j][2];

                unsigned char Y = 0.299 * R + 0.587 * G + 0.114 * B;
                unsigned char U = 128 - 0.168736 * R - 0.331264 * G + 0.5 * B;
                unsigned char V = 128 + 0.5 * R - 0.418688 * G - 0.081312 * B;

                ymat.ptr<uchar>(i)[j] = Y;
                umat.ptr<uchar>(i)[j] = U;
                vmat.ptr<uchar>(i)[j] = V;
            }
        }
        Mat result;
        std::vector<Mat> channels{ ymat, umat, vmat };

        // Create the output matrix
        merge(channels, result);
        return result;
    }

}


/*! \function customYUV2BGR
*
*   this function accepts a image and the mode and converts
*   the image from YUV to BGR from the following modes
*   4:4:4 , 4:2:2 , 4:2:0
* 
*   Thanks to 
*            Wei 
*               for helping me out with this function
*/
Mat customYUV2BGR(Mat YUV, String mode = "4:4:4") {
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

/*! \function showVideo
*   
*   this function accepts a video input and shows
*   the video with the proper edit made in
*   the menu
*/
int showVideo(VideoCapture inputVideo,int delay) {
    //delete this after testing
    //D:\Desktop\Universidade\CSLP\projeto\videos\ducks_take_off_420_720p50.y4m
    //D:\Desktop\Universidade\CSLP\projeto\videos\UnusAnnus.mp4
    bool waterstring = false;
    Mat watermark;
    //checks if watermark is active
    if (watermarkFlag) {
        //checks if watermark is a file
        watermark = imread(watermarkContent);
        if (watermark.empty()) {
            waterstring = true;
        }
    }

    while (1) {
        // Capture frame-by-frame
        inputVideo >> frame;
        // If the frame is empty, break immediately
        if (frame.empty())
            break;
        //Create window
        namedWindow("Frame", WINDOW_AUTOSIZE);
        //Apply watermark
        if (watermarkFlag) {
            if (waterstring) {
                char TrackbarX[50];
                char TrackbarY[50];
                char TrackbarName[50];
                xSliderMax = frame.cols;
                ySliderMax = frame.rows;
                sprintf_s(TrackbarName, "Alpha x %d", alphaSliderMax);
                sprintf_s(TrackbarX, "X_coord x %d", xSliderMax);
                sprintf_s(TrackbarY, "Y_coord x %d", ySliderMax);
                edit = frame.clone();
                oldFrame = frame.clone();
                frame.release();
                createTrackbar(TrackbarX, "Frame", &xSlider, xSliderMax, xCoord_trackbar);
                xCoord_trackbar(xSlider, 0);
                createTrackbar(TrackbarY, "Frame", &ySlider, ySliderMax, ycoord_trackbar);
                ycoord_trackbar(ySlider, 0);
                putText(edit, watermarkContent, Point2f(xCoord, yCoord), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255, 255));
                createTrackbar(TrackbarName, "Frame", &alphaSlider, alphaSliderMax, alpha_trackbar);
                alpha_trackbar(alphaSlider, 0);
             }
            else {
                char TrackbarName[50];
                sprintf_s(TrackbarName, "Alpha x %d", alphaSliderMax);
                resize(watermark, edit, frame.size());
                oldFrame = frame.clone();
                frame.release();
                createTrackbar(TrackbarName, "Frame", &alphaSlider, alphaSliderMax, alpha_trackbar);
                alpha_trackbar(alphaSlider, 0);
            }
        }
        //Apply color spaces and channels
        else if (colorspacesFlag) {
            Mat colorChannel[3];
            split(frame, colorChannel);

            namedWindow("Red", WINDOW_AUTOSIZE);
            imshow("Red", colorChannel[2]);

            namedWindow("Green", WINDOW_AUTOSIZE);
            imshow("Green", colorChannel[1]);

            namedWindow("Blue", WINDOW_AUTOSIZE);
            imshow("Blue", colorChannel[0]);

            char TrackbarName[50];
            selectionSliderMax = 1;
            sprintf_s(TrackbarName, "YUV = 0 or HSV = 1 x %d", selectionSliderMax);
            oldFrame = frame.clone();
            frame.release();
            createTrackbar(TrackbarName, "Frame", &selectionSlider, selectionSliderMax, selection_trackbar);
            selection_trackbar(selectionSlider, 0);
            
            if (selection == 0) {
                cvtColor(oldFrame, frame, COLOR_BGR2YUV);
            }
            else {
                cvtColor(oldFrame, frame, COLOR_BGR2HSV);
            }
        }
        //Apply histograms
        else if (histogramsFlag) {
            //separate into r g b planes
            vector<Mat> bgr_planes;
            split(frame, bgr_planes);
            //we know the values interval
            int histSize = 256;
            //setting range values
            float range[] = { 0, 256 }; //the upper boundary is exclusive
            const float* histRange = { range };
            //setting bins to have same size (uniform) and clear the histogram
            bool uniform = true, accumulate = false;
            //calculate the histograms using function calchist
            Mat b_hist, g_hist, r_hist;
            calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
            calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
            calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);
            //creating an image to display all 3 histograms
            int hist_w = 512, hist_h = 400;
            int bin_w = cvRound((double)hist_w / histSize);
            Mat histImageB(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
            Mat histImageG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
            Mat histImageR(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
            //all
            Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
            //we normalize so that each value fall in the range indicated by the parameters entered
            normalize(b_hist, b_hist, 0, histImageB.rows, NORM_MINMAX, -1, Mat());
            normalize(g_hist, g_hist, 0, histImageG.rows, NORM_MINMAX, -1, Mat());
            normalize(r_hist, r_hist, 0, histImageR.rows, NORM_MINMAX, -1, Mat());
            //all
            normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
            normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
            normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
            //used to access bin (in this case in this 1D-histogram) ???
            for (int i = 1; i < histSize; i++)
            {
                line(histImageB, Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
                    Scalar(255, 0, 0), 2, 8, 0);
                line(histImageG, Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
                    Scalar(0, 255, 0), 2, 8, 0);
                line(histImageR, Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
                    Scalar(0, 0, 255), 2, 8, 0);
                //all
                line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
                    Scalar(255, 0, 0), 2, 8, 0);
                line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
                    Scalar(0, 255, 0), 2, 8, 0);
                line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
                    Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
                    Scalar(0, 0, 255), 2, 8, 0);
            }
            imshow("Blue histogram", histImageB);
            imshow("Green histogram", histImageG);
            imshow("Red histogram", histImageR);
            //all
            imshow("ALL histogram", histImage);
        }
        //Apply grayscale and histogram equalization
        else if (grayscaleFlag) {
            //convert to grayscale
            cvtColor(frame, frame, COLOR_BGR2GRAY);
            //apply equalization
            equalizeHist(frame, frame);
        }
        //Apply blur filters
        else if (gbkfiltersFlag) {
            char TrackbarSelection[50];
            selectionSliderMax = 4;
            sprintf_s(TrackbarSelection, "blur,gaussian,median,bilateral,kernel x %d", selectionSliderMax);
            oldFrame = frame.clone();
            frame.release();
            createTrackbar(TrackbarSelection, "Frame", &selectionSlider, selectionSliderMax, selection_trackbar);
            selection_trackbar(selectionSlider, 0);
            if (selection == 0) {
                char TrackbarKernel[50];
                generalSliderMax = 31;
                sprintf_s(TrackbarKernel, "Kernel x %d", generalSliderMax);
                createTrackbar(TrackbarKernel, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);

                blur(oldFrame, frame, Size(general+1,general+1));
            }
            else if (selection == 1) {
                char TrackbarKernel[50];
                generalSliderMax = 31;
                sprintf_s(TrackbarKernel, "Kernel x %d", generalSliderMax);
                createTrackbar(TrackbarKernel, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                if (general % 2 == 0 or general == 0) {
                    general += 1;
                }
                GaussianBlur(oldFrame, frame, Size(general, general), 0, 0);
            }
            else if (selection == 2) {
                char TrackbarKernel[50];
                generalSliderMax = 31;
                sprintf_s(TrackbarKernel, "Kernel x %d", generalSliderMax);
                createTrackbar(TrackbarKernel, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                if (general % 2 == 0 or general == 0) {
                    general += 1;
                }
                medianBlur(oldFrame, frame, general);
            }
            else if (selection == 3) {
                char TrackbarKernel[50];
                generalSliderMax = 31;
                sprintf_s(TrackbarKernel, "Kernel x %d", generalSliderMax);
                createTrackbar(TrackbarKernel, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                general += 1;
                bilateralFilter(oldFrame, frame, general, general * 2, general / 2);
            }else {
                char TrackbarKernel[50];
                generalSliderMax = 31;
                sprintf_s(TrackbarKernel, "Kernel x %d", generalSliderMax);
                createTrackbar(TrackbarKernel, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                general += 1;
                Mat kernelMatrix = Mat::ones(general, general, CV_32F) / (float)(general * general);
                filter2D(oldFrame, frame, -1, kernelMatrix, Point(-1, -1), 0, BORDER_DEFAULT);
            }
        }
        //Apply watershed segmented algorithm
        else if (segmentationalgorithmFlag) {
            // Change the background from white to black, since that will help later to extract
            // better results during the use of Distance Transform
            for (int i = 0; i < frame.rows; i++) {
                for (int j = 0; j < frame.cols; j++) {
                    if (frame.at<Vec3b>(i, j) == Vec3b(255, 255, 255))
                    {
                        frame.at<Vec3b>(i, j)[0] = 0;
                        frame.at<Vec3b>(i, j)[1] = 0;
                        frame.at<Vec3b>(i, j)[2] = 0;
                    }
                }
            }
            // Create a kernel that we will use to sharpen our image
            Mat kernel = (Mat_<float>(3, 3) <<
                1, 1, 1,
                1, -8, 1,
                1, 1, 1);
            // an approximation of second derivative, a quite strong kernel
            // do the laplacian filtering as it is
            // well, we need to convert everything in something more deeper then CV_8U
                // CV_8U is unsigned 8bit/pixel - a pixel can have values 0-255, this is the normal range for most image and video formats.
            // because the kernel has some negative values,
            // and we can expect in general to have a Laplacian image with negative values
            // BUT a 8bits unsigned int (the one we are working with) can contain values from 0 to 255
            // so the possible negative number will be truncated
            Mat imgLaplacian;
            filter2D(frame, imgLaplacian, CV_32F, kernel);
            Mat sharp;
            frame.convertTo(sharp, CV_32F);
            Mat imgResult = sharp - imgLaplacian;
            // convert back to 8bits gray scale
            imgResult.convertTo(imgResult, CV_8UC3);
            imgLaplacian.convertTo(imgLaplacian, CV_8UC3);
            // Create binary image from source image
            Mat bw;
            cvtColor(imgResult, bw, COLOR_BGR2GRAY);
            threshold(bw, bw, 40, 255, THRESH_BINARY | THRESH_OTSU);
            // Perform the distance transform algorithm
            Mat dist;
            distanceTransform(bw, dist, DIST_L2, 3);
            // Normalize the distance image for range = {0.0, 1.0}
            // so we can visualize and threshold it
            normalize(dist, dist, 0, 1.0, NORM_MINMAX);
            // Threshold to obtain the peaks
            // This will be the markers for the foreground objects
            threshold(dist, dist, 0.4, 1.0, THRESH_BINARY);
            // Dilate a bit the dist image
            Mat kernel1 = Mat::ones(3, 3, CV_8U);
            dilate(dist, dist, kernel1);
            // Create the CV_8U version of the distance image
            // It is needed for findContours()
            Mat dist_8u;
            dist.convertTo(dist_8u, CV_8U);
            // Find total markers
            vector<vector<Point> > contours;
            findContours(dist_8u, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            // Create the marker image for the watershed algorithm
            Mat markers = Mat::zeros(dist.size(), CV_32S);
            // Draw the foreground markers
            for (size_t i = 0; i < contours.size(); i++)
            {
                drawContours(markers, contours, static_cast<int>(i), Scalar(static_cast<int>(i) + 1), -1);
            }
            // Draw the background marker
            circle(markers, Point(5, 5), 3, Scalar(255), -1);
            // Perform the watershed algorithm
            watershed(imgResult, markers);
            Mat mark;
            markers.convertTo(mark, CV_8U);
            bitwise_not(mark, mark);
            // image looks like at that point
            // Generate random colors
            vector<Vec3b> colors;
            for (size_t i = 0; i < contours.size(); i++)
            {
                int b = theRNG().uniform(0, 256);
                int g = theRNG().uniform(0, 256);
                int r = theRNG().uniform(0, 256);
                colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
            }
            // Create the result image
            Mat dst = Mat::zeros(markers.size(), CV_8UC3);
            // Fill labeled objects with random colors
            for (int i = 0; i < markers.rows; i++)
            {
                for (int j = 0; j < markers.cols; j++)
                {
                    int index = markers.at<int>(i, j);
                    if (index > 0 && index <= static_cast<int>(contours.size()))
                    {
                        dst.at<Vec3b>(i, j) = colors[index - 1];
                    }
                }
            }
            frame = dst.clone();
        }
        //Applying morphological transformation to a segmented frame
        else if (morphologicaloperatorsFlag) {
            // Change the background from white to black, since that will help later to extract
            // better results
            for (int i = 0; i < frame.rows; i++) {
                for (int j = 0; j < frame.cols; j++) {
                    if (frame.at<Vec3b>(i, j) == Vec3b(255, 255, 255))
                    {
                        frame.at<Vec3b>(i, j)[0] = 0;
                        frame.at<Vec3b>(i, j)[1] = 0;
                        frame.at<Vec3b>(i, j)[2] = 0;
                    }
                }
            }
            // Create a kernel that we will use to sharpen our image
            Mat kernel = (Mat_<float>(3, 3) <<
                1, 1, 1,
                1, -8, 1,
                1, 1, 1); // an approximation of second derivative, a quite strong kernel
            // do the laplacian filtering as it is
            // well, we need to convert everything in something more deeper then CV_8U
            // because the kernel has some negative values,
            // and we can expect in general to have a Laplacian image with negative values
            // BUT a 8bits unsigned int (the one we are working with) can contain values from 0 to 255
            // so the possible negative number will be truncated
            Mat imgLaplacian;
            filter2D(frame, imgLaplacian, CV_32F, kernel);
            Mat sharp;
            frame.convertTo(sharp, CV_32F);
            Mat imgResult = sharp - imgLaplacian;
            // convert back to 8bits gray scale
            imgResult.convertTo(imgResult, CV_8UC3);
            imgLaplacian.convertTo(imgLaplacian, CV_8UC3);
            // Create binary image from source image
            Mat bw;
            cvtColor(imgResult, bw, COLOR_BGR2GRAY);
            threshold(bw, bw, 40, 255, THRESH_BINARY | THRESH_OTSU);
            frame.release();
            //AfterSegmentational algorithm

            char TrackbarSelection[100];
            selectionSliderMax = 6;
            sprintf_s(TrackbarSelection, "erosion,dilation,opening,closing,mGradient,tHat,bHat x %d", selectionSliderMax);
            frame.release();
            createTrackbar(TrackbarSelection, "Frame", &selectionSlider, selectionSliderMax, selection_trackbar);
            selection_trackbar(selectionSlider, 0);

            char TrackbarElement[50];
            generalSliderMax = 2;
            sprintf_s(TrackbarElement, "Rect,Cross,Ellipse x %d", generalSliderMax);
            createTrackbar(TrackbarElement, "Frame", &generalSlider, generalSliderMax, general_trackbar);
            general_trackbar(generalSlider, 0);

            char TrackbarKernel[50];
            general2SliderMax = 31;
            sprintf_s(TrackbarKernel, "Kernel x %d", general2SliderMax);
            createTrackbar(TrackbarKernel, "Frame", &general2Slider, general2SliderMax, general2_trackbar);
            general2_trackbar(general2Slider, 0);
            if (general2 % 2 == 0) {
                general2 += 1;
            }
            if (selection == 0) {
                int erosion_type = general;
                Mat element = getStructuringElement(erosion_type,
                    Size(2 * general2 + 1, 2 * general2 + 1),
                    Point(general2, general2));
                erode(bw, frame, element);
            }
            else if (selection == 1) {
                int erosion_type = general;
                Mat element = getStructuringElement(erosion_type,
                    Size(2 * general2 + 1, 2 * general2 + 1),
                    Point(general2, general2));
                dilate(bw, frame, element);
            }
            else if (selection == 2) {
                Mat element = getStructuringElement(general, Size(2 * general2 + 1, 2 * general2 + 1), Point(general2, general2));
                morphologyEx(bw, frame, MORPH_OPEN, element);
            }
            else if (selection == 3) {
                Mat element = getStructuringElement(general, Size(2 * general2 + 1, 2 * general2 + 1), Point(general2, general2));
                morphologyEx(bw, frame, MORPH_CLOSE, element);
            }
            else if (selection == 4) {
                Mat element = getStructuringElement(general, Size(2 * general2 + 1, 2 * general2 + 1), Point(general2, general2));
                morphologyEx(bw, frame, MORPH_GRADIENT, element);
            }
            else if (selection == 5) {
                Mat element = getStructuringElement(general, Size(2 * general2 + 1, 2 * general2 + 1), Point(general2, general2));
                morphologyEx(bw, frame, MORPH_TOPHAT, element);
            }
            else{
                Mat element = getStructuringElement(general, Size(2 * general2 + 1, 2 * general2 + 1), Point(general2, general2));
                morphologyEx(bw, frame, MORPH_BLACKHAT, element);
            }

        }
        //Applying gradients
        else if (gradientsFlag) {
            char TrackbarSelection[100];
            selectionSliderMax = 2;
            sprintf_s(TrackbarSelection, "sobel,scharr,laplace x %d", selectionSliderMax);
            createTrackbar(TrackbarSelection, "Frame", &selectionSlider, selectionSliderMax, selection_trackbar);
            selection_trackbar(selectionSlider, 0);

            if (selection == 0) {

                char TrackbarScale[50];
                generalSliderMax = 55;
                sprintf_s(TrackbarScale, "Scale factor x %d", generalSliderMax);
                createTrackbar(TrackbarScale, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                if (general == 0) {
                    general += 1;
                }
                char TrackbarKsize[50];
                general2SliderMax = 3;
                sprintf_s(TrackbarKsize, "Sobel kernel x %d", general2SliderMax);
                createTrackbar(TrackbarKsize, "Frame", &general2Slider, general2SliderMax, general2_trackbar);
                general2_trackbar(general2Slider, 0);

                general2 = 2 * general2 + 1;

                char TrackbarDelta[50];
                general3SliderMax = 55;
                sprintf_s(TrackbarDelta, "Delta added to the result x %d", general3SliderMax);
                createTrackbar(TrackbarDelta, "Frame", &general3Slider, general3SliderMax, general3_trackbar);
                general3_trackbar(general3Slider, 0);

                Mat newFrame;
                // Remove noise by blurring with a Gaussian filter ( kernel size = 3 )
                GaussianBlur(frame, newFrame, Size(3, 3), 0, 0, BORDER_DEFAULT);
                // Convert the image to grayscale
                cvtColor(newFrame, frame, COLOR_BGR2GRAY);
                Mat grad_x, grad_y;
                Mat abs_grad_x, abs_grad_y;
                Sobel(frame, grad_x, CV_16S, 1, 0, general2, general, general3, BORDER_DEFAULT);
                Sobel(frame, grad_y, CV_16S, 0, 1, general2, general, general3, BORDER_DEFAULT);
                // converting back to CV_8U
                convertScaleAbs(grad_x, abs_grad_x);
                convertScaleAbs(grad_y, abs_grad_y);
                newFrame.release();
                frame.release();
                addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, newFrame);
                frame = newFrame.clone();
            }
            else if (selection == 1) {
                char TrackbarScale[50];
                generalSliderMax = 55;
                sprintf_s(TrackbarScale, "Scale factor x %d", generalSliderMax);
                createTrackbar(TrackbarScale, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                if (general == 0) {
                    general += 1;
                }
                
                char TrackbarDelta[50];
                general3SliderMax = 55;
                sprintf_s(TrackbarDelta, "Delta added to the result x %d", general3SliderMax);
                createTrackbar(TrackbarDelta, "Frame", &general3Slider, general3SliderMax, general3_trackbar);
                general3_trackbar(general3Slider, 0);

                Mat newFrame, cedge, edge1, edge2;
                cvtColor(frame, frame, COLOR_BGR2GRAY);
                blur(frame, newFrame, Size(3, 3));
                // Run the edge detector on grayscale
                Canny(newFrame, edge1, 1, 1 * 3, 3);
                cedge = Scalar::all(0);
                frame.copyTo(cedge, edge1);
                Mat dx, dy;
                Scharr(newFrame, dx, CV_16S, 1, 0, general, general3);
                Scharr(newFrame, dy, CV_16S, 0, 1, general, general3);
                Canny(dx, dy, edge2, 1, 1 * 3);
                cedge = Scalar::all(0);
                frame.copyTo(cedge, edge2);
                frame.release();
                frame = cedge.clone();
            }
            else {
                char TrackbarScale[50];
                generalSliderMax = 55;
                sprintf_s(TrackbarScale, "Scale factor x %d", generalSliderMax);
                createTrackbar(TrackbarScale, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);
                if (general == 0) {
                    general += 1;
                }
                char TrackbarKsize[50];
                general2SliderMax = 3;
                sprintf_s(TrackbarKsize, "Sobel kernel x %d", general2SliderMax);
                createTrackbar(TrackbarKsize, "Frame", &general2Slider, general2SliderMax, general2_trackbar);
                general2_trackbar(general2Slider, 0);

                general2 = 2 * general2 + 1;

                char TrackbarDelta[50];
                general3SliderMax = 55;
                sprintf_s(TrackbarDelta, "Delta added to the result x %d", general3SliderMax);
                createTrackbar(TrackbarDelta, "Frame", &general3Slider, general3SliderMax, general3_trackbar);
                general3_trackbar(general3Slider, 0);

                Mat smoothed, laplace;
                cvtColor(frame, frame, COLOR_BGR2GRAY);
                blur(frame, smoothed, Size(general2, general2));
                Laplacian(smoothed, laplace, CV_16S, general2,general,general3);
                frame.release();
                convertScaleAbs(laplace, frame, (3 + 1) * 0.25);
            }
        }
        //Applying canny edge algorithm
        else if (cannyedgeFlag) {
                char TrackbarScale[50];
                generalSliderMax = 100;
                sprintf_s(TrackbarScale, "Threshold x %d", generalSliderMax);
                createTrackbar(TrackbarScale, "Frame", &generalSlider, generalSliderMax, general_trackbar);
                general_trackbar(generalSlider, 0);

            Mat frameGray, detectedEdges;
            oldFrame = frame.clone();
            frame.release();
            frame.create(oldFrame.size(), oldFrame.type());
            cvtColor(oldFrame, frameGray, COLOR_BGR2GRAY);
            blur(frameGray, detectedEdges, Size(3, 3));
            Canny(detectedEdges, detectedEdges, general, general * 3, 3);
            frame = Scalar::all(0);
            oldFrame.copyTo(frame, detectedEdges);
        }
        //Applying my own YUV
        else if (YUVBGRFlag) {
            char TrackbarYUVType[100];
            generalSliderMax = 2;
            sprintf_s(TrackbarYUVType, "YUV\n0-4:4:4\n1-4:2:2\n2-4:2:0 x %d", generalSliderMax);
            createTrackbar(TrackbarYUVType, "Frame", &generalSlider, generalSliderMax, general_trackbar);
            general_trackbar(generalSlider, 0);
            char TrackbarBGRflag[100];
            general2SliderMax = 1;
            sprintf_s(TrackbarBGRflag, "BGR?\n0-False\n1-True x %d", general2SliderMax);
            createTrackbar(TrackbarBGRflag, "Frame", &general2Slider, general2SliderMax, general2_trackbar);
            general2_trackbar(general2Slider, 0);
            oldFrame = frame.clone();
            frame.release();
            if (general == 0) {
                frame = customBGR2YUV(oldFrame, "4:4:4");
                if (general2 == 1) {
                    oldFrame = frame.clone();
                    frame.release();
                    frame = customYUV2BGR(oldFrame, "4:4:4");
                }
            }
            else if (general == 1) {
                frame = customBGR2YUV(oldFrame, "4:2:2");
                if (general2 == 1) {
                    oldFrame = frame.clone();
                    frame.release();
                    frame = customYUV2BGR(oldFrame, "4:2:2");
                }
            }
            else {
                frame = customBGR2YUV(oldFrame, "4:2:0");
                if (general2 == 1) {
                    oldFrame = frame.clone();
                    frame.release();
                    frame = customYUV2BGR(oldFrame, "4:2:0");
                }
            }

        }
        // Display the resulting 
        imshow("Frame", frame);
        // Press  ESC on keyboard to exit
        char c = (char)waitKey(25);
        if (c == 27)
            break;
        if (waitKey(delay)) {
            continue;
        }
    }
    resetTrackbars();
    destroyWindow("Frame");
    destroyWindow("Red");
    destroyWindow("Blue");
    destroyWindow("Green");
    destroyWindow("Blue histogram");
    destroyWindow("Green histogram");
    destroyWindow("Red histogram");
    destroyWindow("ALL histogram");
    return 0;
}

/*! \function checkVideo
*   will check if what is in videoPath is valid and will start if it is valid
*/
void checkVideo() {
    if (videoPath == "Null") {
        cout << "Use HOPtion 'l' to load a video." << endl;
    }
    else {
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
            showVideo(inputVideo, delay);
            // When everything done, release the video capture object
            inputVideo.release();
            // Closes all the frames
            void destroyAllWindows();
        }
    }
}

/*! \function menuSpeech
* prints all available HOPtions
*/
String menuSpeech() {
    cout << "Edit options available and active: " << endl;
    cout << "'a':Watermark." << endl;
    cout << "'b':Color spaces and channels." << endl;
    cout << "'c':Each color histogram." << endl;
    cout << "'d':Grayscale image and apply histogram equalization." << endl;
    cout << "'e':Apply gaussian and blur filters." << endl;
    cout << "'f':Use segmentation algorithms." << endl;
    cout << "'g':Apply morphological operators on the segmented images." << endl;
    cout << "'h':Get image gradients." << endl;
    cout << "'i':Apply canny edge algorithm." << endl;
    cout << "'l':load a video path." << endl;
    cout << "'p':personal YUV2BGR and BGR2YUV." << endl;
    cout << "'n':n for compiling" << endl;
    cout << "'m':m for uncompiling" << endl;
    cout << "'q':Leave." << endl;
    cout << "Hoption= ";
    String hoption;
    getline(cin, hoption);
    return hoption;
}

/*! \function editMenu
*
*   this function opens the editing menu that
*   will be extremely usefull
*/
void editMenu() {
    String hoption = menuSpeech();
    while (hoption != "q") {
        if (hoption == "a") {
            watermarkFlag = true;
            cout << "Please input file path or a text: ";
            getline(cin, watermarkContent);
            checkVideo();
            watermarkFlag = false;
        }
        else if(hoption == "b"){
            colorspacesFlag = true;
            checkVideo();
            colorspacesFlag = false;
        }
        else if(hoption == "c"){
            histogramsFlag = true;
            checkVideo();
            histogramsFlag = false;
        }
        else if (hoption == "d") {
            grayscaleFlag = true;
            checkVideo();
            grayscaleFlag = false;
        }
        else if (hoption == "e") {
            gbkfiltersFlag = true;
            checkVideo();
            gbkfiltersFlag = false;
        }
        else if (hoption == "f") {
            segmentationalgorithmFlag = true;
            checkVideo();
            segmentationalgorithmFlag = false;
        }
        else if (hoption == "g") {
            morphologicaloperatorsFlag = true;
            checkVideo();
            morphologicaloperatorsFlag = false;
        }
        else if (hoption == "h") {
            gradientsFlag = true;
            checkVideo();
            gradientsFlag = false;
        }
        else if (hoption == "i") {
            cannyedgeFlag = true;
            checkVideo();
            cannyedgeFlag = false;
        }
        else if (hoption == "l") {
            cout << "Please give full path to the video file including the video file (example: D:\\videos\\video.mp4): ";
            getline(cin, videoPath);
            cout << videoPath << endl;
        }
        else if (hoption == "p") {
            YUVBGRFlag = true;
            checkVideo();
            YUVBGRFlag = false;
        }
        else if (hoption == "n") {
            cout << "Please input the yuv type(1-yuv444,2-yuv422,3-yuv420): ";
            string yuvType = "";
            getline(cin, yuvType);
            if (yuvType == "2") {
                cout << "Selected yuv422!" << endl;
                yuvType = "4:2:2";
            }
            else if (yuvType == "3") {
                cout << "Selected yuv420!" << endl;
                yuvType = "4:2:0";
            }
            else {
                cout << "Selected yuv444!" << endl;
                yuvType = "4:4:4";
            }
            cout << "Please input the name of the compressed bin file: ";
            string binF = "";
            getline(cin, binF);
            binF = "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\" + binF + ".bin";
            cout << "Please input the compression mode: \n1- a\n2- b\n3- c\n4- a+b-c\n5- a+(b-c)/2\n6- b+(a-c)/2\n7- (a+b)/2\n8- non linear\n: ";
            string commandInput = "";
            getline(cin, commandInput);
            int mode = stoi(commandInput);
            startTime = time(0);

            if (videoPath == "Null") {
                cout << "Use HOPtion 'l' to load a video." << endl;
            }
            else {
                // Create a VideoCapture object and open the input file
                VideoCapture inputVideo(videoPath);
                // Check if camera opened successfully
                if (!inputVideo.isOpened())
                {
                    cout << "Could not open the input video: " << videoPath << endl;
                }
                else {
                    //instead of showing compile
                    int fps = inputVideo.get(CAP_PROP_FPS);
                    int frame_width = inputVideo.get(CAP_PROP_FRAME_WIDTH);
                    int frame_height = inputVideo.get(CAP_PROP_FRAME_HEIGHT);
                    int frames = inputVideo.get(CAP_PROP_FRAME_COUNT);
                    int frameqty = 1;
                    Predictor predict(mode);
                    predict.saveInfo(frame_height, frame_width, yuvType, fps, frames, binF);
                    while (1) {
                        // Capture frame-by-frame
                        inputVideo >> frame;
                        // If the frame is empty, break immediately
                        if (frame.empty())
                            break;
                        Mat yuvFrame = customBGR2YUV(frame, yuvType);
                        predict.encode(yuvFrame);
                        if (frameqty % 5 == 0) {
                            endTime = time(0);
                            double estimated = difftime(endTime, startTime);
                            //estimated = (estimated/frameqty) * frames;
                            estimated = (estimated / ((frameqty * 100) / frames)) * (100 - ((frameqty * 100) / frames));
                            int seconds, hours, minutes;
                            seconds = int(estimated);
                            minutes = seconds / 60;
                            hours = minutes / 60;
                            cout << (frameqty * 100) / frames << "% estimated remaining time: " << hours << "H:" << int(minutes%60) << "M:" << int(seconds%60) << "S" << endl;
                        }
                        frameqty++;
                    }
                    // When everything done, release the video capture object
                    inputVideo.release();
                    predict.golombo.ReadWrite.close();
                }
            }
            endTime = time(0);
            double estimated = difftime(endTime, startTime);
            int seconds, hours, minutes;
            seconds = int(estimated);
            minutes = seconds / 60;
            hours = minutes / 60;
            cout << hours << ":" << int(minutes % 60) << ":" << int(seconds % 60) << endl;
        }
        else if (hoption == "m") {
            // Default resolution of the frame is obtained.The default resolution is system dependent. 
            cout << "Please input the name of the compressed bin file: ";
            string binF = "";
            getline(cin, binF);
            binF = "D:\\Desktop\\Universidade\\CSLP\\projeto\\binFiles\\" + binF + ".bin";
            Predictor predict2(8);
            startTime = time(0);
            predict2.readInfo(binF);
            int frame_width = predict2.videoWidth;
            int frame_height = predict2.videoHeight;
            int fps = predict2.videoFPS;
            string yuvType = predict2.type;
            cout << "Please input the name of the video file (saving as .avi): ";
            string videoF = "";
            getline(cin, videoF);
            string output = "D:\\Desktop\\Universidade\\CSLP\\projeto\\videos\\" + videoF + ".avi";
            // Define the codec and create VideoWriter object.The output is stored in 'outcpp.avi' file. 
            VideoWriter video(output, VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frame_width, frame_height));
            int frameqty = 1;
            int totalFrames = predict2.videoFrames;
            while (frameqty <= totalFrames)
            {
                Mat frame;
                // Capture frame-by-frame 
                frame = predict2.decode();
                //get decoded frame
                // If the frame is empty, break immediately
                if (frame.empty())
                    break;
                Mat BGRFrame = customYUV2BGR(frame, yuvType);
                // Write the frame into the file deterrmined in the string output
                video.write(BGRFrame);
                if (frameqty % 5 == 0) {
                    endTime = time(0);
                    double estimated = difftime(endTime, startTime);
                    //estimated = (estimated/frameqty) * totalFrames;
                    estimated = (estimated / ((frameqty * 100) / totalFrames)) * (100 - ((frameqty * 100) / totalFrames));
                    int seconds, hours, minutes;
                    seconds = int(estimated);
                    minutes = seconds / 60;
                    hours = minutes / 60;
                    cout << (frameqty * 100) / totalFrames << "% estimated remaining time: " << hours << "H:" << int(minutes % 60) << "M:" << int(seconds % 60) << "S" << endl;
                }
                frameqty++;
            }
            // When everything done, release and write object
            video.release();
            predict2.golombo.ReadWrite.close();
            endTime = time(0);
            double estimated = difftime(endTime, startTime);
            int seconds, hours, minutes;
            seconds = int(estimated);
            minutes = seconds / 60;
            hours = minutes / 60;
            cout << hours << ":" << int(minutes % 60) << ":" << int(seconds % 60) << endl;
        }
        else {
            cout << "Invalid HOPtion!" << endl;
        }

        hoption = menuSpeech();
    }
}

int main(int argc, char** argv)
{
    cout << "OpenCV version: " << CV_VERSION << endl;
    cout << "Welcome to my video player write 'h' to get all options available: ";
    String hoption;
    getline(cin, hoption);
    while (hoption != "q") {
        if (hoption == "h") {
            cout << "Current available hoptions are: " << endl << "h - to get all options available!" << endl << "q - to exit video player." << endl << "p - to play a video." << endl;
            cout << "e - to check current active edits or edit the video" << endl << "l - to load a video with his full path." << endl;
        }
        else if (hoption == "p") {
            if (videoPath == "Null") {
                cout << "Use HOPtion 'l' to load a video." << endl;
            }
            else {
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
                    showVideo(inputVideo, delay);
                    // When everything done, release the video capture object
                    inputVideo.release();
                    // Closes all the frames
                    void destroyAllWindows();
                }
            }
        }
        else if (hoption == "e") {
            editMenu();
        }
        else if (hoption == "l") {
            cout << "Please give full path to the video file including the video file (example: D:\\videos\\video.mp4): ";
            getline(cin, videoPath);
            cout << videoPath << endl;
        }
        else {
            cout << "Hoption not recognized" << endl;
        }
        cout << "Use 'h' to get all options available: ";
        getline(cin, hoption);
    }
    cout << "Bye!" << endl;
	return 0;
}