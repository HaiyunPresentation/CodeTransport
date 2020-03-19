#include "Encoder.h"
#include <iostream>

/*
    project grew in linux or windows
    linux:  save every QR-Code as images, then run ffmpeg in shell make the video
    windows:save all QR-Code as video directly
*/

#ifdef  __linux__
    #include <stdlib.h>
#elif _WIN64
    #include <opencv2/videoio.hpp>
    #include <windows.h>
#endif

int main(int argc, char *argv[])
{
    // open file by agrv[1]
    // FILE* file = fopen(argv[1], "rb");
    FILE* file = fopen("in.bin", "rb");


    // if in Windows, write frames into this video
    #ifdef _WIN64
	cv::VideoWriter WinVideo;
	WinVideo.open("in.mp4",cv::VideoWriter::fourcc('A', 'V', 'C', '1'), VIDEO_FPS, cv::Size(OUT_FRAME_SIZE, OUT_FRAME_SIZE), false);
    #endif

    Encoder coder;
    
    bool isLastFrame  = false;   
    int  FrameCounter = 0;        

    while(!isLastFrame){
        // loop times : how many bytes per Frame
            // get a byte from file
            // add a byte to   encode

        coder.resetCounter();
        for (size_t i = 0; i < FRAME_BYTES && !isLastFrame; i++){
            byte data;
            isLastFrame = (fscanf(file, "%c", &data) == EOF);
            coder.addByte(data, isLastFrame);
        }
        
        // resize, extend to output Frame (add Anchor & status)
        // write Frame image to dir/
        cv::Mat out;
        coder.outFrame(out, FrameCounter);
        FrameCounter++;

        #ifdef  __linux__
            cv::imwrite("output/Frame" + std::to_string(FrameCounter) + ".jpg", out);
        #elif   _WIN64
            cv::imwrite("output\\Frame" + std::to_string(FrameCounter) + ".jpg", out);
            WinVideo << out;
        #endif
    }

    std::fclose(file);

	// if running in Linux, use ffmpeg command
    #ifdef __linux__
    system(("ffmpeg -y -framerate "+ std::to_string(VIDEO_FPS) +
      " -i \'output/Frame%d.jpg\' output/in.mp4").c_str());
    #endif
    
    return 0;
}
