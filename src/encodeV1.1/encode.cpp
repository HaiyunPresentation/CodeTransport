#include "Encoder.h"
#include <iostream>

#ifdef  __linux__
    #include <stdlib.h>
#elif   WIN64
    #include <opencv2/videoio.hpp>
    #include <windows.h>
#endif

int main(int argc, char *argv[])
{
    // open file by agrv[1]
    // FILE* file = fopen(argv[1], "r");
    FILE* file = fopen("in.bin", "r");

    // if in windows, write frames into this
    #ifdef WIN64
	VideoWriter WinVideo;
	WinVideo.open("Video.avi", VideoWriter::fourcc('A', 'V', 'C', '1'), VIDEO_FPS, outputImg.size(), false);
    #endif

    Encoder coder;
    
    bool isLastFrame = false;   
    int FrameConter = 0;        

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
        FrameConter++;
        #ifdef  __linux__
            cv::imwrite("output/Frame" + std::to_string(FrameConter) + ".jpg", out);
        #elif   WIN64
            cv::imwrite("output\\Frame" + std::to_string(FrameConter) + ".jpg", out);
            WinVideo << out;
        #endif
    }

    std::fclose(file);

    #ifdef __linux__
    system(("ffmpeg -y -framerate "+ std::to_string(VIDEO_FPS) +
      " -i \'output/Frame%d.jpg\' output/Video.avi").c_str());
    #endif
    
    return 0;
}
