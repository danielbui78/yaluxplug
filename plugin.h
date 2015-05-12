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

    DzRenderHandler *handler;
    DzRenderSettings *settings; // used when calling node->render
    DzRenderOptions options;

    QString LuxExecPath;
    QString CmdLineArgs;
    QString cachePath;
    QString tempPath;
    QString tempFilenameBase;
    QString workingRenderFilename;
    QStringList tokenList;
    QProcess *luxRenderProc;
    int tempCounter;
};

extern struct G YaLuxGlobal;


#endif // YALUX_PLUGIN_H