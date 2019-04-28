#ifndef YAIPOCHESSBOARD_H
#define YAIPOCHESSBOARD_H

#include "yaimgprocop.h"

#include <opencv2/core/base.hpp>

class YaIpoChessBoard : public YaImgProcOp
{
public:
    YaIpoChessBoard();
    ~YaIpoChessBoard();

public slots:
   void process(cv::Mat &imgL, cv::Mat &imgR, cv::Mat &imgOutL, cv::Mat &imgOutR);

private:

};

#endif // YAIPOCHESSBOARD_H
