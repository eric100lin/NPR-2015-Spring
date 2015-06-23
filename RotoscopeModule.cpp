#include "RotoscopeModule.h"
#include "frameviewer.h"
#include <assert.h>
#include <QDebug>

RotoscopeModule::RotoscopeModule(FrameViewer *parent,
                                 cv::VideoCapture *capture, int length)
{
    _parent = parent;
    _capture = capture;
    _currPath = NULL;
    _ctrlDrag = NULL;
    _dragCtrlNum = -1;
    _nowFrame = 0;
    _length = length;
    _lastFrameTouched = -1;
    _changeLastFrameTouched = -1;
    _tracking = false;
    _isCorrShow = false;
    _manualStage = false;
    _visEffort = true;
    _showTrackPoints = true;
    _toolMode = T_MANUAL;
    _rotoCurvesArray = new RotoCurves[length+1];
    _Cpyrms = new KLT_FullCPyramid[length+1];
    _pyrmsE = new KLT_FullPyramid[length+1];
    _currRC = _rotoCurvesArray;
    mutualInit();
}

RotoscopeModule::~RotoscopeModule()
{
    if(_capture!=NULL)
    {
        _capture->release();
        delete _capture;
    }
}

void RotoscopeModule::frameChange(int i)
{
    if (!_selected.empty())
    {
        _selected.clear();
        emit enablePbCopySplinesAcrossTime(false);
    }
    if (_toolMode == T_MANUAL && _currPath)
    {
        finishManualRotoCurve();
    }
    if (_changeLastFrameTouched > -1)
    {
        _lastFrameTouched = _changeLastFrameTouched;
        _changeLastFrameTouched = -1;
    }
    _nowFrame = i;
    _currRC = &_rotoCurvesArray[i];
    if (_isCorrShow)
    {
        _isCorrShow = false;
        _currPath = NULL;
    }
}

void RotoscopeModule::paintGL()
{
    if (_tracking)
    {
        list<TrackGraph*>::iterator tgc;
        list<MultiSplineData*>::iterator mtc;
        list<KLT_TrackingContext*>::iterator kc;
        for (tgc = _ccompV.begin(), mtc = _trackDataV.begin(), kc = _TCV.begin();
                tgc != _ccompV.end() && mtc != _trackDataV.end()
                        && kc != _TCV.end();)
        {

            (*mtc)->_z_mutex->lock();
            if ((*kc)->_stateOk)
                (*tgc)->copyLocs(*mtc);

            (*mtc)->_z_mutex->unlock();

            (*kc)->quit();
            if ((*kc)->isFinished())
            {
                delete *tgc;
                tgc = _ccompV.erase(tgc);
                delete *mtc;
                mtc = _trackDataV.erase(mtc);
                delete *kc;
                kc = _TCV.erase(kc);
            }
            else
            {
                ++tgc;
                ++mtc;
                ++kc;
            }

            if (_ccompV.size() == 0)
            {
                assert(_trackDataV.size() == 0 && _TCV.size() == 0);
                killTimer (_trackingTimer);
                _tracking = false;
                emit enablePbCopySplinesAcrossTime(false);
                break;
            }
        }
    }

    // paint older paths
    _currRC->renderAllPaths(_showTrackPoints, _visEffort);
    if (_currPath)
        _currPath->renderHandles();

    // draw correspondence lines
    if (_isCorrShow && _currPath && _currPath->prevC())
    {
        glColor3f(0, 1, 1);
        assert(_currPath->prevC() && _currPath->prevC()->nextC() == _currPath);
        _currPath->prevC()->renderNextCorrLines();
    }

    // render selection
    if (!_selected.empty())
    {
        glColor3f(1, 0, 0);
        for (PathV::const_iterator c = _selected.begin(); c != _selected.end();
                ++c)
            (*c)->renderSelected();
    }

//    qDebug() << "paintGL()";
}

