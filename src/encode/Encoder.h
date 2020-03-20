#include "definition.h"
#include <opencv2/opencv.hpp>

class Encoder
{
	/*cv::Mat     codeMatTop;     // top    part of codemat, record 48  byte; size(8row  * 48col)
	cv::Mat     codeMatMiddle;  // middle part of codemat, record 384 byte; size(48row * 64col)
	cv::Mat     codeMatBottom;  // bottom part of codemat, record 48  byte; size(8row  * 48col)
								// totally record 480B per Frame*/

	cv::Mat QRcode;				// save byte as QRcode


	cv::Mat     baseAnchor;     // make the Anchor(7*7)

	size_t      writeBit;      // how many byte this frame has been written
	size_t      writeChan;      // how many channels (BGR)  has been written

	size_t      eofBit;
	size_t      eofChan;

	cv::Size    outSize;        // the output frame size (default 1200 * 1200)

private:
	// pos -> where to write, and data -> what to write, isMask -> is start with mask==1, changeLocation -> where changes mask, isEOF -> is EOF
	// colorful QR input byte by step=3 (BGR 3 color channel)
	void __inByte__(cv::Vec3b*, byte, bool, int, bool);

	uint8_t crc4ITU(byte);

	// init the baseAnchor
	void __initAnchor__();

	// fix the block of anchor in code-mat
	void fixAnchor(cv::Mat&);

	// set an Anchor by given rate to mat        
	void setAnchor(cv::Mat&);

	// set status in right-bottom, show which frame & where EOF
	void setStatus(cv::Mat&, size_t);

public:

	Encoder();

	// set writeBit zero
	void resetCounter();

	// set outFrame Size
	void setOutSize(cv::Size);

	// set outFrame Size (default 512*512)
	void setOutSize(size_t, size_t);

	// addByte(byte data, bool isEOF);
	void addByte(byte, bool);

	// codeMat-base, and set output in given mat
	void outFrame(cv::Mat&, size_t);
};
