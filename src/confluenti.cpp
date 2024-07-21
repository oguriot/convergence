#include "confluenti.h"
#include "ui_confluenti.h"

#include <QInputDialog>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QString>
#include <QList>
#include <QStringList>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QDateTime>
#include <QAction>
#include <QEvent>
#include <QCloseEvent>
#include <QDir>
#include <QDirIterator>
#include <QPixmap>
#include <QPainter>
#include <QDrag>
#include <QMimeData>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QAction>

#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <opencv2/opencv.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/core/types_c.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>

#include <opencv2/video.hpp>
#include <opencv2/videoio/videoio.hpp>


int faceCurrentPos(0), fingerCurrentPos(0);
int rowPos(1);
int faceWidth(1), faceHeight(1);

using namespace cv;

ConfluentI::ConfluentI(QWidget *parent) : QMainWindow(parent), ui(new Ui::ConfluentI)
{
    ui->setupUi(this);

//    dataBase = QSqlDatabase::addDatabase("QPSQL");
    dataBase = QSqlDatabase::addDatabase("QSQLITE");

    ui->sub_scan->setEnabled(false);
    ui->sub_insert->setEnabled(false);
    ui->sub_search->setEnabled(false);
    ui->sub_faceonly->setEnabled(false);

    att_PixImage = new QPixmap();
    att_PixImageForFaceOnly = new QPixmap();

    att_GrabKnownFacesTimer = new QTimer();

    //ui->sub_control->showMinimized();
    ui->sub_scan->showMinimized();
    ui->sub_insert->showMinimized();
    ui->sub_log->showMinimized();
    ui->sub_search->showMinimized();
    ui->sub_table->showMinimized();

    ui->mdiArea->setWindowIcon(QIcon(":/qrc_icons/icons/EkipaR_logo.jpg"));

    att_ActionCascadeSubWindows = new QAction(QIcon(":/qrc_icons/icons/MAIA_icon_Cascade.png"), tr("&Cascade"));
    att_ActionCascadeSubWindows->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_C);

    att_ActionTileSubWindows = new QAction(QIcon(":/qrc_icons/icons/MAIA_icon_Tile.png"), tr("&Tile"));
    att_ActionTileSubWindows->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_T);

    att_ActionCustomizeSubWindows = new QAction(QIcon(":/qrc_icons/icons/Send.png"), tr("C&ustomised"));
    att_ActionCustomizeSubWindows->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_U);

    att_ActionToolBarSubControl = new QAction(QIcon(":/qrc_icons/icons/Control.png"), tr(""), this);
    att_ActionToolBarSubControl->setIconText(tr("&Control"));
    connect(att_ActionToolBarSubControl, SIGNAL(triggered()), ui->sub_control, SLOT(setFocus()));

    att_ActionToolBarSubScan = new QAction(QIcon(":/qrc_icons/icons/Facial Rec.png"), tr(""), this);
    att_ActionToolBarSubScan->setIconText(tr("&Scan."));
    connect(att_ActionToolBarSubScan, SIGNAL(triggered()), ui->sub_scan, SLOT(setFocus()));

    att_ActionToolBarSubLog = new QAction(QIcon(":/qrc_icons/icons/Log.png"), tr(""), this);
    att_ActionToolBarSubLog->setIconText(tr("&Log"));
    connect(att_ActionToolBarSubLog, SIGNAL(triggered()), ui->sub_log, SLOT(setFocus()));

    att_ActionToolBarSubInsert = new QAction(QIcon(":/qrc_icons/icons/Insert.png"), tr(""), this);
    att_ActionToolBarSubInsert->setIconText(tr("&Insert"));
    att_ActionToolBarSubInsert->setEnabled(false);//deactivate action for a deactivated sub window
    connect(att_ActionToolBarSubInsert, SIGNAL(triggered()), ui->sub_insert, SLOT(setFocus()));

    att_ActionToolBarSubSearch = new QAction(QIcon(":/qrc_icons/icons/Search.png"), tr(""), this);
    att_ActionToolBarSubSearch->setIconText(tr("&Search"));
    att_ActionToolBarSubSearch->setEnabled(false);//deactivate action for a deactivated sub window
    connect(att_ActionToolBarSubSearch, SIGNAL(triggered()), ui->sub_search, SLOT(setFocus()));

    ui->mainToolBar->addAction(att_ActionToolBarSubControl);
    ui->mainToolBar->addAction(att_ActionToolBarSubScan);
    ui->mainToolBar->addAction(att_ActionToolBarSubInsert);
    ui->mainToolBar->addAction(att_ActionToolBarSubLog);
    ui->mainToolBar->addAction(att_ActionToolBarSubSearch);

    // i moved this declaration here for not to re-create the object
    // each time the methode m_log is called and then we got multi row log table
    model = new QStandardItemModel(this);

    connect(att_ActionCascadeSubWindows, SIGNAL(triggered()), this, SLOT(m_reorganizeCascade()));
    connect(att_ActionTileSubWindows, SIGNAL(triggered()), this, SLOT(m_reorganizeTile()));
    connect(att_ActionCustomizeSubWindows, SIGNAL(triggered()), this, SLOT(m_reorganizeCustom()));

    connect(att_ActionToolBarSubControl, SIGNAL(triggered()), this, SLOT(m_handleSubControl()));
    connect(att_ActionToolBarSubScan, SIGNAL(triggered()), this, SLOT(m_handleSubScan()));
    connect(att_ActionToolBarSubInsert, SIGNAL(triggered()), this, SLOT(m_handleSubInsert()));
    connect(att_ActionToolBarSubLog, SIGNAL(triggered()), this, SLOT(m_handleSubLog()));
    connect(att_ActionToolBarSubSearch, SIGNAL(triggered()), this, SLOT(m_handleSubSearch()));

    connect(ui->b_ctrl_connect, SIGNAL(clicked()), this, SLOT(m_connectToDb()));
    connect(ui->b_ctrl_disconnect, SIGNAL(clicked()), this, SLOT(m_disConnectToDb()));
    connect(ui->b_ctrl_clear, SIGNAL(clicked()), this, SLOT(m_clear()));
    connect(ui->b_ctrl_quit, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->b_insert_insert, SIGNAL(clicked()), this, SLOT(m_sqlQueryInsert()));
    connect(ui->b_ctrl_select, SIGNAL(clicked()), this, SLOT(m_sqlQuerySelect()));

    connect(att_GrabKnownFacesTimer, SIGNAL(timeout()), this, SLOT(m_handleScanStuff()));
    connect(ui->b_scan_launch, SIGNAL(clicked()), this, SLOT(m_startScanning()));

    connect(ui->b_scan_stop, SIGNAL(clicked()), this, SLOT(m_stopScanning()));

    connect(ui->b_insert_load_image, SIGNAL(clicked()), this, SLOT(m_captureImageFromCamera()));
    connect(ui->b_insert_load_fingerprint, SIGNAL(clicked()), this, SLOT(m_captureFingerPrint()));


    connect(ui->line_insert_region, SIGNAL(currentIndexChanged(QString)), this, SLOT(m_hasher(QString)));
    //connect(ui->b_scan_loadImageToScan, SIGNAL(clicked()), this, SLOT(m_grabImageForRecognition()));

    connect(ui->b_faceOncly_src, SIGNAL(clicked()), this, SLOT(m_faceOnlyBrowseSrc()));
    connect(ui->b_faceOnly_launch, SIGNAL(clicked()), this, SLOT(m_faceOnly()));
}
//#################################################
//void ConfluentI::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        QImage im("../others/subject_faces/29.jpg");
//        QMimeData* data = new QMimeData;
//        data->setImageData(im);

