#define CODEMAT_SIDE_LENGTH 64
#define CODEMAT_ANCHOR_SIZE 8
#define CODEMAT_MIDDLE_SIZE CODEMAT_SIDE_LENGTH
#define CODEMAT_TOPBOT_SIZE (CODEMAT_MIDDLE_SIZE-2*CODEMAT_ANCHOR_SIZE)
// NaiveCode data-Mat size

#define PIXEL_SIDE_LENGTH   1024
#define OUT_FRAME_SIZE      1200
	// pixel length of NaiveCode side & background(output size)


// non-block area of NaiveCode mat 
#define NON_BLOCK_AREA(x) ((x / CODEMAT_SIDE_LENGTH < CODEMAT_ANCHOR_SIZE \
	|| x / CODEMAT_SIDE_LENGTH >= CODEMAT_SIDE_LENGTH - CODEMAT_ANCHOR_SIZE) \
&& (x % CODEMAT_SIDE_LENGTH >= CODEMAT_SIDE_LENGTH - CODEMAT_ANCHOR_SIZE \
	|| x % CODEMAT_SIDE_LENGTH < CODEMAT_ANCHOR_SIZE))


#define N_CHANNEL           3
	// nChannel for number of channel

#define BASE_ANCHOR_SIZE    7
#define BIG_ANCHOR_RATE     16
#define SML_ANCHOR_RATE     8
#define BIG_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*BIG_ANCHOR_RATE)
#define SML_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*SML_ANCHOR_RATE)
	// Extend base-anchor by "rate", to "size"

#define TOPBOT_BYTE         (CODEMAT_TOPBOT_SIZE/12)
#define MIDDLE_BYTE         (CODEMAT_MIDDLE_SIZE/12)
	// how many bytes per row in top/bottom part


#define ACC_REAL_BIT_BOT    3840
#define ACC_BIT_BOT         4088
	// accumulative byte in each part of code-mat(top / middle / bottom)


#define FRAME_BITS          (ACC_REAL_BIT_BOT*N_CHANNEL)

#define PIX_BIT_ONE         0
#define PIX_BIT_ZRO         255
	// bit(0) set as color 0, bit(1) set as color 255

#define VIDEO_FPS           20
	// output QRcode into video by this frame-rate

#define TEST_HERE    std::cout << "test success here" << std::endl;

typedef unsigned char byte;
// 8 bit