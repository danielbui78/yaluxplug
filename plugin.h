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

struct G
{
    DzOptionsFrame *optFrame;
    DzProgress *RenderProgress;
    DzNode *currentNode;
    bool inProgress;
    int totalFrames;
    int frame_counter;
    int activeFrame;
    int endFrame;
    bool bDefaultLightsOn;
    bool bIsSpotRender;
    QRect cropWindow;

    DzRenderHandler *handler; // is only alive during render() function
    DzRenderSettings *settings; // used when calling node->render
    DzRenderOptions options;

    QString LuxExecPath;
    QString CmdLineArgs;
    bool bShowLuxRenderWindow;
    bool bSaveAlphaChannel;
    int haltAtTime;
    int haltAtSamplesPerPixel;
    float haltAtThreshold;
    int debugLevel;
    float cameraGamma;
    float cameraFstop;
    float cameraExposureTime;
    int cameraISO;
    QString LuxToneMapper;
    QStringList slaveNodeList;

    QString cachePath;
    QString tempPath;
    QString tempFilenameBase;
    QString workingRenderFilename;
    QStringList tokenList;
    DzTokenBuffer tokenBuffer;
    QProcess *luxRenderProc;
    int tempCounter;
};

extern struct G YaLuxGlobal;


#endif // YALUX_PLUGIN_H