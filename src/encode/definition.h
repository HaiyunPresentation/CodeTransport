#define CODEMAT_ANCHOR_SIZE 8
#define CODEMAT_MIDDLE_SIZE 64
#define CODEMAT_TOPBOT_SIZE (CODEMAT_MIDDLE_SIZE-2*CODEMAT_ANCHOR_SIZE)
    // QRcode data-Mat size

#define BASE_ANCHOR_SIZE    7

#define PIXEL_SIZE          1024
#define OUT_FRAME_SIZE      1200
    // pixel-size of QRcode & background(output size)

#define N_CHANNEL           3
    // nChannel for number of channel

#define BIG_ANCHOR_RATE     16
#define SML_ANCHOR_RATE     8
#define BIG_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*BIG_ANCHOR_RATE)
#define SML_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*SML_ANCHOR_RATE)
    // Extend base-anchor by "rate", to "size"

#define TOPBOT_BYTE         (CODEMAT_TOPBOT_SIZE/8)
#define MIDDLE_BYTE         (CODEMAT_MIDDLE_SIZE/8)
    // how many bytes per row in top/bottom part

#define ACC_BYTE_TOP        48
#define ACC_BYTE_MID        432
#define ACC_BYTE_BOT        480
    // accumulative byte in each part of code-mat(top / middle / bottom)

#define FRAME_BYTES         (ACC_BYTE_BOT*N_CHANNEL)
    // how many byte per frame

#define PIX_BIT_ONE         0
#define PIX_BIT_ZRO         255
    // bit(0) set as color 0, bit(1) set as color 255

#define VIDEO_FPS           10
    // output QRcode into video by this frame-rate

#define TEST_HERE    std::cout << "test success here" << std::endl;

typedef unsigned char byte;
    // 8 bit