void RotoscopeModule::mousePressEvent(QMouseEvent *e)
{
    if (_isCorrShow)
    {
        assert(_currPath);
        _currPath = NULL;
        _isCorrShow = false;
    }
    if (_toolMode == T_MANUAL)      // manual
    {
        bool curveStart = (_currPath == NULL);
        if (curveStart)
        {
            _currPath = new RotoPath();
            _currRC->addPath(_currPath);
            _currPath->startManualCreation();
            _manualStage = 0;
        }
        _parent->setMouseTracking(1);
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        if (_manualStage == 0)
        {
            _currPath->addCtrl(newLoc);
            _currPath->addCtrl(newLoc);
        }
        else
        {
            _currPath->addCtrl(newLoc);
            _currPath->addCtrl(newLoc);
            _currPath->addCtrl(newLoc);
        }
        _parent->updateGL();
    }
    else if (_toolMode == T_SELECT)  // select
    {
        RotoPath* pick = _currRC->pickPrimitive(e->x(), e->y());
        if (pick)
        {
            if (_shift) // multiple selection
            {
                if (find(_selected.begin(), _selected.end(), pick)
                        == _selected.end())
                    _selected.push_back(pick);
            }
            else // not multiple selection
            {
                _selected.clear();
                _selected.push_back(pick);
            }
        }
        else
        {
            _selected.clear();
            emit enablePbCopySplinesAcrossTime(false);
        }

        if (_selected.size() > 0)
        {
            _parent->setMouseTracking(1);
            _dragLoc = unproject(e->x(), e->y(), _parent->_h);
            emit enablePbCopySplinesAcrossTime(true);
        }

        _parent->updateGL();
        _changeLastFrameTouched = _nowFrame;
    }
    else if (_toolMode == T_NUDGE)  // nudge
    {
        // for now, implement endpoint mouse dragging
        Vec2f loc = unproject(e->x(), e->y(), _parent->_h);
        _ctrlDrag = _currRC->distanceToCtrls2(loc, &_dragCtrlNum);

        if (_ctrlDrag)
        {
            int whichEnd;
            if (_ctrlDrag->isCtrlEnd(_dragCtrlNum, &whichEnd))
                _ctrlDrag->fixEndpoint(whichEnd);
            else
                _ctrlDrag->fixInternal(_dragCtrlNum);

            _ctrlDrag->setCtrlTouched(_dragCtrlNum, true);
//            if (_tracking)
//                signalTracking();
            _parent->updateGL();
        }
    }
}

void RotoscopeModule::mouseReleaseEvent(QMouseEvent *e)
{
    if (_toolMode == T_MANUAL && _currPath)
    {
        _parent->setMouseTracking(0);
        _manualStage = true;
    }
    else if (_toolMode == T_SELECT && _selected.size() > 0)
    {
        PathV::iterator i;
        for (i = _selected.begin(); i != _selected.end(); ++i)
        {
            (*i)->handleNewBezCtrls();
        }
        _parent->setMouseTracking(0);
        _parent->updateGL();
    }
    else if (_ctrlDrag != NULL && _toolMode == T_NUDGE)   // endpoint nudge
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        _ctrlDrag->setControl(newLoc, _dragCtrlNum);
        int whichEnd;
        if (_ctrlDrag->isCtrlEnd(_dragCtrlNum, &whichEnd))
            _ctrlDrag->reconcileOneJointToMe(whichEnd);
        _ctrlDrag->handleNewBezCtrls();
//        if (_tracking)
//            restartPausedTracking();
        _ctrlDrag = NULL;
        _parent->updateGL();
    }
}

void RotoscopeModule::mouseMoveEvent(QMouseEvent *e)
{
    if (_toolMode == T_MANUAL && _currPath)
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        int n = _currPath->getNumControls();
        if (_manualStage == 0)
        {
            _currPath->setControl(newLoc, n - 1);
        }
        else
        {
            _currPath->setControl(newLoc, n - 1);
            // set -3 to reflection of -1 across -2
            Vec2f rloc(_currPath->getCtrlRef(n - 2),
                    _currPath->getCtrlRef(n - 1));
            rloc += _currPath->getCtrlRef(n - 2);
            _currPath->setControl(rloc, n - 3);
        }
        _parent->updateGL();
    }
    else if (_toolMode == T_SELECT && _selected.size() > 0)
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        Vec2f delta(newLoc, _dragLoc);
        PathV::iterator i;
        for (i = _selected.begin(); i != _selected.end(); ++i)
        {
            (*i)->translate(delta);
        }
        _dragLoc = newLoc;
        _parent->updateGL();
    }
    else if (_toolMode == T_NUDGE && _ctrlDrag != NULL)   // endpoint nudge
    {
        Vec2f newLoc = unproject(e->x(), e->y(), _parent->_h);
        _ctrlDrag->setControl(newLoc, _dragCtrlNum);
        int whichEnd;
        if (_ctrlDrag->isCtrlEnd(_dragCtrlNum, &whichEnd))
            _ctrlDrag->reconcileOneJointToMe(whichEnd);
        _ctrlDrag->handleNewBezCtrls();
        _parent->updateGL();
    }
}

