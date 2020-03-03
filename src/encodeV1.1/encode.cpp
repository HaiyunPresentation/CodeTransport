#include "Encoder.h"
#include <iostream>

int main(int argc, char *argv[])
{
    // open file by agrv[1]
    // FILE* file = fopen(argv[1], "r");
    FILE* file = fopen("in.bin", "r");
    
    // create a Encoder
    Encoder coder;
    
    bool isLastFrame = false;   // End encode sign
    int FrameConter = 0;        // counte how many frames

    while(!isLastFrame){
        // loop times : how many bytes per Frame
            // get a byte from file
            // Encoder.addByte(btye, isEOF);

        coder.resetCounter();
        for (size_t i = 0; i < CUMU_BYTE_BOT && !isLastFrame; i++){
            byte data;
            isLastFrame = (fscanf(file, "%c", &data) == EOF);
            coder.addByte(data, isLastFrame);
        }
        
        // resize() to extend to Frame(add DingWeiDian)
        // write Frame to dir
        
        cv::Mat out;
        coder.outFrame(out);
		cv::imwrite("output/Frame" + std::to_string(FrameConter) + ".jpg", out);
        FrameConter++;
    }

    // system(ffmpeg) imgs->video
    
    return 0;
}
