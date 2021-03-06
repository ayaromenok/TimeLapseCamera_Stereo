#ifndef YASTEREOWIDGET_H
#define YASTEREOWIDGET_H

#include <QWidget>
class QLayout;
class QVBoxLayout;
class QGroupBox;
class QLabel;
class QPushButton;
class QImage;
class QTimer;
class QComboBox;
class QSettings;
class QStackedWidget;
class YaImageProcess;

class YaStereoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit YaStereoWidget(QWidget *parent = nullptr);
    ~YaStereoWidget();

signals:

public slots:
    void timerUpdate();
    void updateSource(int source);
    void updateSourceScale(int scale);
    void updateTimerInterval(int index);
    void updateProcessOp(int index);
    void getImageL();
    void getImageR();

private:
    void setWindowSize();
    void setUI();

    QLayout         *_loutMain;
    QGroupBox           *_gbImgL;
    QVBoxLayout             *_loutImgL;
    QImage                      *_imgL;
    QLabel                      *_lbImgL;
    QGroupBox           *_gbImgR;
    QVBoxLayout             *_loutImgR;
    QImage                      *_imgR;
    QLabel                      *_lbImgR;
    QGroupBox           *_gbCtrl;
    QVBoxLayout             *_loutCtrl;
    QImage                      *_imgCtrl;
    QPushButton                 *_pbCtrlQuit;
    QPushButton                 *_pbCtrlProcess;
    QComboBox                   *_cbCtrlSource;
    QComboBox                   *_cbCtrlSourceScale;
    QComboBox                   *_cbCtrlTimer;
    QComboBox                   *_cbCtrlProcessOp;
    QLabel                      *_lbCtrlImage;
    QGroupBox                   *_gbCtrlIpO;
    QVBoxLayout                 *_loutCtrlIpO;
    QStackedWidget              *_swCtrlIpO;

    QTimer          *_timer;
    QSettings       *_settings;
    YaImageProcess  *_imp;
};

#endif // YASTEREOWIDGET_H
