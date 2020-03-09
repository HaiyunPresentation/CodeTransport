#include "Encoder.h"

Encoder::Encoder():
  codeMatTop(CODEMAT_ANCHOR_SIZE, CODEMAT_TOPBOT_SIZE, CV_8UC1),
  codeMatMiddle(CODEMAT_TOPBOT_SIZE, CODEMAT_MIDDLE_SIZE, CV_8UC1),
  codeMatBottom(CODEMAT_ANCHOR_SIZE, CODEMAT_TOPBOT_SIZE, CV_8UC1),
  baseAnchor(cv::Mat::zeros(BASE_ANCHOR_SIZE, BASE_ANCHOR_SIZE, CV_8UC1)),
  outSize(cv::Size(OUT_FRAME_SIZE, OUT_FRAME_SIZE)){
    this->writeByte = 0;

    for (size_t bAnchorRow = 1; bAnchorRow < BASE_ANCHOR_SIZE - 1; bAnchorRow++){
        // make Anchor like "回"

        uchar* pDraw = this->baseAnchor.data + bAnchorRow * BASE_ANCHOR_SIZE;

        if (bAnchorRow == 1 || bAnchorRow == BASE_ANCHOR_SIZE - 2){
            for (size_t bAnchorCol = 1; bAnchorCol < BASE_ANCHOR_SIZE - 1; bAnchorCol++)
                *(pDraw + bAnchorCol) = WHITE_PIX;
        }
        else{
            *(pDraw + 1) = *(pDraw + BASE_ANCHOR_SIZE -2) = WHITE_PIX;
        }
    }
}

void Encoder::resetCounter(){
    this->writeByte = 0;
}

void Encoder::setOutSize(cv::Size outSize){
    this->outSize=outSize;
}

void Encoder::setOutSize(size_t size_row=OUT_FRAME_SIZE, size_t size_col=OUT_FRAME_SIZE){
    this->setOutSize(cv::Size(size_row, size_col));
}

void Encoder::addByte(byte data, bool isEOF){
    if (isEOF){
        // EndFrame: add '0000 0000' until 120B in this frame
        while(this->writeByte < CUMU_BYTE_BOT){
            this->addByte(0x00, false);
        }

        return;
    }

    // data is OK
    uchar* pos;         // position of insert
    bool isMaskBlack;   // prepare for Mask, is the byte's first bit meets a black

    if (this->writeByte < CUMU_BYTE_TOP){
        pos = this->codeMatTop.data + 8 * (this->writeByte);
        isMaskBlack = (this->writeByte / 3U) % 2;
    }
    else if (this->writeByte < CUMU_BYTE_MID){
        pos = this->codeMatMiddle.data + 8 * (this->writeByte - CUMU_BYTE_TOP);
        isMaskBlack = ((this->writeByte - CUMU_BYTE_TOP) / 4U) % 2;
    }
    else if (this->writeByte < CUMU_BYTE_BOT){
        pos = this->codeMatBottom.data + 8 * (this->writeByte - CUMU_BYTE_MID);
        isMaskBlack = ((this->writeByte - CUMU_BYTE_MID) / 3U) % 2;
    }
    else{
        // ERROR
        return;
    }

    for (size_t i = 0U; i < 8U; i++){
        // data & 0x80 => is bit(1)[ought to be black]
        *(pos + i) = (bool(data & 0x80) xor isMaskBlack) ? BLACK_PIX : WHITE_PIX;
        data <<= 1;
        isMaskBlack = !isMaskBlack;     // the next bit mask ought to chuange
    }
    this->writeByte++;
}

void Encoder::outFrame(cv::Mat& out){
    cv::Mat anchorMat(CODEMAT_ANCHOR_SIZE, CODEMAT_ANCHOR_SIZE, CV_8UC1, cv::Scalar(WHITE_PIX));
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
    
    out = cv::Mat(this->outSize, CV_8UC1, cv::Scalar(WHITE_PIX));
            // set out as white backgroud
    outRect = out(cv::Rect((this->outSize.width - PIXEL_SIZE)/2, (this->outSize.height - PIXEL_SIZE)/2,
      QRcode.cols, QRcode.rows));
    QRcode.copyTo(outRect);
            // add QRcode into white backgroud
}