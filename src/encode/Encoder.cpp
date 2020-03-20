#include "Encoder.h"

// private:

void Encoder::__initAnchor__() {
	// make Anchor like "回", in black-backgroud

	for (size_t bAnchorRow = 1; bAnchorRow < BASE_ANCHOR_SIZE - 1; bAnchorRow++) {
		// scan each rows from 1 to 5, put white pix

		uchar* pDraw = this->baseAnchor.data + bAnchorRow * BASE_ANCHOR_SIZE * N_CHANNEL;

		if (bAnchorRow == 1 || bAnchorRow == BASE_ANCHOR_SIZE - 2) {
			for (size_t bAnchorCol = N_CHANNEL; bAnchorCol < (BASE_ANCHOR_SIZE - 1) * N_CHANNEL; bAnchorCol++)
				*(pDraw + bAnchorCol) = PIX_BIT_ZRO;
		}
		else {
			for (size_t nChan = N_CHANNEL; nChan < 2 * N_CHANNEL; nChan++)
				*(pDraw + nChan) = *(pDraw + BASE_ANCHOR_SIZE * N_CHANNEL - 1 - nChan) = PIX_BIT_ZRO;
		}
	}
}

void Encoder::__inByte__(cv::Vec3b* pix, byte data, bool isMask, int changeLocation, bool isEOF) {
	// write byte data start at pix
	// a byte only write in one color-channel
	if (isEOF)
	{
		for (size_t i = 0U; i < 4U; i++, pix++) {
			// data & 0x80 => is bit(1)? [ bit(1) ought to be black ]
			(*pix)[this->writeChan] = (bool(data & 0x80) ^ isMask) ? PIX_BIT_ONE : PIX_BIT_ZRO;
			data <<= 1;
			isMask = !isMask;     // the mask of next bit ought to be changed
		}
		this->writeBit += 4;
		return;
	}
	byte crc = crc4ITU(data);
	for (size_t i = 0U; i < 8U; i++, pix++) {
		if (i == changeLocation) isMask = !isMask;
		// data & 0x80 => is bit(1)? [ bit(1) ought to be black ]
		(*pix)[this->writeChan] = (bool(data & 0x80) ^ isMask) ? PIX_BIT_ONE : PIX_BIT_ZRO;
		data <<= 1;
		isMask = !isMask;     // the mask of next bit ought to be changed

	}
	if (changeLocation == 8) isMask = !isMask;
	for (size_t i = 0U; i < 4U; i++, pix++) {
		// data & 0x08 => is bit(1)? [ bit(1) ought to be black ]
		(*pix)[this->writeChan] = (bool(crc & 0x08) ^ isMask) ? PIX_BIT_ONE : PIX_BIT_ZRO;
		crc <<= 1;
		isMask = !isMask;     // the mask of next bit ought to be changed
	}
	this->writeBit += 12;
}

uint8_t Encoder::crc4ITU(byte data)
{
	uint8_t i;
	uint8_t crc = 0;        // Initial value  
	crc ^= data;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0x0C;// 0x0C = (reverse 0x03)>>(8-4)  
		else
			crc = (crc >> 1);
	}
	return crc;
}

/*void Encoder::fixAnchor(cv::Mat& fixMat){
	// fix anchor part with 3 part of code-mat into a Mat

	fixMat = cv::Mat(CODEMAT_MIDDLE_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC3, cv::Scalar(255, 255, 255));

	// copy codeMats into fixMat
	cv::Mat outRect = fixMat(cv::Rect(CODEMAT_ANCHOR_SIZE, 0, this->codeMatTop.cols, this->codeMatTop.rows));
	this->codeMatTop.copyTo(outRect);

	outRect = fixMat(cv::Rect(0, CODEMAT_ANCHOR_SIZE, this->codeMatMiddle.cols, this->codeMatMiddle.rows));
	this->codeMatMiddle.copyTo(outRect);

	outRect = fixMat(cv::Rect(CODEMAT_ANCHOR_SIZE, fixMat.rows - CODEMAT_ANCHOR_SIZE,
	  this->codeMatBottom.cols, this->codeMatBottom.rows));
	this->codeMatBottom.copyTo(outRect);
}*/

