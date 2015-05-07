//
//  utility_classes.h
//  yaluxplug
//
//  Created by Daniel Bui on 5/5/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#ifndef __yaluxplug__utility_classes__
#define __yaluxplug__utility_classes__

#include "dzoptionsframe.h"
#include "dzrendersettings.h"
#include "dzrenderoptions.h"
#include "dzprogress.h"
#include "dzrenderhandler.h"
#include "dznode.h"
#include <QtCore/QThread>
#include "dztexture.h"

const QStringList LXSfilm = QStringList() <<
"Film \"fleximage\"\n" <<
"\t\"integer outlierrejection_k\"\t[3]\n" <<
"\t\"bool premultiplyalpha\"\t[\"false\"]\n" <<
"\t\"float gamma\"\t[2.2]\n" <<
"\t\"integer displayinterval\"\t[5]\n" <<
"\t\"integer writeinterval\"\t[5]\n" <<
"\t\"string tonemapkernel\"\t[\"autolinear\"]\n" <<
//png
"\t\"bool write_png\"\t[\"true\"]\n" <<
"\t\"string write_png_channels\"\t[\"RGB\"]\n" <<
"\t\"bool write_png_16bit\"\t[\"false\"]\n" <<
"\t\"bool write_png_gamutclamp\"\t[\"true\"]\n" <<
//tga...
//exr...
//flm...
//colorspace...
"\t\"bool debug\"\t[\"true\"]\n";

const QStringList LXSpixelfilter = QStringList() <<
"PixelFilter \"mitchell\"\n" <<
"\t\"float xwidth\"\t[1.5]\n" <<
"\t\"float ywidth\"\t[1.5]\n" <<
"\t\"float B\"\t[0.3333]\n" <<
"\t\"float C\"\t[0.3333]\n" <<
"\t\"bool supersample\"\t[\"true\"]\n";

const QStringList LXSaccelerator = QStringList() <<
"Accelerator \"qbvh\"\n" <<
"\t\"integer maxprimsperleaf\"\t[4]\n" <<
"\t\"integer fullsweepthreshold\"\t[16]\n" <<
"\t\"integer skipfactor\"\t[1]\n";

const QStringList LXSsampler = QStringList() <<
"Sampler \"metropolis\"\n" <<
"\t\"float largemutationprob\"\t[0.40]\n" <<
"\t\"bool noiseaware\"\t[\"false\"]\n" <<
"\t\"bool usecooldown\"\t[\"false\"]\n";

const QStringList LXSsurfaceintegrator = QStringList() <<
"SurfaceIntegrator \"path\"\n" <<
"\t\"integer maxdepth\"\t[16]\n" <<
"\t\"string lightstrategy\"\t[\"one\"]\n" <<
"\t\"bool includeenvironment\"\t[\"true\"]\n";

const QStringList LXSvolumeintegrator = QStringList() <<
"VolumeIntegrator \"multi\"\n" <<
"\t\"float stepsize\"\t[1.0]\n";

const QStringList classNamesProperties = QStringList() << "DzStringProperty" << "DzColorProperty" << "DzFloatProperty" << "DzIntProperty" <<  "DzNodeProperty" << "DzImageProperty" ;

QString LuxProcessObject(DzObject *daz_obj);
QString LuxProcessMaterial(DzMaterial *material, QString &mesg, QString matLabel);
QString LuxProcessProperties(DzElement *el, QString &mesg);
QString LuxProcessLight(DzLight *currentLight, QString &mesg);
QString LuxMakeSceneFile(QString filenameLXS, DzRenderer *r, DzCamera *camera, const DzRenderOptions &opt);

class WorkerPrepareImage : public QObject
{
    Q_OBJECT
    
public:
    WorkerPrepareImage(const DzTexture *arg_i, const QString &arg_f)
    {   
        img = arg_i;
        filename = arg_f;
    };
    QThread *myThread;
    
    public slots:
    void doPrepareImage();
    
signals:
    void prepareImageComplete( WorkerPrepareImage *worker, const DzTexture *img, const QString &filename);
    void finished();
    
private:
    const DzTexture *img;
    QString filename;
    
};




#endif /* defined(__yaluxplug__utility_classes__) */