//        QDrag *drag = new QDrag(this);
//        drag->setMimeData(data);
//        drag->setPixmap(QPixmap(50, 50));
//        drag->start();
//    }
//}
////#################################################
//void ConfluentI::dragEnterEvent(QDragEnterEvent *event)
//{
//    ui->lab_scan_subject->setText("DRAG");
//    //ui->lab_scan_subject->setBackgroundRole(QPalette::Dark);
//    ui->lab_scan_subject->setAcceptDrops(true);
//    event->acceptProposedAction();
//}
////#################################################
//void ConfluentI::dropEvent(QDropEvent *event)
//{
//    if (event->mimeData()->hasImage())
//    {
//        ui->lab_scan_subject->setText("DROP");
//        //ui->lab_scan_subject->setText(event->mimeData()->text());

//        QImage image = qvariant_cast<QImage>(event->mimeData()->imageData());
//        QPixmap mapg;
//        mapg.fromImage(image);
//        ui->lab_scan_subject->setPixmap(mapg);

//        //ui->lab_scan_subject->setPixmap(qvariant_cast<QPixmap>(event->mimeData()->imageData()));
//        event->acceptProposedAction();
//    }
//}
//#################################################
void ConfluentI::m_log(QString textToLog)
{
    model->setColumnCount(6);
    model->setHeaderData(0, Qt::Horizontal, tr("Date & Time"));
    model->setHeaderData(1, Qt::Horizontal, tr("User"));
    model->setHeaderData(2, Qt::Horizontal, tr("Host"));
    model->setHeaderData(3, Qt::Horizontal, tr("Database"));
    model->setHeaderData(4, Qt::Horizontal, tr("Table"));
    model->setHeaderData(5, Qt::Horizontal, tr("Task"));

    QStandardItem *itemDateAndTime = new QStandardItem(this->m_currentDateAndTime());
    QStandardItem *itemUser = new QStandardItem(dataBase.userName());
    QStandardItem *itemHost = new QStandardItem(dataBase.hostName());
    QStandardItem *itemDatabase = new QStandardItem(dataBase.databaseName());
    QStandardItem *itemTable = new QStandardItem(ui->combo_ctrl_table_name->currentText());
    QStandardItem *itemTextToLog = new QStandardItem(textToLog);

    QList<QStandardItem*> itemList;
    itemList << itemDateAndTime << itemUser << itemHost << itemDatabase << itemTable << itemTextToLog;

    int c(model->rowCount());
    model->setRowCount(c++);
    model->appendRow(itemList);

    ui->tableView_log->setModel(model);
    ui->tableView_log->resizeColumnsToContents();
    rowPos++;
    ui->tableView_log->scrollToBottom();
}

