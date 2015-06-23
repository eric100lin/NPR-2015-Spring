#ifndef ABSTRACTPATH_H
#define ABSTRACTPATH_H

#include <assert.h>
#include <stdio.h>
#ifdef _MSC_VER
#include "windows.h"
#include <gl/gl.h>
#include <gl/GLU.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <qdatastream.h>
#include "dynarray.h"
#include "jl_vectors.h"
#include "KLT/TalkFitCurve.h"
#include "KLT/BezSpline.h"
#include "Bboxf2D.h"

class AbstractPath: public DynArray<Vec2f, 100>
{

public:

    AbstractPath();
    AbstractPath(const AbstractPath& other);

    virtual ~AbstractPath();

    Vec2f tangent(const int i) const;

    void smooth();
    void sharpen();
    virtual void fair()
    {
        smooth();
        sharpen();
    }

    void renderPS(FILE* fp) const;
    void renderDirection() const;

    void print() const;

    double distanceTo2(const Vec2f pt, int& index) const;
    double distanceTo2(const Vec2f pt) const;
    // these are also 2
    virtual double distanceToEnd(const Vec2f pt) const;
    virtual double distanceToStart(const Vec2f pt) const;

    virtual void modifyEnd(const int which, const Vec2f newloc);

    void mutualInit();

    void resample(const int* numSamples, DynArray<float, 100>* interpMe);

    // pointer documentation
    int* getPtrDoc()
    {
        return _ptrDoc;
    }
    const int* getPtrDoc() const
    {
        return _ptrDoc;
    }
    void setPtrDoc(const int frame, const int num)
    {
        _ptrDoc[0] = frame;
        _ptrDoc[1] = num;
    }

    void writePtr(FILE* fp) const;
    void writePtr(QDataStream* fp) const;

    virtual void translate(const Vec2f& delta);
    virtual void fixLoc();

    const Vec2f* getData() const
    {
        return data;
    }
    Vec2f* getData()
    {
        return data;
    }

    virtual void nudge(const int i, const Vec2f& loc);

    //void centroidBbox(Vec2f* cent, Vec2f* dims) const;
    //void getCentroid(Vec2f& here) const;

    static int _globalh;
    static TalkFitCurve _talkFit;
    static int _nudgeRadius;

private:

    void filterOffset(const int i, Vec2f& here) const;

protected:

    // pointer documentation
    int _ptrDoc[2]; // frame number, num in list

    static void writeNullPtr(FILE* fp);
    static void writeNullPtr(QDataStream* fp);

    static void writeIntArray(FILE* fp, int* array, int size);
    static void writeIntArray(QDataStream* fp, int* array, int size);

    void readIntArray(FILE* fp, int* array, int size);
    void readIntArray(QDataStream* fp, int* array, int size);

    static void writeBool(QDataStream *fp, bool b);
    static void readBool(QDataStream *fp, bool& b);

    Vec2f *_tangents; // TO DO:  get rid of this
    bool _inMotion;
    Vec2f _motion;
    //mutable Vec2f _centroid;
    //mutable bool _centroidCalculated;

};

#endif
