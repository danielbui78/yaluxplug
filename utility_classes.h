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

#include "luxcore/DzMaterialToLuxCoreMaterial.h"

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

bool LuxGetBoolProperty(DzElement* el, QString propertyName, bool& prop_val, QString& mesg);
bool LuxGetIntProperty(DzElement* el, QString propertyName, int& prop_val, QString& mesg);
bool LuxGetFloatProperty(DzElement* el, QString propertyName, float& prop_val, QString& mesg);
QString LuxGetStringProperty(DzElement* el, QString propertyName, QString& mesg);
QString LuxGetImageMapProperty(DzElement* el, QString propertyName, QString& mesg);
QString propertyNumericImagetoString(DzNumericProperty* prop);
int whichClass(QObject* obj, const QStringList& classNames);
QString propertyValuetoString(DzProperty* prop);

//QString LuxProcessObject(DzObject *daz_obj, QString &mesg);
//QString LuxProcessGenMaterial(DzMaterial *material, QString &mesg, QString matLabel);
//QString LuxProcessGlossyMaterial(DzMaterial *material, QString &mesg, QString matLabel);
//QString LuxProcessMatteMaterial(DzMaterial *material, QString &mesg, QString matLabel);
//QString LuxProcessProperties(DzElement *el, QString &mesg);
//QString LuxProcessLight(DzLight *currentLight, QString &mesg);
//QString LuxCoreProcessLight(DzLight* currentLight, QString& mesg);
//bool LuxMakeLXSFile(QString filenameLXS, DzRenderer *r, DzCamera *camera, const DzRenderOptions &opt);

//bool LuxMakeCFGFile(QString filenameLXS, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt);
//bool LuxMakeSCNFile(QString filenameLXS, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt);
//QString LuxCoreProcessObject(DzObject* daz_obj, QString& mesg);
//QString LuxCoreProcessDazDefaultMaterial(DzMaterial* material, QString& mesg, QString matLabel);
//QString LuxCoreProcessOmUberSurfaceMaterial(DzMaterial* material, QString& mesg, QString matLabel);
//QString LuxCoreProcessIrayUberMaterial(DzMaterial* material, QString& mesg, QString matLabel);

QString GenerateCoreTextureBlock3(QString textureName, QString mapName, float textureValue1, float textureValue2, float textureValue3,
    float uscale = 1, float vscale = -1, float uoffset = 0, float voffset = 0, float gamma = 2.2,
    QString wrap = "", QString channel = "rgb");
QString GenerateCoreTextureBlock1(QString textureName, QString mapName, float textureValue,
    float uscale = 1, float vscale = -1, float uoffset = 0, float voffset = 0, float gamma = 2.2,
    QString wrap = "", QString channel = "mean");
QString GenerateCoreTextureBlock1_Grey(QString textureName, QString mapName, float textureValue,
    float uscale = 1, float vscale = -1, float uoffset = 0, float voffset = 0, float gamma = 1.0,
    QString wrap = "", QString channel = "mean");

/* Returns textureblock named: "<texturename>_cutoff_feathered" */
QString CreateFeatheredCutOffTexture(QString texturename, QString cutoffFunction, double cutoff_threshold, double feather_edge_amount);

/* Returns texture name for mask: "<texture_block.name>_mask" */
QString GenerateMask(DzMaterialToLuxCoreMaterial::TextureBlock &texture_block, QString cutoff_function="greaterthan", double cutoff_threshold=0.25, double feather_amount=0.02, double noise_strength=1.0);

QString SanitizeCoreLabel(QString label);
QString LuxGetImageMapProperty(DzElement* el, QString propertyName, QString& mesg);
QString MakeTempImgFilename(QString origFilename);

double gammaUnCorrect(double x);
double gammaCorrect(double x);

double GetRed(QColor color);
double GetGreen(QColor color);
double GetBlue(QColor color);


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

struct VolumeData
{
    QString name = "";
    QString type = "";
    int transmission_color = 0;
    float transmission_distance = 1;
    int scattering_color = 0;
    float scattering_distance = 1;
    float asymmetry_val = 0;
    bool    multiscattering = 1;

    friend bool operator==(const VolumeData& a, const VolumeData& b);

};

class Worker_UpdateInfoWindow : public QObject
{
    Q_OBJECT

public:
    QThread* myThread;
    void processCoreRenderLog();

public slots:
    void doUpdate();

signals:
    void finished();
    void updateLogWindow(QString data, QColor textcolor = QColor(255, 255, 255), bool bIsBold = false);
    void updateData();

};

#endif /* defined(__yaluxplug__utility_classes__) */
