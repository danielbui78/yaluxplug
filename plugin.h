//
//  plugin.h
//  yaluplug
//
//  Created by Daniel Bui on 4/23/15.
//
//

#ifndef YALUX_PLUGIN_H
#define YALUX_PLUGIN_H

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
    DzRenderSettings *settings;
    DzRenderOptions *options;
    DzOptionsFrame *optFrame;
    DzProgress *RenderProgress;
    DzNode *currentNode;
    bool inProgress;
    QString cachePath;
    QString pathTempName;
    QStringList tokenList;
    DzRenderHandler *handler;
    QThread threadList[8];
    int tempCounter;
};

extern struct G YaLuxGlobal;


#endif // YALUX_PLUGIN_H