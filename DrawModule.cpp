#include "DrawModule.h"
#include "RotoscopeModule.h"
#include "frameviewer.h"
#include <assert.h>
#include <QColorDialog>

DrawModule::DrawModule(FrameViewer *parent, int length)
{
    _parent = parent;
    _toolMode = D_DRAW;
    _corrShow = false;
    _currDPath = NULL;
    _selected = NULL;
    _spliceIndex = _spliceFlag =-1;
    _captureColor = false;
    _mousePressed = 0;
    _freeDraw = true;
    _dragDirty = false;

    _frame = 0;
    _sens = 30.f;
    _pressure = 20.f;
    _currColor.Set(0,0,0);
    _dAlpha=1.f;
    _renderMode=0;
    _drawCurvesArray = new DrawCurves[length+1];
    _dc = _drawCurvesArray;
    mutualInit();
}

DrawModule::~DrawModule()
{
}

void DrawModule::paintGL()
{
    if (_dAlpha==0)
    {
        if (_selected)
            _selected->render(_renderMode);
        if (_currDPath)
            _currDPath->render(_renderMode,false);
        return;
    }
    _dc->renderAllPaths(_renderMode);

    // debug SKQ -- ??? making corresponded strokes aqua green
    glColor3f(1,0,0);
    if (_corrShow && _currDPath)
    {
        _currDPath->renderCorr();
    }

    if (_showAllCorr)
        _dc->renderAllCorr();

    if (_selected)
    {
        glColor4f(1,0,0, _dAlpha);
        _selected->renderSelected();
    }

    if (_currDPath)
        _currDPath->render(_renderMode,false);

    if (_captureColor)
        captureColor();
}

void DrawModule::mousePressEvent(QMouseEvent *e)
{
    _mousePressed = 1;
    if (_toolMode == D_DRAW)
    {
        if (_corrShow)
        {
            _corrShow = 0;
            _currDPath = NULL;
        }
        if (_currDPath != NULL)
            return;

        _pressure = 20.f;
        _currDPath = new DrawPath();
        _currDPath->setColor(_currColor);
        _currDPath->initHertzStroke(); // comment out to avoid Hertzman strokes
    }
    else if (_toolMode==D_SELECT)   // select
    {
        int which; // 0 for patch, 1 for path, -1 for neither
        void* selected = _dc->pickPrimitive(e->x(), e->y(), &which);
        _dragLoc = unproject(e->x(), e->y(), _parent->_h);
        if (selected)
        {
            if (which==0)
            {
                //_selectedPatch = (DrawPatch*) selected;
                _selected = NULL;
            }
            else
            {
                _selected = (DrawPath*) selected;
                //_selectedPatch = NULL;
            }
        }
        else
        {
            _selected = NULL;
        }
        emit selectChanged();
    }
    _parent->updateGL();
}

void DrawModule::mouseReleaseEvent(QMouseEvent *e)
{
    _mousePressed = 0;
    if (_toolMode == D_DRAW && _currDPath) // draw
    {
        if (_currDPath->getNumElements() > 1)
        {
            _parent->setCursor(Qt::WaitCursor);
            Vec2f loc = unproject(e->x(), e->y(), _parent->_h);
            _currDPath->addVertex(loc.x(), loc.y(), mapPressure());
            _dc->addPath(_currDPath);
            _currDPath->resample(NULL);
            _currDPath->redoStroke();

            bool res=false;
            if (!_freeDraw)
            {
                RotoCurves *rotoc = _parent->roto->_rotoCurvesArray+_frame;;
                res = _currDPath->correspondRoto2(rotoc, _parent->_w, _parent->_h);
            }
            _currDPath->calculateStrokeDisplayList();
            if (res)
            {
                _corrShow = 1;
                propagatePathEverywhere(_currDPath);
            }
            else
                _currDPath = NULL;
            _parent->setCursor(Qt::ArrowCursor);
        }
        else
        {
            delete _currDPath;
            _currDPath = NULL;
        }
    }
    else if(_toolMode == D_SELECT)  // select
    {
        if (_selected && _dragDirty)
        {
            regeneratePath(_selected);
            _selected->fixLoc();
            _dragDirty = false;
        }
    }
}

