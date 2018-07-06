// Code for OpenCV + Camera - storage
// Team Genesis, Jul 2018
// Adapted from Okamiya Yuuki code in forum post at
// https://renesasrulz.com/gadget_renesas_user_forum/f/gr-lychee-design-contest-2018-in-india/10860/i2c-queries
// Refer to http://robotpowered.org/renesas/cam.html for more details
//
// How to run:
// Follow the steps (outlined in above blog) to setup the ide for gr, and Camera and Lychee board
// In the IDE, select GR-Lychee(Opencv) as the board, and select the appropriate port after connecting the Embed USB port
// Ensure SD card is inserted in the board
// Build and run the project
// Output:
// The camera files are stored in the SD card (root folder) named as image_%d_fps_%d.jpg
// Note:
// This code does not use OpenCV, though headers are included, to increase performance

#include <Arduino.h>
#include <Camera.h>
#include <opencv.hpp>
#include "SdUsbConnect.h"

using namespace cv;
#define IMAGE_HW 640
#define IMAGE_VW 480
uint8_t bgr_buf[3 * IMAGE_HW * IMAGE_VW]__attribute((section("NC_BSS"),aligned(32)));

Camera camera(IMAGE_HW, IMAGE_VW);
unsigned long lastTimeMS = 0;
unsigned long currTimeMS = 0;
unsigned long countCalls = 0;
float FPS = 0.0f;
unsigned long imageCount = 0;
SdUsbConnect storage("storage");

void setup() 
{
 Serial.begin(9600);

 while (!Serial)
 delay(10); // will pause Zero, Leonardo, etc until serial console opens

 Serial.println("OpenCV test");
 camera.begin();
 storage.wait_connect();
}

CvScalar RGB2YUV(float red,
                 float green,
                 float blue)
{
    return cvScalar(
            0.615 * red - 0.51499 * green - 0.10001 * blue,
            -0.14713 * red - 0.28886 * green + 0.436 * blue,
            0.299 * red + 0.587 * green + 0.114 * blue);
}

int calculateFPS()
{
    countCalls ++;
    currTimeMS = millis();
    float deltaSeconds = (float)(currTimeMS - lastTimeMS)/1000.0f;
    if(!(countCalls%100))
    {
        FPS = (float)countCalls/(deltaSeconds+0.01);
    }
    return (int)FPS;
}

void loop() 
{
    char filename[25];
    imageCount ++; 

    size_t jpegSize = camera.createJpeg(IMAGE_HW, IMAGE_VW, camera.getImageAdr(), Camera::FORMAT_YUV422);
    sprintf(filename, "/storage/image_%d_fps_%d.jpg", imageCount,calculateFPS());
    FILE * wp = fopen(filename, "w");
    if (wp != NULL) 
    {
        fwrite(camera.getJpegAdr(), sizeof(char), (int) jpegSize, wp);
        fclose(wp);
    }
}
