#include "frameviewer.h"
#include <QGLFormat>
#ifdef _MSC_VER
#include <gl/GLU.h>
#else
#include <GL/glu.h>
#endif
#include <assert.h>
#define DEFAULT_WIDTH 620
#define DEFAULT_HEIGHT 410

FrameViewer::FrameViewer(QWidget *parent):
    _w(DEFAULT_WIDTH), _h(DEFAULT_HEIGHT),
    QGLWidget(QGLFormat::defaultFormat(),parent)
{
    roto = NULL;
    _module = NULL;
    _zoom = 1.f;
    _center.Set(_w / 2.f, _h / 2.f);
    QImage buf(DEFAULT_WIDTH,DEFAULT_HEIGHT, QImage::Format_RGB888);
    _glback = QGLWidget::convertToGLFormat(buf);
}

void FrameViewer::setUpModules(cv::VideoCapture *capture, int videoLength)
{
    roto = new RotoscopeModule(this, capture, videoLength);
    draw = new DrawModule(this, videoLength);
    _module = roto;
}

void FrameViewer::changeModules(InterModule *module)
{
    _module = module;
}

void FrameViewer::showFrame(long index, QImage &frame)
{
    QImage buf = frame.scaled(_w,_h);
    _glback = QGLWidget::convertToGLFormat(buf);
    if(_module!=NULL)
        _module->frameChange((int)index);
    updateGL();
}

void FrameViewer::initializeGL()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearDepth(1.0);
    glClear (GL_DEPTH_BUFFER_BIT);
    glDisable (GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearStencil(0x0);
    glDisable (GL_STENCIL_TEST);
    glLineStipple(2, 0xAAAA);
    _backDlist = glGenLists(1);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void FrameViewer::resizeGL(int width, int height)
{
    _w = width;
    _h = height;
    AbstractPath::_globalh = _h;
    _center.Set(_w / 2.f, _h / 2.f);
    QImage buf = _glback.scaled(width,height);
    _glback = QGLWidget::convertToGLFormat(buf);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f, (GLfloat) width, 0.0f, (GLfloat) height);
    glMatrixMode (GL_MODELVIEW);
    myGlLoadIdentity();
    glViewport(0, 0, width, height);
    createDlist();
}

void FrameViewer::createDlist()
{
    glNewList(_backDlist, GL_COMPILE);
    glClear (GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, _h);
    glDrawPixels(_w, _h, GL_RGBA, GL_UNSIGNED_BYTE, _glback.bits());
    glEndList();
}

void FrameViewer::myGlLoadIdentity()
{
    glLoadIdentity();
    glTranslatef(0, _h, 0);
    glScalef(1., -1., 1.);
}

void FrameViewer::paintGL()
{
    glPushMatrix();
    zoomUpdateGL();

    glClear (GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glLoadIdentity(); // we'll do this in GL space

    Vec2f myCenter(_center.x(), float(_h) - _center.y());
    Vec2f fd(_w, _h); // float dimensions
    Vec2f lowerLeft(myCenter.x() - fd.x() / (2.f * _zoom),
            myCenter.y() - fd.y() / (2.f * _zoom));
    glRasterPos2f(MAX(fd.x() / 2.f - myCenter.x() * _zoom, 0),
            MAX(fd.y() / 2.f - myCenter.y() * _zoom, 0));

    glPopMatrix();
    lowerLeft.Clamp(0, _w, _h);
    int offset = 4
            * (int(lowerLeft.y()) * _w + (int) lowerLeft.x());

    assert((int) lowerLeft.y() < _h);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, _w);
    glDrawPixels(_w - (int) ceil(lowerLeft.x()),
            _h - (int) ceil(lowerLeft.y()), GL_RGBA,
            GL_UNSIGNED_BYTE, _glback.bits() + offset);
    assert(!glGetError());

    if(_module!=NULL)
        _module->paintGL();

    glPopMatrix();
}

void FrameViewer::mousePressEvent(QMouseEvent *e)
{
    if(_module!=NULL)
        _module->mousePressEvent(e);
}

void FrameViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if(_module!=NULL)
        _module->mouseReleaseEvent(e);
}

void FrameViewer::mouseMoveEvent(QMouseEvent *e)
{
    if(_module!=NULL)
        _module->mouseMoveEvent(e);
}

void FrameViewer::keyPressEvent(QKeyEvent * e)
{
    if(_module!=NULL)
        _module->keyPressEvent(e);
}

void FrameViewer::wheelEvent(QWheelEvent *e)
{
    //Only work when Ctrl press, whell Up and Down events
    if (e->modifiers().testFlag(Qt::ControlModifier) &&
        e->orientation() == Qt::Vertical)
    {
        float zoomLast = _zoom;
        if (e->delta() > 0)     //Mouse Wheel Event: UP
            _zoom += 1.f;
        else                    //Mouse Wheel Event: DOWN
            _zoom -= 1.f;
        if (_zoom <= 1.f)
            resetZoom();
        else
        {
            Vec2f lowerLeft(_center.x() - float(_w) / (2.f * zoomLast),
                    _center.y() - float(_h) / (2.f * zoomLast));
            _center.Set(lowerLeft.x() + float(e->x()) / zoomLast,
                    lowerLeft.y() + float(e->y()) / zoomLast);
            //_off = true;
            //_draw->reportZoom(_zoom);
            glPixelZoom(_zoom, _zoom);
            zoomUpdateGL();
            updateGL();
        }
    }
}

void FrameViewer::resetZoom()
{
    _zoom = 1.f;
    //_draw->reportZoom(_zoom);
    glPixelZoom(_zoom, _zoom);
    _center.Set(_w / 2.f, _h / 2.f);
    //_off = false;
    myGlLoadIdentity();
    updateGL();
}

void FrameViewer::zoomUpdateGL()
{
    myGlLoadIdentity();
    glTranslatef(_w / 2., _h / 2., 0);
    glScalef(_zoom, _zoom, 1.);
    glTranslatef(-_center.x(), -_center.y(), 0);
}