//#################################################
void ConfluentI::m_captureImageFromCamera()
{
    //we need to get the Mat from Drop event on lab_facialrec_subject
    Mat frameBuffer = imread("../others/subject_faces/29.jpg");

    Mat gray;
    cvtColor(frameBuffer, gray, CV_BGR2GRAY);
    equalizeHist(gray, gray);

    // Face detect
    std::vector<Rect> vecFaces;

    CascadeClassifier face_cascade;
    String face_cascade_name = "/home/monge/STUFF/Qt/Neski/etc/haarcascades/haarcascade_profileface.xml";
    face_cascade.load(face_cascade_name);

    face_cascade.detectMultiScale(gray, vecFaces, 1.1, 2,1 | CASCADE_FIND_BIGGEST_OBJECT, Size(10,10));

    unsigned int i(0);
    for (i = 0; i < vecFaces.size(); i++)
    {
        //face detect rectangle
        Point upperLeftFace(vecFaces[i].x + 3, vecFaces[i].y + 3);
        Point lowerRightFace(vecFaces[i].x+vecFaces[i].width - 3, vecFaces[i].y+vecFaces[i].height - 3);

        //scale the rect alittle because if it has the same dimension than the face, we don't see it, it's confused with the label's border
        rectangle(/*Matrice*/frameBuffer, /*Point*/upperLeftFace, /*Point*/lowerRightFace, /*BGR Color*/Scalar(255, 255,0), /*Line height*/1, /*line type*/8);

        //dynamically create a cropped Mat from the frameBuffer Mat with the face dimensions
        att_MatCroppedFaceOnly = new Mat(frameBuffer, Rect(vecFaces[i].x, vecFaces[i].y, vecFaces[i].width, vecFaces[i].height));

    }
    //send the content of the pointer to the m_Mat2QImage methode
    att_QImage = m_Mat2QImage(*att_MatCroppedFaceOnly);
    *att_PixImage = QPixmap::fromImage(att_QImage.scaled(att_MatCroppedFaceOnly->cols, att_MatCroppedFaceOnly->rows), Qt::AutoColor);
    ui->lab_insert_image->setPixmap(*att_PixImage);
    ui->lab_scan_subject->setPixmap(*att_PixImage);

    frameBuffer.release();
    att_MatCroppedFaceOnly->release();

    this->m_log(tr("Image successfully captured."));
}
//#################################################
void ConfluentI::m_captureFingerPrint()
{
    //here, just hit the > button on the insert subwindow when the user is ready to capture
    Mat frameBuffer = imread("../others/Fingerprints_2.jpg");

    att_QImage = m_Mat2QImage(frameBuffer);
    *att_PixImage = QPixmap::fromImage(att_QImage.scaled(frameBuffer.cols, frameBuffer.rows), Qt::AutoColor);
    ui->lab_insert_fingerprint->setPixmap(*att_PixImage);
    frameBuffer.release();

    this->m_log(tr("Fingerprint successfully captured."));
}
//#################################################
void ConfluentI::m_handleScanStuff()
{
    if (ui->combo_scan_search->currentText() == "Faces")
    {
        this->m_scanKnown("faces");
    }
    else if (ui->combo_scan_search->currentText() == "Fingerprints")
    {
        this->m_scanKnown("fingerprints");
    }
}
//#################################################
void ConfluentI::m_startScanning()
{
    att_GrabKnownFacesTimer->start(100);
    ui->b_scan_launch->setEnabled(false);
    ui->combo_scan_search->setEnabled(false);
    ui->b_scan_stop->setEnabled(true);

    //can not disconnect, scan. ongoing
    ui->b_ctrl_disconnect->setEnabled(false);

    ui->b_ctrl_quit->setEnabled(false);//can not quit during a facial scanning process

    faceCurrentPos = 0;
    this->m_log(tr("Database scan, started."));
}
//#################################################
void ConfluentI::m_stopScanning()
{
    att_GrabKnownFacesTimer->stop();
    ui->progressBar->reset();
    ui->b_scan_launch->setEnabled(true);
    ui->combo_scan_search->setEnabled(true);
    ui->b_scan_stop->setEnabled(false);
    ui->lab_scan_database->clear();
    ui->label_scan_log->setText("Search log");

    ui->b_ctrl_disconnect->setEnabled(true);

    ui->lab_scan_status_current_file->clear();
    ui->lab_scan_status_current_region->clear();
    ui->lab_scan_status_current_subject->clear();

    this->m_log(tr("database scan, stopped."));

    faceCurrentPos = 1;
    fingerCurrentPos = 1;
    //ui->b_ctrl_quit->setEnabled(true);//can quit if facial scanning process terminated
}
//#################################################
void ConfluentI::m_scanKnown(QString scanOptions)
{
    QString pathToFaces("../others/db_scan/faces/Alaotra-Mangoro/");
    QString pathToFingerprints("../others/db_scan/fingerprints/Alaotra-Mangoro/");
    //**************************************************************************************************
    if (scanOptions == "faces")
    {
        QDir facesDir(pathToFaces);//point the face dir

        //just want the files list, not directories and sort them by name
        QStringList facesList = facesDir.entryList(QDir::Files);
        int facesTotal = facesList.size();//get the number of file in the face directory

        QFileInfoList facesListInfo = facesDir.entryInfoList(QDir::Files);//here we want the info about each file

        //be verbose about the scan process
        ui->lab_scan_status_current_file->setText(facesListInfo.at(faceCurrentPos).absoluteFilePath());
        //get an absolute dir and turn it into a dir name
        ui->lab_scan_status_current_region->setText(facesListInfo.at(faceCurrentPos).absoluteDir().dirName());
        ui->lab_scan_status_current_subject->setText(facesListInfo.at(faceCurrentPos).fileName());

        //tell the progress bar to re-adjust its maximum size
        ui->progressBar->setMaximum(facesTotal);

        QString pathToFacesMat(pathToFaces + facesList.at(faceCurrentPos));
        Mat frameBuffer = imread(pathToFacesMat.toLatin1().data());

        Mat gray;
        cvtColor(frameBuffer, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);

        // Face detect
        std::vector<Rect> vecFaces;

        CascadeClassifier face_cascade;
        String face_cascade_name = "/home/monge/STUFF/Qt/Neski/etc/haarcascades/haarcascade_profileface.xml";
        face_cascade.load(face_cascade_name);

        face_cascade.detectMultiScale(gray, vecFaces, 1.1, 2,1 | CASCADE_FIND_BIGGEST_OBJECT, Size(10,10));

        for (unsigned int i = 0; i < vecFaces.size(); i++)
        {
            //face detect rectangle
            Point upperLeftFace(vecFaces[i].x, vecFaces[i].y);
            Point lowerRightFace(vecFaces[i].x+vecFaces[i].width, vecFaces[i].y+vecFaces[i].height);
            rectangle(/*Matrice*/frameBuffer, /*Rect*/upperLeftFace, /*Rect*/lowerRightFace, /*BGR Color*/Scalar(255, 255,0), /*Line height*/1, /*line type*/8);

            CvPoint A1;
            A1.x = vecFaces[i].x;
            A1.y = vecFaces[i].y;
            //horizontal end line (up left)
            CvPoint A2;
            A2.x = vecFaces[i].x - 20;
            A2.y = vecFaces[i].y;
            //vertical start line (up left)
            CvPoint A3;
            A3.x = vecFaces[i].x;
            A3.y = vecFaces[i].y;
            //vertical end line (up left)
            CvPoint A4;
            A4.x = vecFaces[i].x;
            A4.y = vecFaces[i].y - 20;
            //...................................................
            //horizontal start line (down left)
            CvPoint A5;
            A5.x = vecFaces[i].x;
            A5.y = vecFaces[i].y + vecFaces[i].height;
            //horizontal end line (down left)
            CvPoint A6;
            A6.x = vecFaces[i].x;
            A6.y = vecFaces[i].y + vecFaces[i].height + 20;
            //vertical start line (down left)
            CvPoint A7;
            A7.x = vecFaces[i].x;
            A7.y = vecFaces[i].y + vecFaces[i].height;
            //vertical end line (down left)
            CvPoint A8;
            A8.x = vecFaces[i].x - 20;
            A8.y = vecFaces[i].y + vecFaces[i].height;
            //##################################################
            //horizontal start line (up right)
            CvPoint A9;
            A9.x = vecFaces[i].x + vecFaces[i].width;
            A9.y = vecFaces[i].y;
            //horizontal end line (up right)
            CvPoint A10;
            A10.x = vecFaces[i].x + vecFaces[i].width + 20;
            A10.y = vecFaces[i].y;
            //vertical start line (up right)
            CvPoint A11;
            A11.x = vecFaces[i].x + vecFaces[i].width;//X (abscisse) de rectt + longueur de rectt, rectt étant la face
            A11.y = vecFaces[i].y;
            //vertical end line (up right)
            CvPoint A12;
            A12.x = vecFaces[i].x + vecFaces[i].width;
            A12.y = vecFaces[i].y - 20;
            //...................................................
            //horizontal start line (down right)
            CvPoint A13;
            A13.x = vecFaces[i].x +vecFaces[i].width;
            A13.y = vecFaces[i].y + vecFaces[i].height;
            //horizontal end line (down right)
            CvPoint A14;
            A14.x = vecFaces[i].x + vecFaces[i].width + 20;
            A14.y = vecFaces[i].y + vecFaces[i].height;
            //vertical start line (down right)
            CvPoint A15;
            A15.x = vecFaces[i].x + vecFaces[i].width;
            A15.y = vecFaces[i].y + vecFaces[i].height;
            //vertical end line (down right)
            CvPoint A16;
            A16.x = vecFaces[i].x + vecFaces[i].width;
            A16.y = vecFaces[i].y + vecFaces[i].height + 20;
            //##################################################
            //        //middle vertic start
            //        CvPoint A17;
            //        A17.x = vecFaces[i].x + vecFaces[i].width/2;
            //        A17.y = vecFaces[i].y - 5;
            //        //middle vertic end
            //        CvPoint A18;
            //        A18.x = vecFaces[i].x + vecFaces[i].width/2;
            //        A18.y = vecFaces[i].y;

            line(frameBuffer, A1, A2, Scalar(255, 255, 0), 1, 8, 0);//up left horiz
            line(frameBuffer, A3, A4, Scalar(255, 255, 0), 1, 8, 0);//up left vertic
            line(frameBuffer, A5, A6, Scalar(255, 255, 0), 1, 8, 0);//down left horiz
            line(frameBuffer, A7, A8, Scalar(255, 255, 0), 1, 8, 0);//down left vertic

            line(frameBuffer, A9, A10, Scalar(255, 255, 0), 1, 8, 0);//up right horiz
            line(frameBuffer, A11, A12, Scalar(255, 255, 0), 1, 8, 0);//up right vertic
            line(frameBuffer, A13, A14, Scalar(255, 255, 0), 1, 8, 0);//down right horiz
            line(frameBuffer, A15, A16, Scalar(255, 255, 0), 1, 8, 0);//down right vertic

            //line(frameBuffer, A17, A18, Scalar(255, 255, 0), 1, 8, 0);//middle vertic
        }

        att_QImage = m_Mat2QImage(frameBuffer);
        *att_PixImage = QPixmap::fromImage(att_QImage.scaled(frameBuffer.cols, frameBuffer.rows, Qt::KeepAspectRatio), Qt::AutoColor);
        ui->lab_scan_database->setPixmap(*att_PixImage);
        frameBuffer.release();

        //go to the next face position in the faces list
        faceCurrentPos++;

        ui->label_scan_log->setText(tr("Scanning database... ") + QString::number(faceCurrentPos, 10) + " / " + QString::number(facesTotal, 10) + " items scanned.");
        ui->progressBar->setValue(faceCurrentPos + 1);

        //scan known faces folder
        if (faceCurrentPos == facesTotal)//when we get at the end of the list
        {
            //go to the first
            faceCurrentPos = 0;
        }

    }
    //**************************************************************************************************
    else if (scanOptions == "fingerprints")
    {
        QDir fingersDir(pathToFingerprints);//point the face dir
        QStringList fingersList = fingersDir.entryList(QDir::Files);//just want the files list, not directories
        int fingersTotal = fingersList.size();//get the number of file in the face directory

        QFileInfoList fingersListInfo = fingersDir.entryInfoList(QDir::Files);//here we want the info about each file

        //be verbose about the scan process
        ui->lab_scan_status_current_file->setText(fingersListInfo.at(fingerCurrentPos).absoluteFilePath());
        //get an absolute dir and turn it into a dir name
        ui->lab_scan_status_current_region->setText(fingersListInfo.at(fingerCurrentPos).absoluteDir().dirName());
        ui->lab_scan_status_current_subject->setText(fingersListInfo.at(fingerCurrentPos).fileName());


        //tell the progress bar to re-adjust its maximum size
        ui->progressBar->setMaximum(fingersTotal);

        QString pathToFingerprintsMat(pathToFingerprints + fingersList.at(fingerCurrentPos));
        Mat frameBuffer = imread(pathToFingerprintsMat.toLatin1().data());

        Mat gray;
        cvtColor(frameBuffer, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);

        // Face detect
        std::vector<Rect> vecFaces;

        CascadeClassifier face_cascade;
        String face_cascade_name = "/home/monge/STUFF/Qt/Neski/etc/haarcascades/haarcascade_profileface.xml";
        face_cascade.load(face_cascade_name);

        face_cascade.detectMultiScale(gray, vecFaces, 1.1, 2,1 | CASCADE_FIND_BIGGEST_OBJECT, Size(10,10));

        for (unsigned int i = 0; i < vecFaces.size(); i++)
        {
            //face detect rectangle
            Point upperLeftFace(vecFaces[i].x, vecFaces[i].y);
            Point lowerRightFace(vecFaces[i].x+vecFaces[i].width, vecFaces[i].y+vecFaces[i].height);
            rectangle(/*Matrice*/frameBuffer, /*Rect*/upperLeftFace, /*Rect*/lowerRightFace, /*BGR Color*/Scalar(255, 255,0), /*Line height*/1, /*line type*/8);

            CvPoint A1;
            A1.x = vecFaces[i].x;
            A1.y = vecFaces[i].y;
            //horizontal end line (up left)
            CvPoint A2;
            A2.x = vecFaces[i].x - 20;
            A2.y = vecFaces[i].y;
            //vertical start line (up left)
            CvPoint A3;
            A3.x = vecFaces[i].x;
            A3.y = vecFaces[i].y;
            //vertical end line (up left)
            CvPoint A4;
            A4.x = vecFaces[i].x;
            A4.y = vecFaces[i].y - 20;
            //...................................................
            //horizontal start line (down left)
            CvPoint A5;
            A5.x = vecFaces[i].x;
            A5.y = vecFaces[i].y + vecFaces[i].height;
            //horizontal end line (down left)
            CvPoint A6;
            A6.x = vecFaces[i].x;
            A6.y = vecFaces[i].y + vecFaces[i].height + 20;
            //vertical start line (down left)
            CvPoint A7;
            A7.x = vecFaces[i].x;
            A7.y = vecFaces[i].y + vecFaces[i].height;
            //vertical end line (down left)
            CvPoint A8;
            A8.x = vecFaces[i].x - 20;
            A8.y = vecFaces[i].y + vecFaces[i].height;
            //##################################################
            //horizontal start line (up right)
            CvPoint A9;
            A9.x = vecFaces[i].x + vecFaces[i].width;
            A9.y = vecFaces[i].y;
            //horizontal end line (up right)
            CvPoint A10;
            A10.x = vecFaces[i].x + vecFaces[i].width + 20;
            A10.y = vecFaces[i].y;
            //vertical start line (up right)
            CvPoint A11;
            A11.x = vecFaces[i].x + vecFaces[i].width;//X (abscisse) de rectt + longueur de rectt, rectt étant la face
            A11.y = vecFaces[i].y;
            //vertical end line (up right)
            CvPoint A12;
            A12.x = vecFaces[i].x + vecFaces[i].width;
            A12.y = vecFaces[i].y - 20;
            //...................................................
            //horizontal start line (down right)
            CvPoint A13;
            A13.x = vecFaces[i].x +vecFaces[i].width;
            A13.y = vecFaces[i].y + vecFaces[i].height;
            //horizontal end line (down right)
            CvPoint A14;
            A14.x = vecFaces[i].x + vecFaces[i].width + 20;
            A14.y = vecFaces[i].y + vecFaces[i].height;
            //vertical start line (down right)
            CvPoint A15;
            A15.x = vecFaces[i].x + vecFaces[i].width;
            A15.y = vecFaces[i].y + vecFaces[i].height;
            //vertical end line (down right)
            CvPoint A16;
            A16.x = vecFaces[i].x + vecFaces[i].width;
            A16.y = vecFaces[i].y + vecFaces[i].height + 20;
            //##################################################
            //        //middle vertic start
            //        CvPoint A17;
            //        A17.x = vecFaces[i].x + vecFaces[i].width/2;
            //        A17.y = vecFaces[i].y - 5;
            //        //middle vertic end
            //        CvPoint A18;
            //        A18.x = vecFaces[i].x + vecFaces[i].width/2;
            //        A18.y = vecFaces[i].y;

            line(frameBuffer, A1, A2, Scalar(255, 255, 0), 1, 8, 0);//up left horiz
            line(frameBuffer, A3, A4, Scalar(255, 255, 0), 1, 8, 0);//up left vertic
            line(frameBuffer, A5, A6, Scalar(255, 255, 0), 1, 8, 0);//down left horiz
            line(frameBuffer, A7, A8, Scalar(255, 255, 0), 1, 8, 0);//down left vertic

            line(frameBuffer, A9, A10, Scalar(255, 255, 0), 1, 8, 0);//up right horiz
            line(frameBuffer, A11, A12, Scalar(255, 255, 0), 1, 8, 0);//up right vertic
            line(frameBuffer, A13, A14, Scalar(255, 255, 0), 1, 8, 0);//down right horiz
            line(frameBuffer, A15, A16, Scalar(255, 255, 0), 1, 8, 0);//down right vertic

            //line(frameBuffer, A17, A18, Scalar(255, 255, 0), 1, 8, 0);//middle vertic
        }

        att_QImage = m_Mat2QImage(frameBuffer);
        *att_PixImage = QPixmap::fromImage(att_QImage.scaled(frameBuffer.cols, frameBuffer.rows, Qt::KeepAspectRatio), Qt::AutoColor);
        ui->lab_scan_database->setPixmap(*att_PixImage);
        frameBuffer.release();

        //go to the next face position in the faces list
        fingerCurrentPos++;

        ui->label_scan_log->setText(tr("Scanning database... ") + QString::number(fingerCurrentPos, 10) + " / " + QString::number(fingersTotal, 10) + " items scanned.");
        ui->progressBar->setValue(fingerCurrentPos+1);

        //scan known fingers folder
        if (fingerCurrentPos == fingersTotal)//when we get at the end of the faces list
        {
            //go to the first
            fingerCurrentPos = 0;
        }
    }
    //**************************************************************************************************
}
//#################################################
void ConfluentI::m_faceOnlyBrowseSrc()
{
    QFileDialog srcDialog;
    //QString srcPath = srcDialog.getExistingDirectory(this, tr("Select source directory"), QDir::homePath());
    QString srcPath = srcDialog.getExistingDirectory(this, tr("Select source directory"), "../others/faceonlySrc/");
    if (srcPath.isEmpty())
    {
        return;
    }
    // if the path from the browse dialog does not ends with "/", we, explicitly, add it
    if (!srcPath.endsWith("/"))
    {
        srcPath.append("/");
    }
    ui->line_faceOnly_path_src->setText(srcPath);
    ui->line_faceOnly_path_src->setReadOnly(true);

    if (!ui->b_faceOnly_launch->isEnabled())
    {
        ui->b_faceOnly_launch->setEnabled(true);
    }

    this->m_log(tr("Browse source directory for cropping face process."));
}
//#################################################
void ConfluentI::m_faceOnly()
{
    this->m_log(tr("Cropping image process, launched"));
    // can not launch another instance of cropping
    ui->b_faceOnly_launch->setEnabled(false);

    if (ui->line_faceOnly_path_src->text().isEmpty())
    {
        QMessageBox::information(this, tr("Information..."), tr("You need to browse or type the image source directory, first."));
    }

    QString imgPathSrc(ui->line_faceOnly_path_src->text());

    QDir imgDir(imgPathSrc);//point the face dir

    //just want the files list, not directories, and sort them by name
    QStringList imgList = imgDir.entryList(QDir::Files);
    int imgListTotal = imgList.size();//get the number of file in the face directory

    QFileInfoList facesListInfo = imgDir.entryInfoList(QDir::Files);//here we want the info about each file

    //tell the progress bar to re-adjust its maximum size
    ui->progressBar_faceOnly->setMaximum(imgListTotal);

    for (int y = 0; y < imgListTotal; y++)
    {
        QString pathToImgMat(imgPathSrc + imgList.at(y));
        Mat frameBuffer = imread(pathToImgMat.toLatin1().data());

        Mat gray;
        cvtColor(frameBuffer, gray, CV_BGR2GRAY);
        equalizeHist(gray, gray);

        QCoreApplication::processEvents();// the ui is not frozen while processing

        // Face detect
        std::vector<Rect> vecFaces;
        CascadeClassifier face_cascade;
        String face_cascade_name = "/home/monge/STUFF/Qt/Neski/etc/haarcascades/haarcascade_profileface.xml";
        face_cascade.load(face_cascade_name);

        face_cascade.detectMultiScale(gray, vecFaces, 1.1, 2,1 | CASCADE_FIND_BIGGEST_OBJECT, Size(10,10));

        for (unsigned int i = 0; i < vecFaces.size(); i++)
        {
            //face detect rectangle plus more pixel to make the picture recognizable by human
            Point upperLeftFace(vecFaces[i].x - 5, vecFaces[i].y - 5);
            Point lowerRightFace(vecFaces[i].x+vecFaces[i].width + 5, vecFaces[i].y+vecFaces[i].height + 5);

            rectangle(/*Matrice*/frameBuffer, /*Rect*/upperLeftFace, /*Rect*/lowerRightFace, /*BGR Color*/Scalar(255, 255,0), /*Line height*/1, /*line type*/8);

            // get image and face width x height and convert these to QString so they can be displayed on QLabel
            QString imgSrcInfo("Image: " + QString::number(frameBuffer.cols, 10) + " x " + QString::number(frameBuffer.rows, 10) + ";\t\t Face: " +  QString::number(vecFaces[i].width, 10) + " x " + QString::number(vecFaces[i].height, 10));
            ui->lab_faceOnly_src_log2->setText(imgSrcInfo);

            QString imgDstInfo("Cropped image: " +  QString::number(vecFaces[i].width, 10) + " x " + QString::number(vecFaces[i].height, 10));
            faceWidth = vecFaces[i].width;// for the face dimension Pixmap
            faceHeight = vecFaces[i].height;// for the face dimension Pixmap
            ui->lab_faceOnly_dest_log2->setText(imgDstInfo);

            //dynamically create a cropped Mat from the frameBuffer Mat with the face dimensions
            att_MatCroppedFaceOnly = new Mat(frameBuffer, Rect(vecFaces[i].x, vecFaces[i].y, vecFaces[i].width, vecFaces[i].height));
        }
        //******************************************************src
        att_QImage = m_Mat2QImage(frameBuffer);
        *att_PixImage = QPixmap::fromImage(att_QImage.scaled(frameBuffer.cols, frameBuffer.rows, Qt::KeepAspectRatio), Qt::AutoColor);

        ui->lab_faceOnly_src->setPixmap(*att_PixImage);
        ui->lab_faceOnly_src_log->setText(facesListInfo.at(y).fileName());

        //******************************************************dst
        // create a QImage from the att_MatCroppedFaceOnly Matrix with vecFaces (the detected face) dimensions
        att_QImageForFaceOnly = m_Mat2QImage(*att_MatCroppedFaceOnly);
        *att_PixImageForFaceOnly = QPixmap::fromImage(att_QImageForFaceOnly.scaled(faceWidth, faceHeight, Qt::KeepAspectRatio), Qt::AutoColor);

        ui->lab_faceOnly_dest->setPixmap(*att_PixImageForFaceOnly);

        QString dstFileName = facesListInfo.at(y).fileName() + "_cropped_" + QDateTime::currentDateTime().toString("dd-MM-yyyy_hh:mm:ss");
        ui->lab_faceOnly_dest_log->setText(dstFileName);
        //******************************************************store dst image
        QFile savedFile(imgPathSrc + dstFileName + ".png");
//        if (!savedFile.open(QIODevice::WriteOnly))//test if can write
//        {
//            QMessageBox::critical(this, "Warning...", "Unable to write the converted image file!");
//            return;
//        }
        //save the att_QImageForFaceOnly QImage to file name in an uncompressed png image file
        att_QImageForFaceOnly.save(imgPathSrc + dstFileName + ".png", "PNG", 100);//write the image to the same path with different name
        savedFile.close(); //close the file

        ui->lab_faceOnly_count->setText(QString::number(y + 1, 10) + " / " + QString::number(imgListTotal, 10));
        ui->progressBar_faceOnly->setValue(y + 1);

        gray.release();
        frameBuffer.release();
//        delete att_PixImage;
//        delete att_PixImageForFaceOnly;
//        delete att_MatCropped;
//        delete att_MatCroppedFaceOnly;
    }
    // tell the user, we are done
    QMessageBox::information(this, tr("Information..."), tr("Cropping process is done."));

    //******************************************************src
    ui->lab_faceOnly_src->clear();
    ui->lab_faceOnly_src->setText(tr("Done"));
    ui->lab_faceOnly_src_log->clear();
    ui->lab_faceOnly_src_log2->clear();

    //******************************************************dst
    ui->lab_faceOnly_dest->clear();
    ui->lab_faceOnly_dest->setText(tr("Done"));
    ui->lab_faceOnly_dest_log->clear();
    ui->lab_faceOnly_dest_log2->clear();

    ui->progressBar_faceOnly->setValue(0);
    ui->lab_faceOnly_count->setText("--- / ---");

    // can launch another instance of cropping
    ui->b_faceOnly_launch->setEnabled(true);

    this->m_log(tr("Cropping image process, finished"));
}
//#################################################
QImage ConfluentI::m_Mat2QImage(cv::Mat Mat2Convert)
{
    cvtColor(Mat2Convert, Mat2Convert, CV_BGR2RGB);
    QImage img = QImage((const unsigned char*) (Mat2Convert.data), Mat2Convert.cols, Mat2Convert.rows, Mat2Convert.step, QImage::Format_RGB888);
    return img;
}
//#################################################
QImage ConfluentI::m_Mat2QImageGray(cv::Mat Mat2ConvertToGray)
{
    cvtColor(Mat2ConvertToGray, Mat2ConvertToGray, CV_BGR2GRAY);
    QImage img = QImage((const unsigned char*) (Mat2ConvertToGray.data), Mat2ConvertToGray.cols, Mat2ConvertToGray.rows, Mat2ConvertToGray.step, QImage::Format_Indexed8);
    return img;
}
//########################################################
void ConfluentI::m_hasher(QString dataStringToHash)
{

    if (ui->line_insert_name->text().isEmpty() || ui->line_insert_firstname->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Warning..."), tr("You first have to fill all the gray field to get the id."));
    }
    else
    {
        //take all the fiedls to identify the subject
        dataStringToHash = ui->line_insert_name->text() + ui->line_insert_firstname->text() + ui->line_insert_region->currentText();
        QByteArray dataByteArray = dataStringToHash.toLocal8Bit();//convert String to ByteArray

        QCryptographicHash dataHash(QCryptographicHash::Sha1);//create a hash with Sha1 algorithm
        //QCryptographicHash dataHash(QCryptographicHash::MD5);//create a hash with MD5 algorithm


        QByteArray dataHashedByteArray = dataHash.hash(dataByteArray, QCryptographicHash::Md5);//apply the hash on the data

        QString dataHashedString(dataHashedByteArray.toHex());//pass hashed Byte Array to QString for displaying

        ui->line_insert_id->setText(dataHashedString);
    }
}
//########################################################
void ConfluentI::m_sqlQuerySelect()
{
    QSqlTableModel *model = new QSqlTableModel(this, dataBase);
    model->setTable(ui->combo_ctrl_table_name->currentText());
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();
    model->setHeaderData(0, Qt::Horizontal, tr("Conf. id."));
    model->setHeaderData(1, Qt::Horizontal, tr("Names"));
    model->setHeaderData(2, Qt::Horizontal, tr("First names"));
    model->setHeaderData(3, Qt::Horizontal, tr("Regions"));

    model->setHeaderData(4, Qt::Horizontal, tr("Arrondis"));
    model->setHeaderData(5, Qt::Horizontal, tr("Quartiers"));
    model->setHeaderData(6, Qt::Horizontal, tr("Addresses"));
    model->setHeaderData(7, Qt::Horizontal, tr("Date of birth"));
    model->setHeaderData(8, Qt::Horizontal, tr("Place of birth"));
    model->setHeaderData(9, Qt::Horizontal, tr("Height"));
    model->setHeaderData(10, Qt::Horizontal, tr("Weight"));
    model->setHeaderData(11, Qt::Horizontal, tr("Hair color"));
    model->setHeaderData(12, Qt::Horizontal, tr("Eyes color"));
    model->setHeaderData(13, Qt::Horizontal, tr("Sex"));
    model->setHeaderData(14, Qt::Horizontal, tr("Citizenship"));
    model->setHeaderData(15, Qt::Horizontal, tr("Language"));
    model->setHeaderData(16, Qt::Horizontal, tr("Marks"));
    model->setHeaderData(17, Qt::Horizontal, tr("Status"));
    model->setHeaderData(18, Qt::Horizontal, tr("Current Description"));

    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();

    //first clear all listWidget to get a non duplicated clean list
    ui->list_search_alaotra_mangoro->clear();
    ui->list_search_amoron_i_mania->clear();
    ui->list_search_analamanga->clear();
    ui->list_search_analanjirofo->clear();
    ui->list_search_androy->clear();
    ui->list_search_anosy->clear();
    ui->list_search_atsimo_andrefana->clear();
    ui->list_search_atsimo_atsinanana->clear();
    ui->list_search_atsinanana->clear();
    ui->list_search_betsiboka->clear();
    ui->list_search_boeny->clear();
    ui->list_search_bongolava->clear();
    ui->list_search_diana->clear();
    ui->list_search_ihorombe->clear();
    ui->list_search_itasy->clear();
    ui->list_search_matsiatra_ambony->clear();
    ui->list_search_melaky->clear();
    ui->list_search_menabe->clear();
    ui->list_search_sava->clear();
    ui->list_search_sofia->clear();
    ui->list_search_vakinankaratra->clear();
    ui->list_search_vatovavy_fitovinany->clear();

    QSqlQuery query("SELECT name, first_name, region FROM " + ui->combo_ctrl_table_name->currentText() + " ORDER BY name");
    QString returnedStringRow0, returnedStringRow1, returnedStringRow2;

    while (query.next())
    {
        returnedStringRow0 = query.value(0).toString();// zero here is not the first column of the table but the first specified field at the SELECT query ci-dessus
        returnedStringRow1 = query.value(1).toString();// One here is firstnames in the select ci-dessus
        returnedStringRow2 = query.value(2).toString();

        if (returnedStringRow2 == "Alaotra-Mangoro")
        {
            ui->list_search_alaotra_mangoro->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Amoron'i Mania")
        {
            ui->list_search_amoron_i_mania->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Analamanga")
        {
            ui->list_search_analamanga->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Analanjirofo")
        {
            ui->list_search_analanjirofo->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Androy")
        {
            ui->list_search_androy->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Anosy")
        {
            ui->list_search_anosy->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Atsimo Andrefana")
        {
            ui->list_search_atsimo_andrefana->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Atsimo Atsinanana")
        {
            ui->list_search_atsimo_atsinanana->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Atsinanana")
        {
            ui->list_search_atsinanana->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Betsiboka")
        {
            ui->list_search_betsiboka->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Boeny")
        {
            ui->list_search_boeny->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Bongolava")
        {
            ui->list_search_bongolava->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Diana")
        {
            ui->list_search_diana->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Ihorombe")
        {
            ui->list_search_ihorombe->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Itasy")
        {
            ui->list_search_itasy->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Matsiatra Ambony")
        {
            ui->list_search_matsiatra_ambony->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Melaky")
        {
            ui->list_search_melaky->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Menabe")
        {
            ui->list_search_menabe->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Sava")
        {
            ui->list_search_sava->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Sofia")
        {
            ui->list_search_sofia->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Vakinankaratra")
        {
            ui->list_search_vakinankaratra->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
        else if (returnedStringRow2 == "Vatovavy Fitovinany")
        {
            ui->list_search_vatovavy_fitovinany->addItem(returnedStringRow0 + " " + returnedStringRow1);
        }
    }
    //update total_input line
    int entryRowCount(query.size());
    ui->line_insert_totalEntry->setText(QString::number(entryRowCount));

    //from here, update all count list of region people
    int entryAlaotraMangoroCount(ui->list_search_alaotra_mangoro->count());
    ui->lab_search_alaotra_mangoro->setText("Alaotra-Mangoro: " + QString::number(entryAlaotraMangoroCount));

    int entryAmoronIManiaCount(ui->list_search_amoron_i_mania->count());
    ui->lab_search_amoron_i_mania->setText("Amoron'i Mania: " + QString::number(entryAmoronIManiaCount));

    int entryAnalamangaCount(ui->list_search_analamanga->count());
    ui->lab_search_analamanga->setText("Analamanga: " + QString::number(entryAnalamangaCount));

    int entryAnalanjirofoCount(ui->list_search_analanjirofo->count());
    ui->lab_search_analanjirofo->setText("Analanjirofo: " + QString::number(entryAnalanjirofoCount));

    int entryAndroyCount(ui->list_search_androy->count());
    ui->lab_search_androy->setText("Androy: " + QString::number(entryAndroyCount));

    int entryAnosyCount(ui->list_search_anosy->count());
    ui->lab_search_anosy->setText("Anosy: " + QString::number(entryAnosyCount));

    int entryAtsimoAndrefanaCount(ui->list_search_atsimo_andrefana->count());
    ui->lab_search_atsimo_andrefana->setText("Atsimo Andrefana: " + QString::number(entryAtsimoAndrefanaCount));

    int entryAtsimoAtsinananaCount(ui->list_search_atsimo_atsinanana->count());
    ui->lab_search_atsimo_atsinanana->setText("Atsimo Atsinanana: " + QString::number(entryAtsimoAtsinananaCount));

    int entryAtsinananaCount(ui->list_search_atsinanana->count());
    ui->lab_search_atsinanana->setText("Atsinanana: " + QString::number(entryAtsinananaCount));

    int entryBetsibokaCount(ui->list_search_betsiboka->count());
    ui->lab_search_betsiboka->setText("Betsiboka: " + QString::number(entryBetsibokaCount));

    int entryBoenyCount(ui->list_search_boeny->count());
    ui->lab_search_boeny->setText("Boeny: " + QString::number(entryBoenyCount));

    int entryBongolavaCount(ui->list_search_bongolava->count());
    ui->lab_search_bongolava->setText("Bongolava: " + QString::number(entryBongolavaCount));

    int entryDianaCount(ui->list_search_diana->count());
    ui->lab_search_diana->setText("Diana: " + QString::number(entryDianaCount));

    int entryIhorombeCount(ui->list_search_ihorombe->count());
    ui->lab_search_ihorombe->setText("Ihorombe: " + QString::number(entryIhorombeCount));

    int entryItasyCount(ui->list_search_itasy->count());
    ui->lab_search_itasy->setText("Itasy: " + QString::number(entryItasyCount));

    int entryMatsiatraAmbonyCount(ui->list_search_matsiatra_ambony->count());
    ui->lab_search_matsiatra_ambony->setText("Matsiatra Ambony: " + QString::number(entryMatsiatraAmbonyCount));

    int entryMelakyCount(ui->list_search_melaky->count());
    ui->lab_search_melaky->setText("Melaky: " + QString::number(entryMelakyCount));

    int entryMenabeCount(ui->list_search_menabe->count());
    ui->lab_search_menabe->setText("Menabe: " + QString::number(entryMenabeCount));

    int entrySavaCount(ui->list_search_sava->count());
    ui->lab_search_sava->setText("Sava: " + QString::number(entrySavaCount));

    int entrySofiaCount(ui->list_search_sofia->count());
    ui->lab_search_sofia->setText("Sofia: " + QString::number(entrySofiaCount));

    int entryVakinankaratraCount(ui->list_search_vakinankaratra->count());
    ui->lab_search_vakinankaratra->setText("Vakinankaratra: " + QString::number(entryVakinankaratraCount));

    int entryVatovavyFitovinanyCount(ui->list_search_vatovavy_fitovinany->count());
    ui->lab_search_vatovavy_fitovinany->setText("Vatovavy Fitovinany: " + QString::number(entryVatovavyFitovinanyCount));

    this->m_log(tr("SELECT Query, executed."));
}

//########################################################
void ConfluentI::m_sqlQueryInsert()
{
    QSqlQuery query;
    query.prepare("INSERT INTO " + ui->combo_ctrl_table_name->currentText() +
                  " (conf_id, name, first_name, region, arrondis, quartier, address, date_of_birth, place_of_birth, height, weight, hair_color, eyes_color, sex, citizenship, language, mark, status, current_desc)"
                  " VALUES (:conf_id, :name, :first_name, :region, :arrondis, :quartier, :address, :date_of_birth, :place_of_birth, :height, :weight, :hair_color, :eyes_color, :sex, :cytizenship, :language, :mark, :status, :current_desc)");
    query.bindValue(0, ui->line_insert_id->text());
    query.bindValue(1, ui->line_insert_name->text());
    query.bindValue(2, ui->line_insert_firstname->text());
    query.bindValue(3, ui->line_insert_region->currentText());

    query.bindValue(4, ui->line_insert_arrondiss->text());
    query.bindValue(5, ui->line_insert_fkt->text());
    query.bindValue(6, ui->line_insert_address->text());
    query.bindValue(7, ui->line_insert_date_of_birth->text());
    query.bindValue(8, ui->line_insert_place_of_birth->text());
    query.bindValue(9, ui->line_insert_height->text());
    query.bindValue(10, ui->line_insert_weight->text());
    query.bindValue(11, ui->line_insert_hair->text());
    query.bindValue(12, ui->line_insert_eyes->text());
    query.bindValue(13, ui->line_insert_sex->currentText());
    query.bindValue(14, ui->line_insert_citizenchip->text());
    query.bindValue(15, ui->line_insert_language->text());

    query.bindValue(16, ui->line_insert_marks->text());
    query.bindValue(17, ui->line_insert_status->text());
    query.bindValue(18, QString(ui->text_insert_description->toPlainText()));

    if (query.exec())
    {
        this->m_log(tr("INSERT Query successfully executed"));
        this->m_sqlQuerySelect();//update the list for search widget
    }
    else
    {
        qDebug() << query.lastError().text();
        this->m_log(query.lastQuery());
        this->m_log(tr("Query error!") + query.QSqlQuery::lastError().driverText());
    }
}
//########################################################
QString ConfluentI::m_currentDateAndTime()
{
    QString date(QDateTime::currentDateTime().date().toString(tr("dd-MM-yyyy")));
    QString time(QDateTime::currentDateTime().time().toString("hh:mm:ss"));

    dateAndTime = QString(date + "_" + time);
    return dateAndTime;
}
//########################################################
void ConfluentI::m_clear()
{
    ui->line_ctrl_db_name->clear();
    ui->line_ctrl_host_name->clear();
    ui->line_ctrl_login->clear();
    ui->line_ctrl_pass->clear();
}
//########################################################
void ConfluentI::m_connectToDb()
{
    dataBase.setHostName(ui->line_ctrl_host_name->text());
    dataBase.setUserName(ui->line_ctrl_login->text());
    dataBase.setPassword(ui->line_ctrl_pass->text());
    dataBase.setDatabaseName(ui->line_ctrl_db_name->text());

    if(dataBase.open())
    {
        //ui->text_log->append(this->m_currentDateAndTime() + " User \"" + dataBase.userName() + "\" connected to host: \"" + dataBase.hostName() + "\"; Database: \"" + dataBase.databaseName() + "\".");
        this->m_log(tr("Connected to Database."));

        //activate disabled subwindows
        ui->sub_insert->setEnabled(true);
        ui->sub_search->setEnabled(true);
        ui->sub_scan->setEnabled(true);
        ui->sub_faceonly->setEnabled(true);

        ui->b_ctrl_select->setEnabled(true);

        //get table list
        QStringList tablesList = dataBase.tables(QSql::Tables);
        ui->combo_ctrl_table_name->addItems(tablesList);

        //needs to be fixed, i don't know why the stop button is disabled after a connect operation
        ui->b_scan_stop->setEnabled(true);

        //activate action for a deactivated sub window
        att_ActionToolBarSubInsert->setEnabled(true);
        att_ActionToolBarSubSearch->setEnabled(true);

        ui->b_ctrl_disconnect->setEnabled(true);//can disconnect

        //disabled, the user is connected
        ui->b_ctrl_connect->setEnabled(false);
        ui->b_ctrl_clear->setEnabled(false);
        ui->b_ctrl_quit->setEnabled(false);

        ui->line_ctrl_host_name->setReadOnly(true);
        ui->line_ctrl_db_name->setReadOnly(true);
        ui->line_ctrl_login->setReadOnly(true);
        ui->line_ctrl_pass->setReadOnly(true);

        //clear all insert widgets so the user can begin to insert
        ui->line_insert_id->clear();
        ui->line_insert_name->clear();
        ui->line_insert_firstname->clear();
        ui->line_insert_region->setCurrentIndex(0);
        ui->line_insert_arrondiss->clear();
        ui->line_insert_fkt->clear();
        ui->line_insert_address->clear();
        ui->line_insert_date_of_birth->clear();
        ui->line_insert_place_of_birth->clear();
        ui->line_insert_height->clear();
        ui->line_insert_weight->clear();
        ui->line_insert_hair->clear();
        ui->line_insert_eyes->clear();
        ui->line_insert_sex->setCurrentIndex(0);
        ui->line_insert_citizenchip->clear();
        ui->line_insert_language->clear();
        ui->line_insert_marks->clear();
        ui->line_insert_status->clear();

        //this->m_sqlQuerySelect();//query a list to the db
    }
    else
    {
        //ui->text_log->append(this->m_currentDateAndTime() + " User \"" + dataBase.userName() + "\" not connected to host: \"" + dataBase.hostName() + "\"; Database: \"" + dataBase.databaseName() + "\".");
    }
}
//########################################################
void ConfluentI::m_disConnectToDb()
{
    if(dataBase.isOpen())
    {
        dataBase.close();

        //ui->text_log->append(this->m_currentDateAndTime() + " Disconnected to host: \"" + dataBase.hostName() + "\"; Database: \"" + dataBase.databaseName() + "\".");
        this->m_log(tr("Disconnected to Database."));

        ui->sub_insert->setEnabled(false);
        ui->b_ctrl_select->setEnabled(false);

        //not connected, disable facial rec. stuff
        ui->sub_scan->setEnabled(false);
        ui->sub_faceonly->setEnabled(false);
        ui->line_faceOnly_path_src->clear();

        //******************************************************src
        ui->lab_faceOnly_src->clear();
        ui->lab_faceOnly_src_log->clear();
        ui->lab_faceOnly_src_log2->clear();

        //******************************************************dst
        ui->lab_faceOnly_dest->clear();
        ui->lab_faceOnly_dest_log->clear();
        ui->lab_faceOnly_dest_log2->clear();

        ui->progressBar_faceOnly->setValue(0);

        //clear table list
        ui->combo_ctrl_table_name->clear();

        //deactivate the search subwindow
        ui->sub_search->setEnabled(false);

        //really leave the app, clean all
        ui->scrollArea_search->ensureVisible(0, 0);
        ui->scrollArea_insert->ensureVisible(0, 0);

        ui->lab_scan_subject->clear();
        ui->lab_scan_database->clear();
        ui->lab_scan_match->clear();

        ui->lab_insert_image->clear();
        ui->lab_insert_fingerprint->clear();

        //deactivate action for a deactivated sub window
        att_ActionToolBarSubInsert->setEnabled(false);
        att_ActionToolBarSubSearch->setEnabled(false);

        ui->b_ctrl_disconnect->setEnabled(false);//can't disconnect if disconnected

        //enabled, the user is not connected
        ui->b_ctrl_connect->setEnabled(true);
        ui->b_ctrl_clear->setEnabled(true);
        ui->b_ctrl_quit->setEnabled(true);

        ui->line_ctrl_host_name->setReadOnly(false);
        ui->line_ctrl_db_name->setReadOnly(false);
        ui->line_ctrl_login->setReadOnly(false);
        ui->line_ctrl_pass->setReadOnly(false);

        //no user connected, clear all list
        ui->list_search_alaotra_mangoro->clear();
        ui->list_search_amoron_i_mania->clear();
        ui->list_search_analamanga->clear();
        ui->list_search_analanjirofo->clear();
        ui->list_search_androy->clear();
        ui->list_search_anosy->clear();
        ui->list_search_atsimo_andrefana->clear();
        ui->list_search_atsimo_atsinanana->clear();
        ui->list_search_atsinanana->clear();
        ui->list_search_betsiboka->clear();
        ui->list_search_boeny->clear();
        ui->list_search_bongolava->clear();
        ui->list_search_diana->clear();
        ui->list_search_ihorombe->clear();
        ui->list_search_itasy->clear();
        ui->list_search_matsiatra_ambony->clear();
        ui->list_search_melaky->clear();
        ui->list_search_menabe->clear();
        ui->list_search_sava->clear();
        ui->list_search_sofia->clear();
        ui->list_search_vakinankaratra->clear();
        ui->list_search_vatovavy_fitovinany->clear();

        //clear all insert widgets before disconnecting
        ui->line_insert_id->clear();
        ui->line_insert_name->clear();
        ui->line_insert_firstname->clear();
        ui->line_insert_region->setCurrentIndex(0);
        ui->line_insert_arrondiss->clear();
        ui->line_insert_fkt->clear();
        ui->line_insert_address->clear();
        ui->line_insert_date_of_birth->clear();
        ui->line_insert_place_of_birth->clear();
        ui->line_insert_height->clear();
        ui->line_insert_weight->clear();
        ui->line_insert_hair->clear();
        ui->line_insert_eyes->clear();
        ui->line_insert_sex->setCurrentIndex(0);
        ui->line_insert_citizenchip->clear();
        ui->line_insert_language->clear();
        ui->line_insert_marks->clear();
        ui->line_insert_status->clear();

        //reset labels content
        ui->lab_search_analamanga->setText("Alaotra-Mangoro:");
        ui->lab_search_analamanga->setText("Amoron'i Mania:");
        ui->lab_search_analamanga->setText("Analamanga:");
        ui->lab_search_analamanga->setText("Analanjirofo:");
        ui->lab_search_analamanga->setText("Androy:");
        ui->lab_search_analamanga->setText("Anosy:");
        ui->lab_search_analamanga->setText("Atsimo Andrefana:");
        ui->lab_search_analamanga->setText("Atsimo Atsinanana:");
        ui->lab_search_analamanga->setText("Atsinanana:");
        ui->lab_search_analamanga->setText("Betsiboka:");
        ui->lab_search_analamanga->setText("Boeny:");
        ui->lab_search_analamanga->setText("Bongolava:");
        ui->lab_search_analamanga->setText("Diana:");
        ui->lab_search_analamanga->setText("Ihorombe:");
        ui->lab_search_analamanga->setText("Itasy:");
        ui->lab_search_analamanga->setText("Matsiatra Ambony:");
        ui->lab_search_analamanga->setText("Melaky:");
        ui->lab_search_analamanga->setText("Menabe:");
        ui->lab_search_analamanga->setText("Sava:");
        ui->lab_search_analamanga->setText("Sofia:");
        ui->lab_search_analamanga->setText("Vakinankaratra:");
        ui->lab_search_analamanga->setText("Vatovavy Fitovinany:");

        ui->combo_scan_search->setCurrentIndex(0);
    }
    else
    {
        //ui->text_log->append(this->m_currentDateAndTime() + " Not Connected to Host: \"" + dataBase.hostName() + "\"; Database: \"" + dataBase.databaseName() + "\".");
    }
}
//########################################################
void ConfluentI::m_reorganizeCascade()
{
    ui->mdiArea->cascadeSubWindows();
}
//########################################################
void ConfluentI::m_reorganizeTile()
{
    ui->mdiArea->tileSubWindows();
}
//########################################################
void ConfluentI::m_reorganizeCustom()
{
}
//#################################################
void ConfluentI::m_handleSubControl()
{
    if (ui->sub_control->isMinimized() || !ui->sub_control->isVisible() || ui->sub_control->isMaximized())
    {
        ui->sub_control->showNormal();
    }
    else
    {
        ui->sub_control->showMinimized();
    }
}
//#################################################
void ConfluentI::m_handleSubScan()
{
    if (ui->sub_scan->isMinimized() || !ui->sub_scan->isVisible() || ui->sub_scan->isMaximized())
    {
        ui->sub_scan->showNormal();
    }
    else
    {
        ui->sub_scan->showMinimized();
    }
}
//#################################################
void ConfluentI::m_handleSubInsert()
{
    if (ui->sub_insert->isMinimized() || !ui->sub_insert->isVisible() || ui->sub_insert->isMaximized())
    {
        ui->sub_insert->showNormal();
    }
    else
    {
        ui->sub_insert->showMinimized();
    }
}
//#################################################
void ConfluentI::m_handleSubLog()
{
    if (ui->sub_log->isMinimized() || !ui->sub_log->isVisible() || ui->sub_log->isMaximized())
    {
        ui->sub_log->showNormal();
    }
    else
    {
        ui->sub_log->showMinimized();
    }
}
//#################################################
void ConfluentI::m_handleSubSearch()
{
    if (ui->sub_search->isMinimized() || !ui->sub_search->isVisible() || ui->sub_search->isMaximized())
    {
        ui->sub_search->showNormal();
    }
    else
    {
        ui->sub_search->showMinimized();
    }
}
//########################################################
void ConfluentI::m_handleLoginFile()
{
    att_LoginList = new QStringList();
    att_LoginFile = new QFile(QDir::currentPath()+ "../others/id_login_pass/" + "id_login_pass.txt");
    if (!att_LoginFile->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Warning..."), tr("Login file not found in: \"") + QDir::currentPath() + "/id_login_pass" "\".");
    }
    else//read all logins in login file
    {
        int index(0);//to point at which place to store the read string from login file
        QString eachReadLine;//store each read line
        QTextStream eachReadStream(att_LoginFile);//stream pointed to login file

        while(!eachReadStream.atEnd())
        {
            eachReadLine = eachReadStream.readLine();//read eachline while at end of stream pointing to login file
            att_LoginList->insert(index, eachReadLine);//insert the read line at index in stringlist
            ++index;//increment stringlist index
        }
    }

    //show an inputdialog to get a login
    att_LoginDialog = new QInputDialog(this);

    bool(ok);
    QString fromDecryptionSequence = att_LoginDialog->getText(this, tr("Login process"), tr("Please login!"), QLineEdit::Normal, "login", &ok);//get entered login
    //test on which button the user clicked
    if (ok && fromDecryptionSequence.isEmpty())//if Ok and left the input dialog empty
    {
        QMessageBox::warning(this, tr("Information"), tr("Empty input!\nLogin process aborted."), QMessageBox::Ok);
        this->m_handleLoginFile();
    }
    //test if the user clicked on Cancel
    else if (!ok)
    {
        //QMessageBox::warning(this, tr("Information"), tr("Cancel button clicked!\nLeaving."), QMessageBox::Ok);
        this->close();
    }
    //compare the string from input dialog to the login file content
    if (!att_LoginList->contains(fromDecryptionSequence))//if list doen't contains the entered login
    {

        QMessageBox::warning(this, tr("Warning..."), tr("Error login."));
        return;

    }
    else if (att_LoginList->contains(fromDecryptionSequence))//if yes
    {
        //execute the confluentI core
    }

    att_LoginFile->close();
}
//########################################################
void ConfluentI::closeEvent(QCloseEvent* ev)
{
    //if a user is connected (disconnect button activated) ignore the close event from the WindowManager
    if (ev->type() == QEvent::Close && ui->b_ctrl_disconnect->isEnabled())
    {
        ev->ignore();
    }
    else
        this->close();
}

//########################################################
ConfluentI::~ConfluentI()
{
    this->m_disConnectToDb();
    delete ui;
}
//########################################################