void RotoscopeModule::keyPressEvent (QKeyEvent *e)
{

}

void RotoscopeModule::toolChange(TrackTool id)
{
    if (_toolMode == T_MANUAL && _currPath)
    {
        finishManualRotoCurve();
    }
    else    killAnyRoto();

    _toolMode = id;

    if (_isCorrShow)
    {
        _isCorrShow = false;
        _currPath = NULL;
    }

    _parent->updateGL();
}

void RotoscopeModule::setVisEffort(const bool s)
{
    _visEffort = s;
    _parent->updateGL();
}

void RotoscopeModule::setShowTrackPoints(const bool s)
{
    _showTrackPoints = s;
    _parent->updateGL();
}

void RotoscopeModule::copySplinesAcrossTime()
{
    int a = _nowFrame + 1, b = _length-1, i, j;
    if (!getRange(a, b))
        return;
    if (a != _nowFrame + 1)
        return;

    PathV::const_iterator c;
    PathV::const_iterator v;
    PathV prev;
    for (c = _selected.begin(); c != _selected.end(); ++c)
        prev.push_back(*c);
    PathV currPaths = prev, next = prev;

    // copy forward
    //a = MAX(a,_frame+1);
    for (i = a; i <= b; ++i)
    {
        for (v = prev.begin(), j = 0; v != prev.end(); ++v, ++j)
        {
            RotoPath* newpath = new RotoPath(**v);
            newpath->buildTouched(false);
            newpath->setFixed(false);
            _rotoCurvesArray[i].addPath(newpath);
            (*v)->setNextC(newpath);
            (*v)->buildINextCorrs();
            newpath->setPrevC(*v);
            newpath->buildIPrevCorrs();
            prev[j] = newpath;
        }
    }

    RotoPath::buildForwardReconcileJoints(currPaths, _nowFrame, b);
}

void RotoscopeModule::finishManualRotoCurve()
{
    _currPath->startFinishManualCreation();
    _currRC->setupJoints(_currPath);
    _currPath->finishManualCreation();
    _currPath = NULL;
}

void RotoscopeModule::killAnyRoto()
{
    _parent->setMouseTracking(0);
    if (_currPath)
    {
        _currPath->clearPrevCorrespondence();
        delete _currPath;
        _currPath = NULL;
    }
    _isCorrShow = false;
}

void RotoscopeModule::rejigWrapper(bool startOver)
{
    if (_selected.empty())
        return;
    emit enablePbCopySplinesAcrossTime(false);

    PathV paths, oPaths;
    paths = _selected;
    unsigned int i;
    bool fixed = false;

    for (i = 0; i < paths.size(); ++i)
    {
        if (paths[i]->fixed())
            fixed = true;
    }

    if (!fixed)
    {
        rejig(&paths, _nowFrame, startOver);
    }
    else
    {
        oPaths = paths;
        bool good = true;
        for (i = 0; i < oPaths.size(); ++i)
        {
            oPaths[i] = oPaths[i]->prevC();
            if (oPaths[i] == NULL)
                good = false;
        }
        if (good)
            rejig(&oPaths, _nowFrame - 1, startOver);

        good = true;
        oPaths = paths;
        for (i = 0; i < oPaths.size(); ++i)
        {
            oPaths[i] = oPaths[i]->nextC();
            if (oPaths[i] == NULL)
                good = false;
        }
        if (good)
            rejig(&oPaths, _nowFrame + 1, startOver);
    }

    _selected.clear();
}

