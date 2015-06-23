#ifndef ROTOSCOPE_MODULE_H
#define ROTOSCOPE_MODULE_H
#include "InterModule.h"
#include "RotoPath.h"
#include "RotoCurves.h"
#include "KLT.h"
#include <QGLWidget>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

enum TrackTool { T_MANUAL, T_SELECT, T_NUDGE };

class FrameViewer;

class RotoscopeModule : public InterModule
{
    Q_OBJECT
public:
    RotoscopeModule(FrameViewer *parent, cv::VideoCapture *capture, int length);
    ~RotoscopeModule();
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void frameChange(int i);

    void toolChange(TrackTool id);
    void setVisEffort(const bool s);
    void setShowTrackPoints(const bool s);
    void copySplinesAcrossTime();
    void rejigWrapper(bool startOver);
    void rejig(PathV* paths,  int frame, bool startOver);

    RotoCurves *_rotoCurvesArray;
    KLT_TrackingContext globalTC;

signals:
    void enablePbCopySplinesAcrossTime(bool enable);

private:
    FrameViewer *_parent;
    RotoPath *_currPath;
    TrackTool _toolMode;
    RotoCurves *_currRC;

    vector<RotoPath*> _selected;
    Vec2f _dragLoc;

    RotoPath* _ctrlDrag;
    int _dragCtrlNum;

    int _nowFrame, _length;
    int _lastFrameTouched, _changeLastFrameTouched;

    std::list<TrackGraph*> _ccompV;
    std::list<KLT_TrackingContext*> _TCV;
    std::list<MultiSplineData*> _trackDataV;
    KLT_FullCPyramid *_Cpyrms;
    KLT_FullPyramid *_pyrmsE;
    PathV _toTrack;
    bool _tracking;
    int _trackingTimer;
    void performTracks(const int aFrame, const int bFrame, bool doInterp=true, bool useExistingInbetweens=false);
    void keyframeSedInterp(RotoPath* aPath, int aFrame, RotoPath *bPath, int bFrame);
    void addMasksToMulti(MultiSplineData* mts, const PathV& key0, const int frame0);
    cv::VideoCapture *_capture;
    QImage getImage();
    bool setImageIndex(int index);

    bool _isCorrShow;
    bool _manualStage;
    bool _visEffort;
    bool _showTrackPoints;

    void finishManualRotoCurve();
    void killAnyRoto();
};

#endif // ROTOSCOPE_MODULE_H
