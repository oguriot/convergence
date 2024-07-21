#ifndef CONFLUENTI_H
#define CONFLUENTI_H

#include <QMainWindow>
#include <QFile>
#include <QInputDialog>
#include <QStringList>
#include <QAction>
#include <QMovie>
#include <QImage>
#include <QTimer>
#include <QStandardItemModel>


#include <QtSql>
#include <QSqlDatabase>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class ConfluentI; }
QT_END_NAMESPACE

class ConfluentI : public QMainWindow
{
    Q_OBJECT
    
public:
    ConfluentI(QWidget *parent = nullptr);
    ~ConfluentI();

private slots:
    void m_handleLoginFile();

    void m_reorganizeCascade();
    void m_reorganizeTile();

    void m_handleSubControl();
    void m_handleSubScan();
    void m_handleSubInsert();
    void m_handleSubLog();
    void m_handleSubSearch();

    void m_reorganizeCustom();

    void m_captureImageFromCamera();
    void m_captureFingerPrint();

    void m_connectToDb();
    void m_disConnectToDb();
    void m_clear();
    QString m_currentDateAndTime();

    void m_log(QString textToLog);

    void m_handleScanStuff();

    void m_scanKnown(QString scanOptions);
    void m_startScanning();
    void m_stopScanning();

    void m_faceOnlyBrowseSrc();
    void m_faceOnly();

    QImage m_Mat2QImage(cv::Mat Mat2Convert);
    QImage m_Mat2QImageGray(cv::Mat Mat2ConvertToGray);

    void m_sqlQueryInsert();
    void m_sqlQuerySelect();

    void closeEvent(QCloseEvent* ev);

    void m_hasher(QString dataStringToHash);

//protected:
//    void mousePressEvent(QMouseEvent *event);
//    void dragEnterEvent(QDragEnterEvent *event);
//    void dropEvent(QDropEvent *event);
private:
    QMovie* att_LabInsertFingerprintMovie;
    QMovie* att_LabInsertFacialRecLineMovie;
    QString dateAndTime;

    QPixmap* att_PixImage;

    QImage  att_QImage;
    cv::Mat* att_MatCropped;

    QImage att_QImageForFaceOnly;
    QPixmap* att_PixImageForFaceOnly;
    cv::Mat* att_MatCroppedFaceOnly;

    QTimer* att_GrabKnownFacesTimer;

    QSqlDatabase dataBase;
    QStandardItemModel *model;

    QAction* att_ActionCascadeSubWindows;
    QAction* att_ActionTileSubWindows;
    QAction* att_ActionCustomizeSubWindows;

    QAction* att_ActionToolBarSubControl;
    QAction* att_ActionToolBarSubScan;
    QAction* att_ActionToolBarSubLog;
    QAction* att_ActionToolBarSubInsert;
    QAction* att_ActionToolBarSubSearch;

    QStringList* att_LoginList;
    QInputDialog* att_LoginDialog;
    QFile* att_LoginFile;
    Ui::ConfluentI *ui;
};

#endif // CONFLUENTI_H
