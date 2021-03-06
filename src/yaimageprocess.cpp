#include "yaimageprocess.h"
#include "yastereocam.h"
#include "yastereotest.h"
#include "op/yaimgprocop.h"
#include "op/yaipocanny.h"
#include "op/yaipochessboard.h"

#include <QDebug>
#include <QImage>
#include <QLabel>
#include <opencv2/opencv.hpp>

YaImageProcess::YaImageProcess(QObject *parent) : QObject(parent)
{
    qInfo() << __PRETTY_FUNCTION__;
    _cam = new YaStereoCam(this);
    _test = new YaStereoTest(this);
    op = OP_OP1;
    src = SRC_CAM;
    srcScale = 1;
    _imgL = new cv::Mat;
    _imgR = new cv::Mat;
    _imgOutL = new cv::Mat;
    _imgOutR = new cv::Mat;
    countImgPtL = 0;
    countImgPtR = 0;
    srcTestChanged = true;
    _ipo = new YaImgProcOp(this);
    _ipoCanny = new YaIpoCanny(_ipo);
    _ipoChBoard = new YaIpoChessBoard(_ipo);
}

YaImageProcess::~YaImageProcess()
{
    qInfo() << __PRETTY_FUNCTION__;
    delete _imgL;
    delete _imgR;
    delete _imgOutL;
    delete _imgOutR;
}

void
YaImageProcess::getImages()
{
//    qInfo() << __PRETTY_FUNCTION__;
    if (src & SRC_CAM){
        _cam->capImages();
        _cam->getImageL(*_imgL);
        _cam->getImageR(*_imgR);

    } else {
        if(srcTestChanged){
            _test->setTestImageSize((YaStereoTest::TEST_IMAGE_SIZE) srcScale);
            _test->setTestImage((YaStereoTest::TEST_IMAGE) src);
            srcTestChanged = false;
        }
        _test->getImages();
        _test->getImageL(*_imgL);
        _test->getImageR(*_imgR);
    }
}

void
YaImageProcess::setOpImage(OPERATION operation)
{
    qInfo() << __PRETTY_FUNCTION__ << operation;
    op = operation;    
}

void
YaImageProcess::setSrcImageScale(int scale)
{
    qInfo() << __PRETTY_FUNCTION__ << scale;
    srcScale = scale;
    srcTestChanged = true;
}

void
YaImageProcess::setSrcImage(SOURCE source)
{
    qInfo() << __PRETTY_FUNCTION__ << source;
    src = source;
    srcTestChanged = true;
}

void
YaImageProcess::getImageL(QImage &img)
{
    getImage(img, true);
}

void
YaImageProcess::getImageR(QImage &img)
{
    getImage(img, false);
}

void
YaImageProcess::getImage(QImage &img, bool isLeft)
{
    //qInfo() << __PRETTY_FUNCTION__;
    int imgDepth = 0;
    int imgChannel = 0;
    QImage::Format format = QImage::Format_RGB888;

    if (isLeft){
        imgDepth = _imgOutL->depth();
        imgChannel = _imgOutL->channels();
    } else {
        imgDepth = _imgOutR->depth();
        imgChannel = _imgOutR->channels();
    }

    if(0 == imgDepth){
        switch (imgChannel) {
        case(1):{
            format = QImage::Format_Grayscale8;
            break;
        }
        case(2):{
            qWarning() << "Qt don't support 2 channel image: need 1 or 3";
            return;
        }
        case(3):{
            format = QImage::Format_RGB888;
            break;
        }
        case(4):{
            format = QImage::Format_RGBA8888;
            break;
        }
        default:{
            qInfo() << "Something wrong with cv:Mat channels";
            return;
        }
        }
    } else {
        qInfo() << "cv::Mat.depth" << _imgOutL->depth();
        qInfo() << "cv::Mat with more than 8U bit/channel not supported yet";
    }

    if (isLeft){
        QImage qimg(_imgOutL->ptr(), _imgOutL->cols, _imgOutL->rows,
                    _imgOutL->step, format);
        img = qimg;
    } else {
        QImage qimg(_imgOutR->ptr(), _imgOutR->cols, _imgOutR->rows,
                    _imgOutR->step, format);
        img = qimg;
    }
}
void
YaImageProcess::process()
{
    getImages();

    switch (op) {
    case OP_NOP:{ qInfo() << "OP_NOP"; break; }
    case OP_OP1:{ op1(); break; }
    case OP_OP2:{ op2(); break; }
    case OP_OP3:{ op3(); break; }
    case OP_OP4:{ op4(); break; }
    case OP_OP5:{ op5(); break; }
    case OP_OP6:{ op6(); break; }

    default:{
        qInfo() << "Operation not implemented";
        break;
    }
    }

    emit imageLReady();
    emit imageRReady();
}

