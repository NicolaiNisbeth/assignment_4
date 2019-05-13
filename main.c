
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "configurationFile.c"

#define PIXEL(frame, W, x, y) (frame+(y)*3*(W)+(x)*3) //https://answers.opencv.org/question/12963/how-to-get-pixels-value-from-a-picture/
#define MeanRGB(r, g, b) ((r + g + b) / 3)

const char *months[] = {
        "January", "February", "March", "April", "May", "June", "July",
        "August", "September", "October", "November", "December", NULL
};
int const FPS = 10;
int const DEVIATION_LIMIT = 5; //TODO: Discuss


// Function headers
char* generateTimestamp();
char *generateFilename();
int strToInt(char str[]);
void writeToFile(char *filename, char *content);
void modifyFirstLineInFile(char *filename, int amount);
void addTextToFrame(IplImage *frames, char *content);

int main (void ) {
    CvCapture* capture;
    IplImage *frame;
    char *pixel, *filename, *timestamp;
    int beginTracking = 0;
    int numOfEvents = 0, numOfFrames = 0;
    int rows, cols;
    int curIntensMean, prevIntensMean;
    int c;

    readConfigurations(); // From configurationFile

    capture = cvCaptureFromCAM( 0 );
    //cvNamedWindow("Video", CV_WINDOW_AUTOSIZE);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 50);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 50);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, FPS);

    if (capture){

        while(1){

            frame = cvQueryFrame(capture);
            if (!frame){break;}
            numOfFrames++;

            // shutdown program post 150 frames to avoid overheating
            //if (numOfFrames > 150){break;}

            // wait 5 seconds after camera starts for adjusting etc..
            if (numOfFrames > (FPS * 5)){
                beginTracking = 1;
            }


            int intensitySum = 0;
            rows = frame->height;
            cols = frame->width;
            for (int row = 0; row < rows; row++){
                for (int col = 0; col < cols; col++){
                    if (isPixelValid(row, col)){ // From configurationFile
                        pixel = PIXEL(frame->imageData, frame->nChannels, row, col);
                        intensitySum += MeanRGB(pixel[0], pixel[1], pixel[2]);
                    }
                    //pixel++;
                }
            }

            curIntensMean = intensitySum / (rows * cols);

            if (beginTracking){
                int deviation = abs(curIntensMean - prevIntensMean); //TODO: maybe a double instead instead of flooring the deviation
                //printf("%d\n", deviation);

                if (deviation >= DEVIATION_LIMIT){
                    filename = generateFilename();
                    timestamp = generateTimestamp();
                    writeToFile(filename, timestamp);

                    addTextToFrame(frame, timestamp);
                    // TODO: find a better way to make valid filename.jpg
                    char *frameFilename = (char*) malloc(40 * sizeof(char));
                    strcpy(frameFilename, timestamp);
                    strcat(frameFilename, ".jpg");
                    cvSaveImage(frameFilename, frame, 0);

                    // resets tracking
                    beginTracking = 0;
                    numOfFrames = 0;

                    numOfEvents++;
                    modifyFirstLineInFile(filename, numOfEvents);

                    free(timestamp);
                    free(filename);
                    free(frameFilename);
                }
            }
            prevIntensMean = curIntensMean;

            //cvShowImage("Video", frame);

            c = cvWaitKey(10);
            if ((char) c == 'c'){
                break;
            }
        }
    }
    else {
        puts("Can't find official driver!\nActivate with cmd: sudo modprobe bcm2835-v4l2");
    }

    cvReleaseImage(&frame);
    cvReleaseCapture (&capture);
    free(xIntervals);
    free(yIntervals);
    //cvDestroyWindow("Video");
}

char* generateTimestamp(){
    char *timeOfEvent = (char*) malloc(26 * sizeof(char));
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(timeOfEvent, 26, "%Y-%m-%d %H:%M:%S\n", tm_info);
    printf("%s", timeOfEvent);

    return timeOfEvent;
}

char* generateFilename() {
    char *filename = (char*) malloc(30 * sizeof(char));
    time_t timer;
    struct tm* timeInfo;
    char year[5];
    char monthStr[3];
    int monthInt;

    time(&timer);
    timeInfo = localtime(&timer);

    strftime(year, 5, "%Y", timeInfo);
    //sprintf(year, "%d", timeInfo->tm_year);

    sprintf(monthStr, "%d", timeInfo->tm_mon + 1);
    monthInt = strToInt(monthStr);

    strcpy(filename, "log");
    strcat(filename, year);
    strcat(filename, months[monthInt-1]);
    strcat(filename, ".txt");

    return filename;
}

int strToInt(char str[]) {
    int monthInt = 0;
    for (int i = 0; str[i] != '\0'; i++){
        monthInt = monthInt * 10 + str[i] - '0';
    }
    return monthInt;
}

void writeToFile(char *filename, char *content) {
    long fileSize = 0;
    FILE *cfPtr;

    if ((cfPtr = fopen(filename, "a+")) == NULL){
        puts("file not found");
    }
    else {
        if (!feof(stdin)){
            fseek(cfPtr, 0, SEEK_END);
            fileSize = ftell(cfPtr);

            if (fileSize == 0){
                fprintf(cfPtr, "     \n"); //5 spaces placeholder equals 6 character replacement.
            }

            fprintf(cfPtr, "%s", content);
        }
    }
    fclose(cfPtr);
}

void modifyFirstLineInFile(char *filename, int amount) {
    FILE *cfPtr;
    char replacement[10];

    if ((cfPtr = fopen(filename, "r+")) == NULL){
        puts("file could not be opened");
    }
    else {
        sprintf(replacement, "%d", amount);
        fseek(cfPtr, 0, SEEK_SET);
        fprintf(cfPtr, "%s", replacement);
    }
    fclose(cfPtr);
}

void addTextToFrame(IplImage *frames, char *content){
    CvScalar color;
    CvFont base_font;

    color = CV_RGB(255,255,255);
    cvInitFont(&base_font, CV_FONT_HERSHEY_DUPLEX, 1.5, 1.5, 0, 1, 8);

    cvPutText(frames, content, cvPoint(0, (frames->height-20)), &base_font, color);
}

