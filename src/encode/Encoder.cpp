#include "Encoder.h"

// private

void Encoder::__initAnchor__(){
    for (size_t bAnchorRow = 1; bAnchorRow < BASE_ANCHOR_SIZE - 1; bAnchorRow++){
        // make Anchor like "回"

        uchar* pDraw = this->baseAnchor.data + bAnchorRow * BASE_ANCHOR_SIZE*N_CHANNEL;

        if (bAnchorRow == 1 || bAnchorRow == BASE_ANCHOR_SIZE - 2){
            for (size_t bAnchorCol = N_CHANNEL; bAnchorCol < (BASE_ANCHOR_SIZE - 1)*N_CHANNEL; bAnchorCol++)
                *(pDraw + bAnchorCol) = PIX_BIT_ZRO;
        }
        else{
            for (size_t nChan = N_CHANNEL; nChan < 2*N_CHANNEL; nChan++)
                *(pDraw + nChan) = *(pDraw + BASE_ANCHOR_SIZE*N_CHANNEL-1 - nChan) = PIX_BIT_ZRO;
        }
    }
}

void Encoder::__inByte__(cv::Vec3b* pix, byte data, bool isMask){
    for (size_t i = 0U; i < 8U; i++, pix++){
        // data & 0x80 => is bit(1)[ought to be black]
        (*pix)[this->writeChan] = (bool(data & 0x80) ^ isMask) ? PIX_BIT_ONE : PIX_BIT_ZRO;
        data <<= 1;
        isMask = !isMask;     // the next bit mask ought to chuange
    }
}

void Encoder::fixAnchor(cv::Mat& fixMat){
    fixMat = cv::Mat(CODEMAT_MIDDLE_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC3, cv::Scalar(255, 255, 255));

    cv::Mat outRect = fixMat(cv::Rect(CODEMAT_ANCHOR_SIZE, 0, this->codeMatTop.cols, this->codeMatTop.rows));
    this->codeMatTop.copyTo(outRect);

    outRect = fixMat(cv::Rect(0, CODEMAT_ANCHOR_SIZE, this->codeMatMiddle.cols, this->codeMatMiddle.rows));
    this->codeMatMiddle.copyTo(outRect);

    outRect = fixMat(cv::Rect(CODEMAT_ANCHOR_SIZE, fixMat.rows - CODEMAT_ANCHOR_SIZE,
      this->codeMatBottom.cols, this->codeMatBottom.rows));
    this->codeMatBottom.copyTo(outRect);
}

