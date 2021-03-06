#include "yastereowidget.h"

#include <QDebug>
#include <QtWidgets>

#include "yastereocam.h"
#include "yaimageprocess.h"

YaStereoWidget::YaStereoWidget(QWidget *parent) : QWidget(parent)
{
    qInfo() << __PRETTY_FUNCTION__;

    _settings = new QSettings(this);
    _timer = new QTimer(this);
    _imp = new YaImageProcess(this);

    setWindowSize();
    setUI();

    connect(_timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    connect(_imp, SIGNAL(imageLReady()), this, SLOT(getImageL()));
    connect(_imp, SIGNAL(imageRReady()), this, SLOT(getImageR()));

    updateTimerInterval(_cbCtrlTimer->currentIndex());
    updateProcessOp(_cbCtrlProcessOp->currentIndex());
    updateSource(_cbCtrlSource->currentIndex());
    updateSourceScale(_cbCtrlSourceScale->currentIndex());
}

YaStereoWidget::~YaStereoWidget()
{
    qInfo() << __PRETTY_FUNCTION__;
    _timer->stop();
    // since all classes are QObject delieverd and use this as a parent,
    // they will be deleted automatically
}

void
YaStereoWidget::getImageL()
{
    //qInfo() << __PRETTY_FUNCTION__;
    QImage imgL;
    _imp->getImageL(imgL);
    _lbImgL->setPixmap(QPixmap::fromImage(imgL).scaled(this->width()/3,this->width()/4));
}

void
YaStereoWidget::getImageR()
{
    //qInfo() << __PRETTY_FUNCTION__;
    QImage imgR;
    _imp->getImageR(imgR);
    _lbImgR->setPixmap(QPixmap::fromImage(imgR).scaled(this->width()/3,this->width()/4));
}

void
YaStereoWidget::timerUpdate()
{
//    qInfo() << "timer:" << QDateTime::currentSecsSinceEpoch();
    _imp->process();
}

void
YaStereoWidget::updateSource(int source)
{
    qInfo() << __PRETTY_FUNCTION__ << source;
    _imp->setSrcImage((YaImageProcess::SOURCE)(1<<source));
    _settings->setValue("UI/source",source);
    timerUpdate();
}

void
YaStereoWidget::updateSourceScale(int scale)
{
    qInfo() << __PRETTY_FUNCTION__ << scale;
    _imp->setSrcImageScale(1<<scale);
    _settings->setValue("UI/sourceScale", scale);
    timerUpdate();
}

void
YaStereoWidget::updateTimerInterval(int index)
{
    _timer->stop();
    if (index > 0){
        _timer->setInterval(index*1000);
        _timer->start();
    }
    _settings->setValue("UI/timerInterval",index);
}

void
YaStereoWidget::updateProcessOp(int index)
{
    _imp->setOpImage((YaImageProcess::OPERATION)(1<<index));
    _imp->process();
    _imp->setCtrlIpoUI(*_swCtrlIpO);
    _settings->setValue("UI/ProcessOp",index);
    //timerUpdate();
}

void
YaStereoWidget::setWindowSize()
{
    qInfo() << __PRETTY_FUNCTION__;
#ifdef DEBUG_RPI_V7L
    this->setGeometry(QGuiApplication::primaryScreen()->geometry());
#endif //DEBUG_RPI_V7L

#ifdef DEBUG_PC
    this->setGeometry(0, 0,1280,720);
#endif //DEBUG_PC
}

void
YaStereoWidget::setUI()
{
    qInfo() << __PRETTY_FUNCTION__;
    _loutMain = new QHBoxLayout;

    _gbImgL = new QGroupBox(tr("Image/Left"));
    _lbImgL = new QLabel("Image/Left");
    _loutImgL = new QVBoxLayout();
    _loutImgL->addWidget(_lbImgL);
    _gbImgL->setLayout(_loutImgL);

    _gbImgR = new QGroupBox(tr("Image/Right"));
    _lbImgR = new QLabel("Image/Right");
    _loutImgR = new QVBoxLayout();
    _loutImgR->addWidget(_lbImgR);
    _gbImgR->setLayout(_loutImgR);

    _gbCtrl = new QGroupBox(tr("Controls"));    
    _loutCtrl = new QVBoxLayout();
    _pbCtrlQuit = new QPushButton(tr("E&xit"), this);
    connect(_pbCtrlQuit, SIGNAL(clicked()), this, SLOT(close()));    
    _loutCtrl->addWidget(_pbCtrlQuit);
    _pbCtrlProcess = new QPushButton(tr("&Process"), this);
    connect(_pbCtrlProcess, SIGNAL(clicked()), this, SLOT(timerUpdate()));
    _loutCtrl->addWidget(_pbCtrlProcess);

    _cbCtrlSource = new QComboBox();
    _cbCtrlSource->addItems(QStringList() << "Camera Source" << "Test: Balkony"
                            << "Test: Checked Board" );
    connect(_cbCtrlSource, QOverload<int>::of(&QComboBox::activated),
          this, &YaStereoWidget::updateSource);    
    _cbCtrlSource->setCurrentIndex(_settings->value("UI/source").toInt());
    _loutCtrl->addWidget(_cbCtrlSource);

    _cbCtrlSourceScale = new QComboBox();
    _cbCtrlSourceScale->addItems(QStringList() << "No Scale" << "Scale 1/2"
                                 << "Scale 1/4" << "Scale 2x" << "Scale 160x120 px"
                                 << "Scale 320x240 px" << "Scale 640x480 px"
                                 << "Scale 1280x960 px");
    connect(_cbCtrlSourceScale, QOverload<int>::of(&QComboBox::activated),
          this, &YaStereoWidget::updateSourceScale);
    _cbCtrlSourceScale->setCurrentIndex(_settings->value("UI/sourceScale").toInt());
    _loutCtrl->addWidget(_cbCtrlSourceScale);

    _cbCtrlTimer = new QComboBox();
    _cbCtrlTimer->addItems(QStringList() << "Timer stopped" << "1 sec" << "2 sec"
                           << "3 sec" << "4 sec" << "5 sec");    
    connect(_cbCtrlTimer, QOverload<int>::of(&QComboBox::activated),
          this, &YaStereoWidget::updateTimerInterval);
    _cbCtrlTimer->setCurrentIndex(_settings->value("UI/timerInterval").toInt());
    _loutCtrl->addWidget(_cbCtrlTimer);

    _cbCtrlProcessOp = new QComboBox();
    _cbCtrlProcessOp->addItems(QStringList() << "BGR 2 RGB" << "BGR 2 HLS/Gray"
                               << "Canny filter" << "Checked Board"
                               << "Camera Calibration" << "Op #6");
    connect(_cbCtrlProcessOp, QOverload<int>::of(&QComboBox::activated),
          this, &YaStereoWidget::updateProcessOp);
    _cbCtrlProcessOp->setCurrentIndex(_settings->value("UI/ProcessOp").toInt());
    _loutCtrl->addWidget(_cbCtrlProcessOp);

    _gbCtrlIpO = new QGroupBox("Controls for IpO");
    _swCtrlIpO = new QStackedWidget();
    _loutCtrlIpO = new QVBoxLayout();
    _loutCtrlIpO->addWidget(_swCtrlIpO);
    _gbCtrlIpO->setLayout(_loutCtrlIpO);
//QStackedWidget/QStackedLayout or use QTabWidget
    _loutCtrl->addWidget(_gbCtrlIpO);

    _lbCtrlImage = new QLabel("Place for Control Image");
    _loutCtrl->addWidget(_lbCtrlImage);
    _gbCtrl->setLayout(_loutCtrl);

    _loutMain->addWidget(_gbImgL);
    _loutMain->addWidget(_gbImgR);
    _loutMain->addWidget(_gbCtrl);

    setLayout(_loutMain);
}