void
YaImageProcess::op1()
{
    qInfo() << __PRETTY_FUNCTION__ << "BGR 2 RGB";
    cv::cvtColor(*_imgL, *_imgOutL, cv::COLOR_BGR2RGB);
    cv::cvtColor(*_imgR, *_imgOutR, cv::COLOR_BGR2RGB);
}

void
YaImageProcess::op2()
{
    qInfo() << __PRETTY_FUNCTION__ << "BGR 2 HLS/Gray";
    cv::Mat tmpR;
    cv::cvtColor(*_imgR, *_imgOutR, cv::COLOR_BGR2HLS);
    cv::cvtColor(*_imgL, *_imgOutL, cv::COLOR_BGR2GRAY);

}

void
YaImageProcess::op3()
{
    QMap<QString, QVariant> p;

    p["lowThresholdL"] = 400;
    p["ratioL"]        = 2.5;
    p["kernelSizeL"]   = 5;

    p["lowThresholdR"] = 200;
    p["ratioR"]        = 10.0;
    p["kernelSizeR"]   = 5;

    p["dumpParamsToCon"] = false;

    _ipo = _ipoCanny;
    _ipoCanny->setParams(p);
    _ipoCanny->process(*_imgL, *_imgR, *_imgOutL, *_imgOutR);
}

void
YaImageProcess::op4()
{
    QMap<QString, QVariant> p;
    p["sizeXL"] = 4;
    p["sizeYL"] = 6;
    p["sizeXR"] = 4;
    p["sizeYR"] = 6;
    p["dumpParamsToCon"] = false;

    _ipo = _ipoChBoard;
    _ipoChBoard->setParams(p);
    _ipoChBoard->process(*_imgL, *_imgR, *_imgOutL, *_imgOutR);
}

void
YaImageProcess::op5()
{
    qInfo() << __PRETTY_FUNCTION__ << "Camera Calibration";

    cv::Mat grayL, grayR;
    std::vector<cv::Point2f>  outPointsCornerL, outPointsCornerR;

    cv::cvtColor(*_imgL, grayL, cv::COLOR_BGR2GRAY);
    cv::cvtColor(*_imgR, grayR, cv::COLOR_BGR2GRAY);

    cv::cvtColor(*_imgL, *_imgOutL, cv::COLOR_BGR2RGB);
    cv::cvtColor(*_imgR, *_imgOutR, cv::COLOR_BGR2RGB);
    //CALIB_CB_FAST_CHECK for FishEye cams!;
    if (cv::findChessboardCorners(*_imgL, cv::Size(4,6), outPointsCornerL))
    {
        cv::cornerSubPix(grayL, outPointsCornerL, cv::Size(7,7),
                         cv::Size(-1,-1),
                         cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.0001));
        cv::drawChessboardCorners(*_imgOutL, cv::Size(4,6), outPointsCornerL, true);
        imgPtL.push_back(outPointsCornerL);

    } else {
        qInfo() << "Left: can't find check board";
    }

    if (cv::findChessboardCorners(*_imgR, cv::Size(4,6), outPointsCornerR))
    {
        cv::drawChessboardCorners(*_imgOutR, cv::Size(4,6), outPointsCornerR, true);
    } else {
        qInfo() << "Right: can't find check board";
    }
    //std::cout << "camL" << outPointsCornerL << std::endl;
    //std::cout << "camR" << outPointsCornerR << std::endl;
}

void
YaImageProcess::op6()
{
    qInfo() << __PRETTY_FUNCTION__;
    qInfo() << "No Op6 for now";
}

void
YaImageProcess::setCtrlIpoUI(QStackedWidget &sw)
{
    qInfo() << __PRETTY_FUNCTION__;    
    _ipo->setCtrlIpoUI(sw);
}
