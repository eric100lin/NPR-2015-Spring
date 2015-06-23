#ifndef FRAMEVIEWER_H
#define FRAMEVIEWER_H
#include <QGLWidget>
#include "jl_vectors.h"
#include "RotoscopeModule.h"
#include "DrawModule.h"

class RotoscopeModule;
class DrawModule;

class FrameViewer : public QGLWidget
{
public:
    int _w, _h;
    RotoscopeModule *roto;
    DrawModule *draw;

    FrameViewer(QWidget *parent = 0);
    void showFrame(long index, QImage &frame);
    void setUpModules(cv::VideoCapture *capture, int videoLength);
    void changeModules(InterModule *module);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent * e);
    virtual void wheelEvent(QWheelEvent *event);

private:
    float _zoom;
    Vec2f _center;
    QImage _glback;
    int _backDlist;
    void createDlist();
    void myGlLoadIdentity();
    void resetZoom();
    void zoomUpdateGL();

    InterModule* _module;
};

#endif // FRAMEVIEWER_H
