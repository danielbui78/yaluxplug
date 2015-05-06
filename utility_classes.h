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

const QStringList classNamesProperties = QStringList() << "DzStringProperty" << "DzColorProperty" << "DzFloatProperty" << "DzIntProperty" <<  "DzNodeProperty" << "DzImageProperty" ;

QString LuxProcessObject(DzObject *daz_obj);
QString LuxProcessMaterial(DzMaterial *material, QString &mesg, QString matLabel);
QString LuxProcessProperties(DzElement *el, QString &mesg);
QString LuxProcessLight(DzLight *currentLight, QString &mesg);

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