void Encoder::setStatus(cv::Mat& QRmat, size_t nFrame) {
	// set Status Block in right-bottom
	// show: No. of Frame, isEOF, where is EOF

	// If a color channel of the frame is full,
	//  the color channel of the background set to 0(bit_one_color)
	cv::Vec3b baseColor;
	for (size_t i = 0; i < N_CHANNEL; i++) {
		baseColor[i] = this->eofChan > i ? PIX_BIT_ONE : PIX_BIT_ZRO;
	}
	cv::Mat statusMat(4, 4, CV_8UC3, cv::Scalar(baseColor));

	// if baseColor is black, stand for 3 channels(BGR) are all full
	// if not, means EOF
	if (baseColor != cv::Vec3b(0, 0, 0)) {
		// EOF,
		// set Status Block by [(4),(4)] [(4),(4)], 
		// first  8 bit stand for EOF in which row
		// second 8 bit stand for EOF in which col (bit)

		byte row = this->eofBit / CODEMAT_SIDE_LENGTH;
		byte col = this->eofBit % CODEMAT_SIDE_LENGTH;
		/*if (this->writeBit < ACC_BYTE_TOP){
			row = this->writeBit / TOPBOT_BYTE;
			col = this->writeBit % TOPBOT_BYTE + 1;
		}
		else if (this->writeBit < ACC_BYTE_MID){
			row = (this->writeBit - ACC_BYTE_TOP) / MIDDLE_BYTE + 8;
			col = (this->writeBit - ACC_BYTE_TOP) % MIDDLE_BYTE;
		}
		else if (this->writeBit < ACC_BYTE_BOT){
			row = (this->writeBit - ACC_BYTE_MID) / TOPBOT_BYTE + 56;
			col = (this->writeBit - ACC_BYTE_MID) % TOPBOT_BYTE + 1;
		}*/

		col = 8 * col;
		cv::Vec3b* pStatus = statusMat.ptr<cv::Vec3b>(0);
		for (size_t i = 0; i < 8; i++, pStatus++) {
			(*pStatus)[this->eofChan] = row & 0x80 ? PIX_BIT_ONE : PIX_BIT_ZRO;
			row <<= 1;
		}
		for (size_t i = 0; i < 8; i++, pStatus++) {
			(*pStatus)[this->eofChan] = col & 0x80 ? PIX_BIT_ONE : PIX_BIT_ZRO;
			col <<= 1;
		}
	}

	// set status-mat to right-bottom
	statusMat.copyTo(QRmat(cv::Rect(
		QRmat.cols - CODEMAT_ANCHOR_SIZE + 4 * ((nFrame % 3) % 2),
		QRmat.rows - CODEMAT_ANCHOR_SIZE + 4 * ((nFrame % 3) / 2),
		statusMat.cols, statusMat.rows
	)));
}

