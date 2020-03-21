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
#endif

void usage();

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		usage();
		return -1;
	}

	FILE* file = fopen(argv[1], "rb");
	//FILE* file = fopen("in.bin", "rb");
	if (file == NULL)
	{
		printf("Can't open the input file!\n");
		return -1;
	}

	// if in Windows, write frames into this video
#ifdef _WIN64
	cv::VideoWriter WinVideo;
	WinVideo.open(argv[2], cv::VideoWriter::fourcc('a', 'v', 'c', '1'), VIDEO_FPS, cv::Size(OUT_FRAME_SIZE, OUT_FRAME_SIZE), true);
#endif

	Encoder coder;

	size_t maxTime = atoi(argv[3]);
	bool isLastFrame = false;
	int  FrameCounter = 0;

	while (!isLastFrame) {
		// loop times : how many bytes per Frame
			// get a byte from file
			// encode a byte   

		coder.resetCounter();
		for (size_t i = 0; i < FRAME_BITS; i += 12) {
			byte data;
			isLastFrame = (fscanf(file, "%c", &data) == EOF);
			coder.addByte(data, isLastFrame);
			if (isLastFrame)
			{
				coder.addByte(data, isLastFrame);
				coder.addByte(data, isLastFrame);
			}
		}

		// resize, extend to output Frame (add Anchor & status)
		// write Frame image to dir/
		cv::Mat out;
		coder.outFrame(out, FrameCounter);


#ifdef  __linux__
		cv::imwrite("Frame" + std::to_string(FrameCounter) + ".jpg", out);
#elif   _WIN64
		cv::imwrite("Frame" + std::to_string(FrameCounter) + ".jpg", out);
		WinVideo << out;
#endif
		FrameCounter++;
		if (FrameCounter * 1000 / VIDEO_FPS >= maxTime)
			break;


	}

	std::fclose(file);

	// if running in Linux, use ffmpeg command
#ifdef __linux__
	system(("ffmpeg -y -framerate " + std::to_string(VIDEO_FPS) +
		" -i \'output/Frame%d.jpg\' " + argv[2]).c_str());
#endif

	return 0;
}

void usage()
{
#define USAGE_FORMAT "  %-12s  %s\n"
	printf("%s\n", "NaiveCode Encoder");
	printf("%s\n", "Copyright (C) by HaiyunPresentation");
	printf("\n");
	printf("%s\n", "Usage:");
	printf("  %s\n", "encode <input_path> <output_path> <time>");
	printf(USAGE_FORMAT, "input_path", "Path of input binary file");
	printf(USAGE_FORMAT, "output_path", "Path of output video");
	printf(USAGE_FORMAT, "time", "Maximum time (ms) of output video");
	printf("\n");

}