void RotoscopeModule::rejig(PathV* paths, int frame, bool startOver)
{
    if (paths->empty())
        return;
    PathV aPaths, bPaths;
    aPaths = *paths;
    bPaths = aPaths;
    int aFrame = frame, bFrame = frame, numFrames;
    bool keepGoing = true;
    PathV::const_iterator c, c2;
    RotoPath *tmp;
    int i;

    while (keepGoing)
    {
        for (c = aPaths.begin(); c != aPaths.end(); ++c)
        {
            if ((*c)->fixed() || (*c)->prevC() == NULL)
                keepGoing = false;
        }
        if (keepGoing) // we're going back in time, step pointers, delete the left behind paths
        {
            for (c = aPaths.begin(), i = 0; c != aPaths.end(); ++c, ++i)
            {
                tmp = *c;
                aPaths[i] = tmp->prevC();
                //if (aFrame!=frame && startOver)  // don't delete frame paths, since next loop forwards will
                //_is->getRotoCurves(aFrame)->deletePath(tmp);
            }
            --aFrame;
        }
    }
    printf("aFrame is %d\n", aFrame);
    assert(aPaths.size() == paths->size());

    keepGoing = true;

    while (keepGoing)
    {
        for (c = bPaths.begin(); c != bPaths.end(); ++c)
        {
            if ((*c)->fixed() || (*c)->nextC() == NULL)
                keepGoing = false;
        }
        if (keepGoing) // we're going forward in time, step pointers, delete the left behind paths
        {
            for (c = bPaths.begin(), i = 0; c != bPaths.end(); ++c, ++i)
            {
                tmp = *c;
                bPaths[i] = tmp->nextC();
                //if (startOver)
                //_is->getRotoCurves(bFrame)->deletePath(tmp);
            }
            ++bFrame;
        }
    }

    printf("bFrame is %d\n", bFrame);
    assert(bPaths.size() == paths->size());
    numFrames = bFrame - aFrame;
    if (numFrames < 2)
    {
        printf("Too short a span\n");
        _toTrack.clear();
        return;
    }

    assert(_toTrack.empty());

    for (c = aPaths.begin(), c2 = bPaths.begin(); c != aPaths.end(); ++c, ++c2)
    {
        _toTrack.push_back(*c2);
    }

    if (startOver) // redoTrack
        performTracks(aFrame, bFrame, true, true);
    else
        performTracks(aFrame, bFrame, false, true);
}

void RotoscopeModule::performTracks(const int aFrame, const int bFrame, bool doInterp,
        bool useExistingInbetweens)
{
    printf("toTrack size %d, a %d b %d\n", _toTrack.size(), aFrame, bFrame);
    bool* done = new bool[_toTrack.size()];
    memset(done, 0, _toTrack.size() * sizeof(bool));
    PathV::const_iterator c2;
    PathV::iterator c;

    // interpolate somehow
    if (doInterp && !useExistingInbetweens)
    {
        for (c2 = _toTrack.begin(); c2 != _toTrack.end(); ++c2)
        {
            keyframeSedInterp((*c2)->prevC(), aFrame, *c2, bFrame);
        }
        printf("Interpolated\n");
    }

    // build & reconcile joints
    if (doInterp && !useExistingInbetweens)
        RotoPath::buildBackReconcileJoints(_toTrack, bFrame, aFrame);
    //ccomp->buildBackReconcileJoints(bFrame, aFrame);
    printf("Joints done\n");

    int i = 0, j;
    RotoscopeModule *_is = this;
    for (c = _toTrack.begin(); c != _toTrack.end(); ++c, ++i)
    {
        if (!done[i])
        {
            // get a full list beg to end of linked up paths (stop at already interpolated ones,
            // ones not in toTrack list)
            TrackGraph* ccomp = new TrackGraph();
            MultiSplineData* mts = new MultiSplineData();

            (*c)->buildccomp(ccomp, &_toTrack);

            // start multiTrack
            mts->_numFrames = bFrame - aFrame;
            printf("started multi\n");

            ccomp->buildMulti(mts);

            addMasksToMulti(mts, ccomp->getKey0Paths(), aFrame);

            // track
            // pyrms and pyrmsE get deleted by longCurveTrack2
            const KLT_FullCPyramid** pyrms =
                    new const KLT_FullCPyramid *[mts->_numFrames + 1];
            const KLT_FullPyramid** pyrmsE =
                    new const KLT_FullPyramid *[mts->_numFrames + 1];

            int _startF = 0;
            for (j = 0; j <= mts->_numFrames; j++)
            {
                int a = aFrame + j;
                QImage img = getImage();
                if (_is->globalTC.useImage)
                {
                    KLT_FullCPyramid *fp = _Cpyrms + a - _startF;
                    if (!fp->img)
                    {
                        fp->initMe(img, &globalTC);
                        printf("Calculated frame %d\n", a);
                    }
                    pyrms[j] = fp;
                    assert(pyrms[j]);
                }
                else
                    pyrms[j] = NULL;

                if (_is->globalTC.useEdges)
                {
                    KLT_FullPyramid *fp = _pyrmsE + a - _startF;
                    if (!fp->img)
                    {
                        fp->initMeFromEdges(img, &globalTC);
                        printf("Calculated edge frame %d\n", a);
                    }
                    pyrmsE[j] = fp;
                    assert(pyrmsE[j]);
                }
                else
                    pyrmsE[j] = NULL;
            }
            assert(setImageIndex(0));   //Back to head

            RotoscopeModule *_is = this;
            KLT_TrackingContext* tc = new KLT_TrackingContext(); // transfer global track settings
            tc->copySettings(&(_is->globalTC));
            tc->setupSplineTrack(pyrms, pyrmsE, mts, doInterp);

            _ccompV.push_back(ccomp);
            _trackDataV.push_back(mts);
            _TCV.push_back(tc);

            tc->start(); // ONE

            if (!_tracking)
            {
                _tracking = true;
                emit enablePbCopySplinesAcrossTime(false);
                _trackingTimer = startTimer(200);
            }

            // Let done know these are processed
            for (c2 = ccomp->paths()->begin(); c2 != ccomp->paths()->end();
                    ++c2)
            {
                PathV::const_iterator which = find(_toTrack.begin(),
                        _toTrack.end(), *c2);
                assert(which != _toTrack.end());
                done[which - _toTrack.begin()] = true;
            }
        }
    }

    _toTrack.clear();
    delete[] done;
}

