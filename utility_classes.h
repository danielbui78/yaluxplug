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
"\t\"integer outlierrejection_k\"\t[1]\n" <<
//"\t\"bool premultiplyalpha\"\t[\"false\"]\n" <<
//"\t\"float gamma\"\t[2.2]\n" <<
//"\t\"integer displayinterval\"\t[12]\n" <<
"\t\"integer writeinterval\"\t[10]\n" <<
"\t\"integer flmwriteinterval\"\t[10]\n" <<
"\t\"string ldr_clamp_method\"\t[\"lum\"]\n" <<
//png
"\t\"bool write_png\"\t[\"true\"]\n" <<
//flm
"\t\"bool write_resume_flm\"\t[\"true\"]\n" <<
//"\t\"bool restart_resume_flm\"\t[\"false\"]";
//"\t\"bool write_png_16bit\"\t[\"false\"]\n" <<
//"\t\"bool write_png_gamutclamp\"\t[\"true\"]\n" <<
//tga...
//"\t\"bool write_tga\"\t[\"false\"]\n" <<
//exr...
"\t\"bool write_exr\"\t[\"true\"]\n" <<
"\t\"string write_exr_compressiontype\"\t[\"PIZ (lossless)\"]\n" <<
"\t\"string write_exr_zbuf_normalizationtype\"\t[\"None\"]\n" <<
"\t\"bool write_exr_ZBuf\"\t[\"true\"]\n";

const QStringList LXSpixelfilter = QStringList() <<
"PixelFilter \"mitchell\"\n";
//"\t\"float xwidth\"\t[1.5]\n" <<
//"\t\"float ywidth\"\t[1.5]\n" <<
//"\t\"float B\"\t[0.3333]\n" <<
//"\t\"float C\"\t[0.3333]\n" <<
//"\t\"bool supersample\"\t[\"true\"]\n";

const QStringList LXSaccelerator = QStringList() <<
"Accelerator \"qbvh\"\n";
//"\t\"integer maxprimsperleaf\"\t[4]\n" <<
//"\t\"integer fullsweepthreshold\"\t[16]\n" <<
//"\t\"integer skipfactor\"\t[1]\n";

const QStringList LXSsampler = QStringList() <<
"Sampler \"metropolis\"\n" <<
//"\t\"float largemutationprob\"\t[0.40]\n" <<
"\t\"bool noiseaware\"\t[\"true\"]\n" <<
"\t\"bool usevariance\"\t[\"true\"]\n" <<
"\t\"bool usecooldown\"\t[\"false\"]\n";

const QStringList LXSsurfaceintegrator = QStringList() <<
"SurfaceIntegrator \"path\"\n";
//"\t\"integer maxdepth\"\t[16]\n" <<
//"\t\"string lightstrategy\"\t[\"auto\"]\n" <<
//"\t\"bool directlightsampling\"\t[\"true\"]\n" <<
//"\t\"float rrcontinueprob\"\t[0.65]\n" <<
//"\t\"bool includeenvironment\"\t[\"true\"]\n";

const QStringList LXSvolumeintegrator = QStringList() <<
"VolumeIntegrator \"multi\"\n";
//"\t\"float stepsize\"\t[1.0]\n";

const QStringList classNamesProperties = QStringList() << "DzStringProperty" << "DzBoolProperty" << "DzColorProperty" << "DzFloatProperty" << "DzIntProperty" <<  "DzNodeProperty" << "DzImageProperty" ;

const QStringList areaLightPlane = QStringList() <<
"Shape \"mesh\"\n" <<
"\t\"point P\" [-2 -2 0 2 -2 0 2 2 0 -2 2 0]\n" <<
"\t\"normal N\" [0 0 1 0 0 1 0 0 1 0 0 1]\n" <<
//"\t\"float uv\" [0 0 1 0 1 1 0 1]\n" <<
"\t\"integer quadindices\" [0 1 2 3]\n";

const QStringList spotLightPlane = QStringList() <<
"Shape \"mesh\"\n" <<
"\t\"point P\" [-0.01 -0.01 0 0.01 -0.01 0 0.01 0.01 0 -0.01 0.01 0]\n" <<
"\t\"normal N\" [0 0 1 0 0 1 0 0 1 0 0 1]\n" <<
//"\t\"float uv\" [0 0 1 0 1 1 0 1]\n" <<
"\t\"integer quadindices\" [0 1 2 3]\n";

const QStringList distantLightPlane = QStringList() <<
"Shape \"mesh\"\n" <<
"\t\"point P\" [-5 -5 -10 5 -5 -10 5 5 -10 -5 5 -10]\n" <<
"\t\"normal N\" [0 0 1 0 0 1 0 0 1 0 0 1]\n" <<
//"\t\"float uv\" [0 0 1 0 1 1 0 1]\n" <<
"\t\"integer quadindices\" [0 1 2 3]\n";


QString LuxProcessObject(DzObject *daz_obj, QString &mesg);
QString LuxProcessGenMaterial(DzMaterial *material, QString &mesg, QString matLabel);
QString LuxProcessGlossyMaterial(DzMaterial *material, QString &mesg, QString matLabel);
QString LuxProcessMatteMaterial(DzMaterial *material, QString &mesg, QString matLabel);
QString LuxProcessProperties(DzElement *el, QString &mesg);
QString LuxProcessLight(DzLight *currentLight, QString &mesg);
QString LuxMakeSceneFile(QString filenameLXS, DzRenderer *r, DzCamera *camera, const DzRenderOptions &opt);

class WorkerPrepareImage : public QObject
{
    Q_OBJECT
    
public:
    WorkerPrepareImage(const DzTexture *arg_image, const QString &arg_filename)
    {   
        img = arg_image;
        filename = arg_filename;
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
