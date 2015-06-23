#ifndef DRAWCURVES_H
#define DRAWCURVES_H

#include <QDataStream>
#include "LList.h"
#include "DrawPath.h"
//#include "drawPatch.h"

class DrawPath;
class DrawPatch;

class DrawCurves
{

public:

    DrawCurves();
    void setFrame(int f)
    {
        _frame = f;
    }

    void addPath(DrawPath* dp);
    //void addPatch(DrawPatch* dp) {
    // _patches.AddToTail(dp);
    //}

    int getNumCurves() const
    {
        return _paths.getSize();
    }
    //int getNumPatches() const { return _patches.getSize(); }

    void renderAllPaths(const int renderMode);
    //void renderAllPatches();

    void renderAllCorr();

    void startDrawPathIterator()
    {
        _paths.SetIterationHead();
    }
    //void startDrawPatchIterator() {
    //_patches.SetIterationHead();
    //}

    void deletePath(DrawPath* dp);
    void deleteAll();

    //void deletePatch(DrawPatch *dch);

    //DrawPath* pickCurve(const int x, const int y);
    //DrawPatch* pickPatch(const int x, const int y);
    void* pickPrimitive(const int x, const int y, int* which);

    DrawPath* IterateNext();
    //DrawPatch* IterateNextPatch() {
    //return _patches.IterateNext(); }

    DrawPath* getCurve(const int i);
    //DrawPatch* getPatch(const int i);

    void setJustDrawn(const int i);
    void documentSelf();

    void addNPaths(const int i);
    //void addNPatches(const int i);

    void MoveToTop(DrawPath* dp);
    void MoveToBottom(DrawPath* dp);

    void MoveOneUp(DrawPath* dp);
    void MoveOneDown(DrawPath* dp);

    DrawPath* cycle(DrawPath* p);

    Vec2i calcStat();

    //void setActivePatch(DrawPatch* a) { _activePatch = a; }
    //DrawPatch *getActivePatch() { return _activePatch; }

private:

    LList<DrawPath> _paths;
    //LList<DrawPatch> _patches;

    int _frame;
    //DrawPatch *_activePatch;
};

#endif
