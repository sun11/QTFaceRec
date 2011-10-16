TARGET = QTFaceRecForPC

HEADERS += \
    videodevice.h \
    processImage.h \
    mainwindow.h \
    facedetect.h \
    facerecognize.h \
    qextserialbase.h \
    posix_qextserialport.h \
    thread.h

SOURCES += \
    videodevice.cpp \
    processImage.cpp \
    main.cpp \
    mainwindow.cpp \
    facedetect.cpp \
    facerecognize.cpp \
    qextserialbase.cpp \
    posix_qextserialport.cpp \
    thread.cpp

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += /usr/local/lib/pkgconfig/opencv-arm.pc
}

FORMS += \
    mainwindow.ui
