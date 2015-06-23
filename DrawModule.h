#ifndef DRAWMODULE_H
#define DRAWMODULE_H
#include "InterModule.h"
#include "frameviewer.h"
#include "DrawPath.h"
#include "DrawCurves.h"
#include <QObject>
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QRgb>

enum DrawTool { D_DRAW, D_DROPPER, D_SELECT };

class FrameViewer;

class DrawModule : public InterModule
{
    Q_OBJECT
public:
    DrawModule(FrameViewer *parent, int length);
    ~DrawModule();
    virtual void paintGL();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void keyPressEvent (QKeyEvent *e);
    virtual void frameChange(int i);

    void toolChange(DrawTool id);
    void propagatedStroke();
    QRgb getCurrColor();
    void setColor(const Vec3f& col);
signals:
    void selectChanged();
    void currColorChanged();

private:
    int _mousePressed;
    FrameViewer *_parent;
    DrawTool _toolMode;

    bool _freeDraw;
    float _dAlpha;
    int _renderMode;

    float _pressure;
    float _sens;
    float mapPressure() const;

    int _frame;
    Vec2f _dragLoc;
    bool _corrShow;
    bool _dragDirty;
    DrawPath *_currDPath;
    DrawPath *_selected;
    DrawCurves *_dc;
    int _spliceIndex, _spliceFlag; // -1 not active, 1 on curve, 0 off,  -2 for needs correspondences
    DrawCurves *_drawCurvesArray;
    void propagatePathEverywhere(DrawPath* path);
    void optimizeDrawShape(DrawPath* aPath, DrawPath* bPath, int aFrame, int bFrame, bool pinLast);
    void regeneratePath(DrawPath* path);
    void corrPropAll();

    Vec3f _currColor;
    Vec2i _captureColorLoc;
    bool _captureColor;
    void captureColor();
};

#endif // DRAWMODULE_H
