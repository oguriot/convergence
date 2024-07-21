QT += core gui sql widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


# Input
HEADERS += confluenti.h
FORMS += confluenti.ui
SOURCES += confluenti.cpp main.cpp
RESOURCES += confluenti.qrc

INCLUDEPATH +=/usr/local/include/opencv4/
DEPENDPATH +=/usr/local/include/opencv4/
PKGCONFIG +=opencv4

LIBS += -L/usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d\
     -lopencv_highgui\
     -lopencv_objdetect\
     -lopencv_imgcodecs\
     -lopencv_video\
     -lopencv_videoio

OTHER_FILES += \
    todo.txt

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

