#-------------------------------------------------
#
# Project created by QtCreator 2015-06-20T15:34:28
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NPR-2015
TEMPLATE = app

INCLUDEPATH += KLT roto

SOURCES += \
    KLT/BezSpline.cpp \
    KLT/ContCorr.cpp \
    KLT/MultiSplineData.cpp \
    roto/AbstractPath.cpp \
    roto/GeigerCorresponder.cpp \
    roto/RotoCorresponder.cpp \
    roto/RotoPath.cpp \
    roto/RotoRegion.cpp \
    roto/TrackGraph.cpp \
    FrameViewer.cpp \
    main.cpp \
    MainWindow.cpp \
    RotoscopeModule.cpp \
    VideoProcessor.cpp \
    roto/FitCurves.c \
    roto/GGVecLib.c \
    InterModule.cpp \
    RangeDialog.cpp \
    roto/RotoCurves.cpp \
    KLT/Error.c \
    KLT/MySparseMat.cpp \
    KLT/LinearSolver.cpp \
    KLT/KLT.cpp \
    KLT/Keeper.cpp \
    KLT/MultiKeeper.cpp \
    KLT/Kernels.cpp \
    KLT/klt_util.cpp \
    KLT/Pyramid.cpp \
    KLT/kltSpline.cpp \
    KLT/HB_Sweep.cpp \
    KLT/SplineKeeper.cpp \
    KLT/ObsCache.cpp \
    KLT/HB_OneCurve.cpp \
    KLT/MultiDiagMatrix.cpp \
    KLT/DiagMatrix.cpp \
    DrawModule.cpp \
    roto/DrawPath.cpp \
    roto/Stroke.cpp \
    roto/Bitmap.cpp \
    roto/Texture.cpp \
    roto/DrawContCorr.cpp \
    roto/DrawCorresponder.cpp \
    roto/DrawCurves.cpp

HEADERS  += \
    KLT/BezSpline.h \
    KLT/ContCorr.h \
    KLT/CubicEval.h \
    KLT/DSample.h \
    KLT/jl_vectors.h \
    KLT/LList.h \
    KLT/MultiSplineData.h \
    KLT/MultiTrackData.h \
    KLT/MyAssert.h \
    KLT/TalkFitCurve.h \
    roto/AbstractPath.h \
    roto/Bboxf2D.h \
    roto/dynarray.h \
    roto/FitCurves.h \
    roto/GC_Node.h \
    roto/GeigerCorresponder.h \
    roto/GraphicsGems.h \
    roto/Joint.h \
    roto/RotoCorresponder.h \
    roto/RotoPath.h \
    roto/RotoRegion.h \
    roto/TrackGraph.h \
    InterModule.h \
    MainWindow.h \
    RotoscopeModule.h \
    VideoProcessor.h \
    RangeDialog.h \
    roto/RotoCurves.h \
    KLT/KLT.h \
    KLT/Error.h \
    KLT/MySparseMat.h \
    KLT/Keeper.h \
    KLT/GenKeeper.h \
    KLT/MultiKeeper.h \
    KLT/HB_Sweep.h \
    KLT/HB_OneCurve.h \
    KLT/SplineKeeper.h \
    KLT/DiagMatrix.h \
    KLT/BuildingSplineKeeper.h \
    KLT/ObsCache.h \
    KLT/MyMontage.h \
    KLT/MyIMatrix.h \
    KLT/Pyramid.h \
    KLT/MultiDiagMatrix.h \
    KLT/klt_util.h \
    KLT/Kernels.h \
    KLT/kltSpline.h \
    KLT/base.h \
    DrawModule.h \
    frameviewer.h \
    roto/DrawPath.h \
    roto/Stroke.h \
    roto/Point.h \
    roto/Color.h \
    roto/Texture.h \
    roto/Bitmap.h \
    roto/DrawContCorr.h \
    roto/DrawCorresponder.h \
    roto/DrawCurves.h

FORMS    += \
    MainWindow.ui \
    RangeDialog.ui

RESOURCES += \
    npr-2015.qrc

unix {
    LIBS   += -lGL -lGLU
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
}

win32 {
INCLUDEPATH += \
        D:\OpenCV-2.4.9\build\include \
        D:\boost_1_58_0
LIBS += -LD:\OpenCV-2.4.9\build\x64\vc12\lib \
        -L"C:\Program Files (x86)\AMD APP SDK\3.0-0-Beta\lib\x86_64" \
        -L"D:\boost_1_58_0\lib64-msvc-12.0" \
        -lopencv_core249d \
        -lopencv_highgui249d \
        -lopencv_imgproc249d \
        -lopencv_features2d249d \
        -lopencv_calib3d249d \
        -lglut -lglew -lGLEW
}
