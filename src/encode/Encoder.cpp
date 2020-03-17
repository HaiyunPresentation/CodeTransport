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

void Encoder::setStatus(cv::Mat& QRmat, size_t nFrame){
    cv::Vec3b baseColor;
    for (size_t i = 0; i < N_CHANNEL; i++){
        baseColor[i] = this->writeChan > i ? PIX_BIT_ONE : PIX_BIT_ZRO;
    }
    cv::Mat statusMat(4, 4, CV_8UC3, cv::Scalar(baseColor));

    if (baseColor != cv::Vec3b(0,0,0)){
        byte row, col;
        if (this->writeByte < ACC_BYTE_TOP){
            row = this->writeByte / TOPBOT_BYTE;
            col = this->writeByte % TOPBOT_BYTE + 1;
        }
        else if (this->writeByte < ACC_BYTE_MID){
            row = (this->writeByte - ACC_BYTE_TOP) / MIDDLE_BYTE + 8;
            col = (this->writeByte - ACC_BYTE_TOP) % MIDDLE_BYTE;
        }
        else if (this->writeByte < ACC_BYTE_BOT){
            row = (this->writeByte - ACC_BYTE_MID) / TOPBOT_BYTE + 56;
            col = (this->writeByte - ACC_BYTE_MID) % TOPBOT_BYTE + 1;
        }

        col = 8*col;
        cv::Vec3b* pStatus = statusMat.ptr<cv::Vec3b>(0);        
        for (size_t i = 0; i < 8; i++, pStatus++){
            (*pStatus)[this->writeChan] = row & 0x80 ? PIX_BIT_ONE : PIX_BIT_ZRO;
            row <<= 1;
        }
        for (size_t i = 0; i < 8; i++, pStatus++){
            (*pStatus)[this->writeChan] = col & 0x80 ? PIX_BIT_ONE : PIX_BIT_ZRO;
            col <<= 1;
        }
    }

    statusMat.copyTo(QRmat(cv::Rect(
        QRmat.cols - CODEMAT_ANCHOR_SIZE + 4*((nFrame%3)%2),
        QRmat.rows - CODEMAT_ANCHOR_SIZE + 4*((nFrame%3)/2),
        statusMat.cols, statusMat.rows
    )));
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
        // remenber writen status
        size_t bytes = this->writeByte;
        size_t chans = this->writeChan;
        
        // EndFrame: add '0000 0000' until ACC_BYTE_BOT Bytes in this frame
        while(this->writeByte < FRAME_BYTES && this->writeChan < 3){
            this->addByte(0xFF, false);
        }

        this->writeByte = bytes;
        this->writeChan = chans;
        return;
    }
    else if (this->writeChan >= N_CHANNEL){
        // error
        return;
    }

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

void Encoder::outFrame(cv::Mat& out, size_t nFrame){
    cv::Mat QRcode;
    this->fixAnchor(QRcode);
    this->setStatus(QRcode, nFrame);
    cv::resize(QRcode, QRcode, cv::Size(PIXEL_SIZE, PIXEL_SIZE), 0.0f, 0.0f, 0);
        // Extend to (PIXEL_SIZE * PIXEL_SIZE)
    
    this->setAnchor(QRcode);
    cv::imshow("test", QRcode);
    cv::waitKey();
    out = cv::Mat(OUT_FRAME_SIZE, OUT_FRAME_SIZE, CV_8UC3, cv::Scalar(255, 255, 255));
    QRcode.copyTo(out(cv::Rect(
        (OUT_FRAME_SIZE - PIXEL_SIZE)/2, (OUT_FRAME_SIZE - PIXEL_SIZE)/2, PIXEL_SIZE, PIXEL_SIZE)
    ));

}