//
//  plugin.h
//  yaluplug
//
//  Created by Daniel Bui on 4/23/15.
//
//

#ifndef YALUX_PLUGIN_H
#define YALUX_PLUGIN_H

#include <QtCore/QProcess>

#include "dzoptionsframe.h"
#include "dzrendersettings.h"
#include "dzrenderoptions.h"
#include "dzprogress.h"
#include "dzrenderhandler.h"
#include "dznode.h"
#include <QtCore/QThread>
#include "dztexture.h"
#include "dztokenbuffer.h"

#include "utility_classes.h"

class QTextEdit;
class QTemporaryFile;

struct G
{
    DzRenderHandler *handler = NULL; // is only alive during render() function
    DzRenderSettings *settings = NULL; // used when calling node->render
    DzRenderOptions options;

    DzOptionsFrame *optFrame;
    QFrame *logWindow = NULL;
    QTextEdit *logText = NULL;
    DzProgress *RenderProgress = NULL;
    DzProgress *FrameProgress = NULL;
    DzBackgroundProgress *backgroundProgress;
    int numBackgroundThreads=0;
    float currentBackgroundProgress=0;

    DzNode *currentNode;
    bool inProgress;
    int totalFrames;
    int frame_counter;
    int activeFrame;
    int endFrame;
    bool bDefaultLightsOn;
    bool bIsSpotRender;
    QRect cropWindow;

    QList<QTemporaryFile*> PLYgarbageCollectionList;

    QString LuxExecPath;
    QString CmdLineArgs;
    bool bShowLuxRenderWindow;
    int maxTextureSize;
    bool bNetworkRenderOn;
    QStringList slaveNodeList;
    int renderMode;
    QString customRenderString;
    int debugLevel;
    bool bSaveAlphaChannel;
    int haltAtTime;
    int haltAtSamplesPerPixel;
    float haltAtThreshold;
    float tonemapGamma;
    float tonemapFstop;
    float tonemapExposureTime;
    int tonemapISO;
    QString LuxToneMapper;
    int specularMode;


    QString cachePath;
    QString tempPath;
    QString tempFilenameBase;
    QString workingRenderFilename;

    QProcess *luxRenderProc = NULL;
    bool bFrameisFinished;
    bool bIsCancelled;
    int tempCounter;

    QStringList tokenList;
    DzTokenBuffer tokenBuffer;
    
    bool bAddTonemapperAndEnvironement;

    bool bPreferNormal;
    bool bDoBumpMaps;
    bool bDoNormalMaps;
    bool bDoNormalAsBump;
    bool bDoMetallic;
    bool bDoSpecular;
    bool bDoRoughnessMaps;
    bool bDoTranslucency;
    bool bDoSSSVolume;
    bool bDoSSSAbsorption;
    bool bDoSSSScattering;
    bool bDoDebugSSS;

    QList<VolumeData*> VolumeList;
    QStringList matLookupTable;

    bool bAbortReceived = false;
};

extern struct G YaLuxGlobal;


#endif // YALUX_PLUGIN_H