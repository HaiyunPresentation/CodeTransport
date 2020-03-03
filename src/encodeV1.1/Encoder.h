#include "definition.h"
#include <opencv2/opencv.hpp>

class Encoder
{
    cv::Mat     codeMatTop;     // size(24, 4)     top part of codemat, record 12 byte
    cv::Mat     codeMatMid;     // size(32, 24) middle part of codemat, record 96 byte
    cv::Mat     codeMatBot;     // size(24, 4)  bottom part of codemat, record 12 byte
                                // totally record 120B per Frame
    cv::Mat     baseAnchor;     // make the Anchor(7*7)
    
    size_t      writeByte;      // how many byte this frame has writen
    cv::Size    outSize;        // the output frame size (default 512*512)

  private:
    void __inByte__(byte);
        // writeByte -> where to write, and data -> what to write

  public:
    Encoder();

    void resetCounter();
        // set writeByte zero

    void setOutSize(cv::Size);
        // set outFrame Size
    void setOutSize(size_t, size_t);
        // set outFrame Size (default 512*512)

    void addByte(byte, bool);
        // addByte(byte data, bool isEOF);
    
    void outFrame(cv::Mat&);
        // resize & add dingweidian;
};