void DrawModule::mouseMoveEvent(QMouseEvent *e)
{
    if ((_toolMode==D_DRAW) && _mousePressed && _currDPath)
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        //printf("%f %f\n",newLoc.x(), newLoc.y());
        if (_currDPath->getNumElements()==0 || _currDPath->distToLast2(newLoc) > 0)
            _currDPath->addVertex(newLoc.x(),newLoc.y(), mapPressure());
        QPoint curs = _parent->mapFromGlobal(QCursor::pos());
        if ((e->x() - curs.x())*(e->x() - curs.x()) +
            (e->y() - curs.y())*(e->y() - curs.y()) < 25)
            _parent->updateGL();
    }
    else if (_toolMode == D_SELECT && _mousePressed && _selected)
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        Vec2f delta(newLoc, _dragLoc);
        _selected->translate(delta);
        _dragLoc = newLoc;
        _dragDirty = true;
        _parent->updateGL();
    }
}

void DrawModule::keyPressEvent (QKeyEvent *e)
{

}

void DrawModule::frameChange(int i)
{
    if (i!=_frame)
    {
        if (_corrShow)
        {
            _corrShow = 0;
            _currDPath = NULL;
        }

        // Interpolate forwards any just drawn curves
        if (_propMode == 0)
        {
            if (i-_frame == 1)
            {
                DrawPath* curr;
                _dc->startDrawPathIterator();
                while ((curr=_dc->IterateNext()) != NULL)
                    if (curr->justDrawn() && !curr->nextC())
                        curr->interpolateForwards(i);

                if (_selected)
                    _selected->interpolateForwards(i);
            }
        }
        _spliceFlag=-1;
        _dc->setJustDrawn(0);
        if (_selected!=NULL)
        {
            _selected = NULL;
            emit selectChanged();
        }
        //_selectedPatch = NULL;

        _frame = i;
        _dc = _drawCurvesArray+_frame;
    }
}

void DrawModule::toolChange(DrawTool id)
{
    _spliceFlag=-1;
//    if (_toolMode==D_SELECT && id!=D_SELECT && _justPasted)
//        finishJustPasted();

    _toolMode = id;
    if (_corrShow)
    {
        _corrShow = false;
        _currDPath = NULL;
    }

    if (id==D_DRAW)
    {
        if (_selected)
        {
            _selected = NULL;
            emit selectChanged();
        }
    }
    _parent->updateGL();
}

void DrawModule::propagatedStroke()
{
    corrPropAll();
}

void DrawModule::corrPropAll()
{
    _parent->setCursor(Qt::WaitCursor);
    _dc->startDrawPathIterator();
    DrawPath* curr;
    while ((curr=_dc->IterateNext()) != NULL)
    {
        if (!curr->corrToRoto())
        {
            RotoCurves *crc = _parent->roto->_rotoCurvesArray+_frame;
            curr->correspondRoto2(crc, _parent->_w, _parent->_h);
        }
        if (curr->nextC() == NULL || curr->prevC() == NULL)
            propagatePathEverywhere(curr);
    }
    _parent->setCursor(Qt::ArrowCursor);
}

QRgb DrawModule::getCurrColor()
{
    QRgb initC = qRgb(int(_currColor.r()*255.f),
                      int(_currColor.g()*255.f),
                      int(_currColor.b()*255.f));
    return initC;
}

void DrawModule::setColor(const Vec3f& col)
{
    _currColor = col;
}

void DrawModule::captureColor()
{
    glReadPixels(_captureColorLoc.x(), _captureColorLoc.y(),
                 1,1, GL_RGB, GL_FLOAT, (void*)&_currColor);
    QRgb rgb = qRgb(int(_currColor.r()*255.), int(_currColor.g()*255.), int(_currColor.b()*255.));
    QColorDialog::setCustomColor(15, rgb);
    emit currColorChanged();
}

float DrawModule::mapPressure() const
{
    float res = _pressure * (_sens+5.f)*.0025;
    return res;
}

