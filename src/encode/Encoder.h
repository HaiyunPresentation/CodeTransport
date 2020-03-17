#include "definition.h"
#include <opencv2/opencv.hpp>

class Encoder
{
    cv::Mat     codeMatTop;     // size(24, 4)     top part of codemat, record 12 byte
    cv::Mat     codeMatMiddle;  // size(32, 24) middle part of codemat, record 96 byte
    cv::Mat     codeMatBottom;  // size(24, 4)  bottom part of codemat, record 12 byte
                                // totally record 120B per Frame

    cv::Mat     baseAnchor;     // make the Anchor(7*7)

    size_t      writeByte;      // how many byte this frame has writen
    size_t      writeChan;      // how many channel(BGR)    has writen

    cv::Size    outSize;        // the output frame size (default 512*512)

  private:
    void __inByte__(cv::Vec3b*, byte, bool);
        // pos -> where to write, and data -> what to write, isMask -> is start with mask==1
        // color QR inByte by step=3
    
    void __initAnchor__();
        // init the baseAnchor

    void fixAnchor(cv::Mat&);
        // fix the block of anchor in code-mat

    void setAnchor(cv::Mat&);
        // set an Anchor by given rate to mat

    void setStatus(cv::Mat&, size_t);
        // set status in right-bottom, show which frame & where EOF

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
    
    void outFrame(cv::Mat&, size_t);
        // codeMat-base, and set output in given mat
};