void RotoscopeModule::keyframeSedInterp(RotoPath* aPath, int aFrame, RotoPath *bPath,
        int bFrame)
{
    assert(aPath && bPath);
    assert (_propMode);
    int numFrames = bFrame - aFrame;
    assert(numFrames > 0);
    int t;

    bPath->setLowHeight(aPath->lowHeight());
    bPath->setHighHeight(aPath->highHeight());
    bPath->setTrackEdges(aPath->trackEdges());
    printf("KeyframeSedInterp\n");

    RotoPath* prev = aPath;
    for (t = 1; t < numFrames; ++t)
    {
        RotoPath *newpath = new RotoPath(*aPath);
        //newpath->setCan(t);
        newpath->setFixed(false);
        newpath->buildTouched(false);

        int a = aFrame + t;
        RotoCurves *curve = _rotoCurvesArray + a;
        curve->addPath(newpath);

        newpath->setPrevC(prev);
        prev->setNextC(newpath);
        if (t == 0)
            newpath->buildINextCorrs();
        else if (t == numFrames - 1 && globalTC.pinLast)
        {
            //newpath->_nextCoors = aPath->_nextCoors;
            newpath->takeNextCont(aPath);
            newpath->buildIPrevCorrs();
        }
        else
        {
            newpath->buildINextCorrs();
            newpath->buildIPrevCorrs();
        }
        prev = newpath;
    }

    bPath->setPrevC(prev);
    prev->setNextC(bPath);
    //aPath->forgetNextCont();
    aPath->buildINextCorrs();
    assert(aPath->nextC());
}

void RotoscopeModule::addMasksToMulti(MultiSplineData* mts, const PathV& key0,
        const int frame0)
{
    PathV currPaths = key0;
    PathV::iterator c;
    mts->setMaskDims(_parent->_w, _parent->_h);
    for (int i = 0; i < mts->_numFrames + 1; ++i) // iterate over frames
    {
        RotoCurves* curve = _rotoCurvesArray + frame0 + i;
        unsigned char* mask = curve->makeCummMask(currPaths);
        mts->addMask(mask);
        for (c = currPaths.begin(); c != currPaths.end(); ++c) // advance paths 1 frame
            (*c) = (*c)->nextC();
    }
}

QImage RotoscopeModule::getImage()
{
    cv::Mat frame, temp;
    _capture->read(frame);
    cvtColor(frame, temp,CV_BGR2RGB); // cvtColor Makes a copt, that what i need
    QImage img = QImage((const unsigned char*)(temp.data),
                        temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
    return img;
}

bool RotoscopeModule::setImageIndex(int index)
{
    return _capture->set(CV_CAP_PROP_POS_FRAMES, index);
}
