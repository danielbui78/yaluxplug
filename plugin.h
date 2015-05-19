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
    DzRenderHandler *handler; // is only alive during render() function
    DzRenderSettings *settings; // used when calling node->render
    DzRenderOptions options;

    DzOptionsFrame *optFrame;
    QFrame *logWindow;
    QTextEdit *logText;
    DzProgress *RenderProgress;
    DzProgress *FrameProgress;

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
    bool bSaveAlphaChannel;
    int haltAtTime;
    int haltAtSamplesPerPixel;
    float haltAtThreshold;
    int debugLevel;
    float tonemapGamma;
    float tonemapFstop;
    float tonemapExposureTime;
    int tonemapISO;
    QString LuxToneMapper;
    QStringList slaveNodeList;
    int specularMode;
    int renderMode;
    QString customRenderString;

    QString cachePath;
    QString tempPath;
    QString tempFilenameBase;
    QString workingRenderFilename;

    QProcess *luxRenderProc;
    bool bRenderisFinished;
    int tempCounter;

    QStringList tokenList;
    DzTokenBuffer tokenBuffer;

};

extern struct G YaLuxGlobal;


#endif // YALUX_PLUGIN_H