#define CODEMAT_ANCHOR_SIZE 8
#define CODEMAT_MIDDLE_SIZE 64
#define CODEMAT_TOPBOT_SIZE (CODEMAT_MIDDLE_SIZE-2*CODEMAT_ANCHOR_SIZE)
#define BASE_ANCHOR_SIZE    7
#define PIXEL_SIZE          512
#define OUT_FRAME_SIZE      800

#define BIG_ANCHOR_RATE     8
#define SML_ANCHOR_RATE     4
#define BIG_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*BIG_ANCHOR_RATE)
#define SML_ANCHOR_SIZE     (BASE_ANCHOR_SIZE*SML_ANCHOR_RATE)

#define CUMU_BYTE_TOP       48
#define CUMU_BYTE_MID       432
#define CUMU_BYTE_BOT       480

#define BLACK_PIX           0
#define WHITE_PIX           255

#define VIDEO_FPS           6

typedef unsigned char byte;
