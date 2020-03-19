#include "definition.h"
#include <opencv2/opencv.hpp>

class Encoder
{
    cv::Mat     codeMatTop;     // top    part of codemat, record 48  byte; size(8row  * 48col)
    cv::Mat     codeMatMiddle;  // middle part of codemat, record 384 byte; size(48row * 64col)
    cv::Mat     codeMatBottom;  // bottom part of codemat, record 48  byte; size(8row  * 48col)
                                // totally record 480B per Frame

    cv::Mat     baseAnchor;     // make the Anchor(7*7)

    size_t      writeByte;      // how many byte this frame has writen
    size_t      writeChan;      // how many channel(BGR)    has writen

    cv::Size    outSize;        // the output frame size (default 1200 * 1200)

  private:
    void __inByte__(cv::Vec3b*, byte, bool);
        // pos -> where to write, and data -> what to write, isMask -> is start with mask==1
        // colorful QR input byte by step=3 (BGR 3 color channel)
    
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