void Encoder::setAnchor(cv::Mat& exMat) {
	cv::Mat anchor;     // extend baseAnchor
	cv::resize(this->baseAnchor, anchor, cv::Size(BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE), 0.0f, 0.0f, 0);

	// add big Anchor in ↖, ↗, and ↙
	cv::Mat outRect = exMat(cv::Rect(0, 0, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
	anchor.copyTo(outRect);
	outRect = exMat(cv::Rect(0, exMat.rows - BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
	anchor.copyTo(outRect);
	outRect = exMat(cv::Rect(exMat.cols - BIG_ANCHOR_SIZE, 0, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
	anchor.copyTo(outRect);

	// add small Anchor in ↘
	cv::resize(this->baseAnchor, anchor, cv::Size(SML_ANCHOR_SIZE, SML_ANCHOR_SIZE), 0.0f, 0.0f, 0);
	outRect = exMat(cv::Rect(exMat.cols - SML_ANCHOR_SIZE, exMat.rows - SML_ANCHOR_SIZE,
		SML_ANCHOR_SIZE, SML_ANCHOR_SIZE));
	anchor.copyTo(outRect);
}

// public:

Encoder::Encoder() :
	QRcode(cv::Mat(CODEMAT_MIDDLE_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC3, cv::Scalar(255, 255, 255))),
	baseAnchor(cv::Mat::zeros(BASE_ANCHOR_SIZE, BASE_ANCHOR_SIZE, CV_8UC3)),
	outSize(cv::Size(OUT_FRAME_SIZE, OUT_FRAME_SIZE)) {
	this->eofBit = -1;
	this->eofChan = -1;
	this->writeBit = 0;
	this->__initAnchor__();
}

void Encoder::resetCounter() {
	// set as non data written
	this->QRcode = cv::Mat(CODEMAT_MIDDLE_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC3, cv::Scalar(255, 255, 255));
	this->writeBit = 0;
	this->writeChan = 0;
}

void Encoder::setOutSize(cv::Size outSize) {
	this->outSize = outSize;
}

void Encoder::setOutSize(size_t size_row = OUT_FRAME_SIZE, size_t size_col = OUT_FRAME_SIZE) {
	this->setOutSize(cv::Size(size_row, size_col));
}

void Encoder::addByte(byte data, bool isEOF) {

	while (NON_BLOCK_AREA(this->writeBit))
	{
		this->writeBit++;
	}

	/*if (isEOF){
		// remenber written status
		size_t bits = this->writeBit;
		size_t chans = this->writeChan;

		// EndFrame: add '1111 1111' until ACC_BYTE_BOT Bytes in this frame
		while(this->writeBit < FRAME_BITS && this->writeChan < 3){
			this->addByte(0x00, isEOF);
		}

		// real data bits
		this->writeBit = bits;
		this->writeChan = chans;
		return;
	}

	else */
	if (this->writeChan >= N_CHANNEL) {
		// error
		return;
	}

	cv::Vec3b* pix;     // position of where to start write byte
	bool isMask;        // prepare for Mask, which implies the byte's first bit meets a black
	int changeLocation = -1;
	size_t pix_row = this->writeBit / CODEMAT_SIDE_LENGTH;
	size_t pix_col = this->writeBit % CODEMAT_SIDE_LENGTH;
	pix = this->QRcode.ptr<cv::Vec3b>(pix_row) + pix_col; // byte-position
	isMask = pix_row % 2;
	if (!isEOF)
	{
		if ((pix_row >= CODEMAT_ANCHOR_SIZE) && (pix_row < CODEMAT_SIDE_LENGTH - CODEMAT_ANCHOR_SIZE))
		{
			if (pix_row % 3 == 2 && pix_col == 60)
				changeLocation = 4;
			if (pix_row % 3 == 0 && pix_col == 56)
				changeLocation = 8;
		}
	}
	else if (this->eofBit == -1)
	{
		this->eofBit = this->writeBit;
		this->eofChan = this->writeChan;
	}

	this->__inByte__(pix, data, isMask, changeLocation, isEOF);
	//this->writeBit+=8;
   // if(!isEOF)

	//std::cout << this->writeBit << std::endl;
	// if a color channel is full
	if (this->writeBit >= ACC_BIT_BOT) {
		this->writeChan++;
		this->writeBit = 0;
	}
}

void Encoder::outFrame(cv::Mat& out, size_t nFrame) {
	cv::Mat QRcode;
	// save byte as QRcode

	/*this->fixAnchor(QRcode);
		// unit 3 part codeMat*/
	this->setStatus(this->QRcode, nFrame);
	// add frame status

	cv::resize(this->QRcode, this->QRcode, cv::Size(PIXEL_SIDE_LENGTH, PIXEL_SIDE_LENGTH), 0.0f, 0.0f, 0);
	// Extend to (PIXEL_SIDE_LENGTH * PIXEL_SIDE_LENGTH)
//cv::imshow("QRC", this->QRcode);
//cv::waitKey();

	this->setAnchor(this->QRcode);
	// add 4 Anchor

	out = cv::Mat(OUT_FRAME_SIZE, OUT_FRAME_SIZE, CV_8UC3, cv::Scalar(255, 255, 255));
	// set ouput backgroud color as white( add white rim to QRcode )
	this->QRcode.copyTo(out(cv::Rect(
		(OUT_FRAME_SIZE - PIXEL_SIDE_LENGTH) / 2, (OUT_FRAME_SIZE - PIXEL_SIDE_LENGTH) / 2, PIXEL_SIDE_LENGTH, PIXEL_SIDE_LENGTH)
	));
}