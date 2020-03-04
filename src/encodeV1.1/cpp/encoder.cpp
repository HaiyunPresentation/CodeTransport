#include "include/encoder.h"

encoder::encoder():
  codeMatTop(CODEMAT_T_SIZE, LOCATION_SIZE, CV_8UC1),
  codeMatMid(CODEMAT_M_SIZE, CODEMAT_T_SIZE, CV_8UC1),
  codeMatBot(CODEMAT_T_SIZE, LOCATION_SIZE, CV_8UC1){
    this->writeByte = 0;
}

void encoder::resetCounter(){
    this->writeByte = 0;
}

void encoder::addByte(byte data, bool isEOF){
    if (isEOF){
        // EndFrame

        return;
    }

    // data is OK
    auto pos = this->codeMatTop.data;
    if (this->writeByte < CUMU_BYTE_TOP){
        pos += this->writeByte;
    }
    else if (this->writeByte < CUMU_BYTE_MID){
        pos += (this->writeByte - CUMU_BYTE_BOT);
    }
    else if (this->writeByte < CUMU_BYTE_BOT){
        pos += (this->writeByte - CUMU_BYTE_MID);
    }
    else{
        // ERROR
        return;
    }

    for (size_t i = 0U; i < 8U; i++){
        *(pos + i) = data & 0x80 ? BLACK_BIT : WHITE_BIT;
        data <<= 1;
    }
    this->writeByte++;
}

const cv::Mat& encoder::outFrame(){
    cv::Mat locationMat = cv::Mat::zeros(LOCATION_SIZE, LOCATION_SIZE, CV_8UC1);
    cv::Mat concatMat;
    cv::Mat tmpMat;
    cv::hconcat(locationMat, this->codeMatTop, tmpMat);
    cv::hconcat(tmpMat, locationMat, concatMat);            // topMat into concatMat
    cv::vconcat(concatMat, this->codeMatMid, concatMat);    // midMat into concatMat
    cv::hconcat(locationMat, this->codeMatBot, tmpMat);
    cv::hconcat(tmpMat, locationMat, tmpMat);
    cv::vconcat(concatMat, tmpMat, concatMat);              // botMat into concatMat
    
    cv::Mat out(PIXEL_ROWS, PIXEL_ROWS, CV_8UC1);
    cv::resize(concatMat, out, out.size(), 0.0f, 0.0f, 0);  // Extend to (512 * 512)

    return out;
}