void Encoder::setAnchor(cv::Mat& exMat){
    cv::Mat anchor;     // extend baseAnchor
    cv::resize(this->baseAnchor, anchor, cv::Size(BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE), 0.0f, 0.0f, 0);
    
    cv::Mat outRect = exMat(cv::Rect(0, 0, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
    anchor.copyTo(outRect);
    outRect = exMat(cv::Rect(0, exMat.rows - BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
    anchor.copyTo(outRect);
    outRect = exMat(cv::Rect(exMat.cols - BIG_ANCHOR_SIZE, 0, BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE));
    anchor.copyTo(outRect);

    cv::resize(this->baseAnchor, anchor, cv::Size(SML_ANCHOR_SIZE, SML_ANCHOR_SIZE), 0.0f, 0.0f, 0);
    outRect = exMat(cv::Rect(exMat.cols - SML_ANCHOR_SIZE, exMat.rows - SML_ANCHOR_SIZE,
      SML_ANCHOR_SIZE, SML_ANCHOR_SIZE));
    anchor.copyTo(outRect);
}

// public

Encoder::Encoder():
  codeMatTop(CODEMAT_ANCHOR_SIZE, CODEMAT_TOPBOT_SIZE, CV_8UC3),
  codeMatMiddle(CODEMAT_TOPBOT_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC3),
  codeMatBottom(CODEMAT_ANCHOR_SIZE, CODEMAT_TOPBOT_SIZE, CV_8UC3),
  baseAnchor(cv::Mat::zeros(BASE_ANCHOR_SIZE, BASE_ANCHOR_SIZE, CV_8UC3)),
  outSize(cv::Size(OUT_FRAME_SIZE, OUT_FRAME_SIZE)){
    this->writeByte = 0;
    this->__initAnchor__();
}

void Encoder::resetCounter(){
    this->writeByte = 0;
    this->writeChan = 0;
}

void Encoder::setOutSize(cv::Size outSize){
    this->outSize=outSize;
}

void Encoder::setOutSize(size_t size_row=OUT_FRAME_SIZE, size_t size_col=OUT_FRAME_SIZE){
    this->setOutSize(cv::Size(size_row, size_col));
}

void Encoder::addByte(byte data, bool isEOF){
    if (isEOF){
        // EndFrame: add '0000 0000' until ACC_BYTE_BOT Bytes in this frame
        while(this->writeByte < FRAME_BYTES && this->writeChan < 3){
            this->addByte(0x00, false);
        }

        return;
    }

    // data is OK

    cv::Vec3b* pix;     // position of where to start write byte
    bool isMask;        // prepare for Mask, is the byte's first bit meets a black
    size_t pix_row;
    size_t pix_col;

    if (this->writeByte < ACC_BYTE_TOP){
        pix_row = this->writeByte / TOPBOT_BYTE;
        pix_col = this->writeByte % TOPBOT_BYTE;
        
        pix = this->codeMatTop.ptr<cv::Vec3b>(pix_row) + 8*pix_col;
            // byte-position
        
        isMask = pix_row % 2;
    }
    else if (this->writeByte < ACC_BYTE_MID){
        pix_row = (this->writeByte - ACC_BYTE_TOP) / MIDDLE_BYTE;
        pix_col = (this->writeByte - ACC_BYTE_TOP) % MIDDLE_BYTE;

        pix = this->codeMatMiddle.ptr<cv::Vec3b>(pix_row) + 8*pix_col;
        isMask = pix_row % 2;
    }
    else if (this->writeByte < ACC_BYTE_BOT){
        pix_row = (this->writeByte - ACC_BYTE_MID) / TOPBOT_BYTE;
        pix_col = (this->writeByte - ACC_BYTE_MID) % TOPBOT_BYTE;

        pix = this->codeMatBottom.ptr<cv::Vec3b>(pix_row) + 8*pix_col;
        isMask = pix_row % 2;
    }
    else{
        // ERROR
        return;
    }
    this->__inByte__(pix, data, isMask);
    this->writeByte++;
    if (this->writeByte >= ACC_BYTE_BOT){
        this->writeChan++;
        this->writeByte = 0;
    }
}

void Encoder::outFrame(cv::Mat& out){
/*
    // BUG HERE
    cv::Mat anchorMat(CODEMAT_ANCHOR_SIZE, CODEMAT_ANCHOR_SIZE, CV_8UC3);
                            // a white part to make up the part of Anchor
    cv::Mat QRcode;         // concat the anchorMat * 4 with the codeMat
    cv::Mat tmpMat;         // temp
    
    cv::hconcat(anchorMat, this->codeMatTop, tmpMat);
    cv::hconcat(tmpMat, anchorMat, out);                // topMat into QR code(save in out)
    cv::vconcat(out, this->codeMatMiddle, out);            // midMat into QR code
    cv::hconcat(anchorMat, this->codeMatBottom, tmpMat);
    cv::hconcat(tmpMat, anchorMat, tmpMat);
    cv::vconcat(out, tmpMat, out);                      // botMat into QR code
            // QR code is now done(save in out)

    cv::resize(out, QRcode, cv::Size(PIXEL_SIZE, PIXEL_SIZE), 0.0f, 0.0f, 0);
            // Extend to (512 * 512), & save QR code in 'QRcode'


    cv::resize(this->baseAnchor, anchorMat, cv::Size(BIG_ANCHOR_SIZE, BIG_ANCHOR_SIZE), 0.0f, 0.0f, 0);
            // Extend to big Anchor
    cv::Mat outRect = QRcode(cv::Rect(0, 0, anchorMat.cols, anchorMat.rows));
    anchorMat.copyTo(outRect);
            // Add the top-left Anchor
    outRect = QRcode(cv::Rect(0, PIXEL_SIZE - BIG_ANCHOR_SIZE, anchorMat.cols, anchorMat.rows));
    anchorMat.copyTo(outRect);
            // Add the bottom-left Anchor
    outRect = QRcode(cv::Rect(PIXEL_SIZE - BIG_ANCHOR_SIZE, 0, anchorMat.cols, anchorMat.rows));
    anchorMat.copyTo(outRect);
            // Add the top-right Anchor

    cv::resize(this->baseAnchor, anchorMat, cv::Size(SML_ANCHOR_SIZE, SML_ANCHOR_SIZE), 0.0f, 0.0f, 0);

    outRect = QRcode(cv::Rect(PIXEL_SIZE - SML_ANCHOR_SIZE, PIXEL_SIZE - SML_ANCHOR_SIZE,
      anchorMat.cols, anchorMat.rows));
    anchorMat.copyTo(outRect);
            // Add the bottom-right Anchor
    
    out = cv::Mat(this->outSize, CV_8UC1, cv::Scalar(PIX_BIT_ONE));
            // set out as white backgroud
    outRect = out(cv::Rect((this->outSize.width - PIXEL_SIZE)/2, (this->outSize.height - PIXEL_SIZE)/2,
      QRcode.cols, QRcode.rows));
    QRcode.copyTo(outRect);
            // add QRcode into white backgroud
*/
    this->fixAnchor(out);
    cv::resize(out, out, cv::Size(PIXEL_SIZE, PIXEL_SIZE), 0.0f, 0.0f, 0);
        // Extend to (512 * 512)
    
    this->setAnchor(out);
    cv::imshow("test", out);
    cv::waitKey();
}