void DrawModule::propagatePathEverywhere(DrawPath* path)
{
    _parent->setCursor(Qt::WaitCursor);
    //RotoPath **currRotos = new RotoPath*[path->getNumElements()];
    bool kosher = true;

    DrawPath* currD;
    int frame = _frame, shit, i;

    // forwards
    kosher = path->getCorrRoto()->nextRotosExist();

    currD = path;
    if (path->nextC()==NULL)
    {
        while (kosher)
        {
            frame++;
            DrawPath* newPath = new DrawPath();
            newPath->setFixed(false);
            newPath->copyLook(path);
            DrawCurves *ptrDC = _drawCurvesArray + frame;
            ptrDC->addPath(newPath);
            newPath->setPrevC(currD);
            currD->setNextC(newPath);
            DrawPath::fillForwardInterpolatedCurve(newPath,currD);
            currD = newPath;
            kosher = currD->getCorrRoto()->nextRotosExist();

            currD->redoStroke(); // HM?
            currD->freshenAppearance();

        } // end while
    }
    if (frame > _frame && 0) //G!
        optimizeDrawShape(path, currD, _frame, frame, false);
    _parent->setCursor(Qt::ArrowCursor);
}

void DrawModule::optimizeDrawShape(DrawPath* aPath, DrawPath* bPath, int aFrame, int bFrame, bool pinLast)
{
    assert(bFrame > aFrame);
    int numP = aPath->getNumElements(), numFrames = bFrame-aFrame, numVec, t, nc;

    if (pinLast)
    {
        if (numFrames<2)
            return;
        numVec = numP*(numFrames-1);
        if (bPath->getNumElements()!=aPath->getNumElements())
            nc = numFrames-1;
        else
            nc=numFrames;
    }
    else
    {
        numVec=numP*numFrames;
        nc = numFrames;
    }

    Vec2f *Plocs = new Vec2f[numP];
    Vec2f *Z = new Vec2f[numP * numFrames], *tZ;
    Vec2f *Sb = new Vec2f[numP * numVec], *tSb;
    memcpy(Plocs,aPath->getData(), numP*sizeof(double));
    DrawPath* curr = aPath;

    for (t=0, tZ=Z, tSb=Sb; t<nc; ++t, tZ+=numP, tSb+=numP)
    {
        curr = curr->nextC();
        assert(curr);
        assert(curr->getNumElements() == numP);
        assert(curr->getShouldBe() && curr->getShouldBe()->getNumElements()==numP);
        memcpy(tZ, curr->getData(), numP*sizeof(Vec2f));
        memcpy(tSb, curr->getShouldBe()->getData(), numP*sizeof(Vec2f));
    }

    if (nc==numFrames-1) // last frame might have different number of points
    {
        curr = curr->nextC();
        assert(curr && curr==bPath);
        int theirnp;
        int* t1d1 = curr->getT1D1(&theirnp);
        assert(t1d1 && theirnp==numP);
        for (int j=0; j<numP; ++j, ++tZ)
            (*tZ) = bPath->getElement(t1d1[j]);
    }

    _parent->roto->globalTC.longCurveReshape(numFrames, numP, Plocs, Z, Sb, pinLast);

    curr = aPath;
    for (t=0, tZ=Z; t<nc; ++t, tZ+=numP)
    {
        curr = curr->nextC();
        memcpy(curr->getData(), tZ, numP*sizeof(Vec2f));
        curr->redoStroke();
        curr->freshenAppearance();
    }

    delete[] Plocs; delete[] Z; delete[] Sb;
}

void DrawModule::regeneratePath(DrawPath* path)
{
    _parent->setCursor(Qt::WaitCursor);
    DrawPath *aPath, *bPath, *curr = path;
    int aFrame = _frame-1, bFrame = _frame+1;

    curr = path->prevC();
    while (curr && !curr->fixed() && curr->prevC() != NULL)
    {
        curr = curr->prevC();
        aFrame--;
    }
    aPath = curr;

    curr = path->nextC();
    while (curr && !curr->fixed() && curr->nextC() != NULL)
    {
        curr = curr->nextC();
        bFrame++;
    }
    bPath = curr;
    _parent->setCursor(Qt::ArrowCursor);
}
