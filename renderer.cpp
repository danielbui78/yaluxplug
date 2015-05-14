//
//  renderer.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/23/15.
//
//

/*****************************
   Include files
*****************************/
#include <QtGui/QMessageBox>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtGui/QColor>
#include <QtCore/QTimer>
#include <QtCore/QProcess>

#include "dzapp.h"
#include "dzscene.h"
#include "dzrenderoptions.h"
#include "dztimerange.h"
#include "dzcamera.h"
#include "dzmatrix4.h"
#include "dzlight.h"
#include "dzdistantlight.h"
#include "dzrenderhandler.h"
#include "dzrendersettings.h"
#include "dzimagemgr.h"
#include "dzfileio.h"
#include "dztexture.h"
#include "dzproperty.h"
#include "dzobject.h"
#include "dzshape.h"
#include "dzgeometry.h"
#include "dzmaterial.h"
#include "dzdefaultmaterial.h"
#include "dztarray.h"
#include "dzstringproperty.h"
#include "dzcolorproperty.h"
#include "dzimageproperty.h"
#include "dzfloatproperty.h"
#include "dzintproperty.h"
#include "dznodeproperty.h"
#include "dznumericproperty.h"
#include "dzvertexmesh.h"
#include "dzfacetmesh.h"
#include "dzfacegroup.h"

#include "dzrenderdata.h"

#include "optionsframe.h"

#include "dazToPLY.h"
#include "renderer.h"
#include "plugin.h"



///////////////////////////////////////////////////////////////////////
// yaluxplug - YaLuxRender class
///////////////////////////////////////////////////////////////////////

/**
**/
YaLuxRender::YaLuxRender() 
{
    // DEBUG
    // DEFAULT LuxPath
    YaLuxGlobal.LuxExecPath = "/Applications/LuxRender1.3.1/LuxRender.app/Contents/MacOS/luxconsole";

//    dzApp->log("yaluxplug: initializing options");
    YaLuxGlobal.activeFrame = -1;
    YaLuxGlobal.frame_counter = 0;
    YaLuxGlobal.endFrame = -1;
    YaLuxGlobal.totalFrames = 0;
    YaLuxGlobal.tempCounter = 0;
    YaLuxGlobal.inProgress = false;
    YaLuxGlobal.currentNode = DI_NULL;
    YaLuxGlobal.cachePath = dzApp->getTempPath() + "/yaluxCache/";
    // create cache working directory
    DzFileIO::pathExists(YaLuxGlobal.cachePath,true);
    dzApp->log("yaluxplug: Initialized.");
    return;
}

///////////////////////////////////////////////////////////////////////
// public
///////////////////////////////////////////////////////////////////////


QString YaLuxRender::getLuxExecPath() const
{
    return YaLuxGlobal.LuxExecPath;
}

void YaLuxRender::setLuxExecPath(const QString &execPath)
{
    YaLuxGlobal.LuxExecPath = execPath;
}


bool YaLuxRender::render(DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions &opt)
{
    QSize renderImageSize;
    QString mesg;
    QString fullPathFileNameLXS;
    QString fullPathTempFileNameNoExt;
    QString tempPath;
    int steps = 10;
    bool bIsAnimation=false;
    DzTimeRange timeRenderingRange;

    dzApp->log("\nyaluxplug: render() called.");

    YaLuxGlobal.inProgress = true;

    YaLuxGlobal.optFrame->applyChanges();

    YaLuxGlobal.RenderProgress = new DzProgress("yaluxplug Render Started", steps, true, true);
//    YaLuxGlobal.RenderProgress->setShowTimeElapsed(DI_TRUE);
//    YaLuxGlobal.RenderProgress->setCloseOnFinish(DI_FALSE);
//    YaLuxGlobal.RenderProgress->setUseCloseCheckbox(DI_TRUE);

    fullPathTempFileNameNoExt = dzApp->getTempFilename();
    YaLuxGlobal.workingRenderFilename = fullPathTempFileNameNoExt + ".png";
    YaLuxGlobal.tempPath = DzFileIO::getFilePath(fullPathTempFileNameNoExt);
    YaLuxGlobal.tempFilenameBase = DzFileIO::getBaseFileName( fullPathTempFileNameNoExt );
    fullPathFileNameLXS = fullPathTempFileNameNoExt + ".lxs";

    // DEBUG
    mesg = "Writing to LXS file = " + fullPathFileNameLXS;
    dzApp->log( QString("yaluxplug: pathTempName=[%1], workingRenderFilename=[%2], fileNameLXS=[%3]").arg(YaLuxGlobal.tempPath).arg(YaLuxGlobal.workingRenderFilename).arg(fullPathFileNameLXS) );

    // Get Render Settings
    // time range
    YaLuxGlobal.options.copyFrom(&opt);
    YaLuxGlobal.frame_counter = 0;
    if ( !opt.isCurrentFrameRender() )
    {
        timeRenderingRange.setEnds(opt.getStartTime(),opt.getEndTime());
        dzScene->setTime(timeRenderingRange.getStart());
        YaLuxGlobal.activeFrame = dzScene->getFrame();
        DzTime timeDuration = timeRenderingRange.getDuration();
        int nFramesToRender = (timeDuration / dzScene->getTimeStep() )+1;
        // DEBUG
        dzApp->log( QString("number of frames to render is %1").arg(nFramesToRender));
        YaLuxGlobal.endFrame = YaLuxGlobal.activeFrame + nFramesToRender;
        YaLuxGlobal.totalFrames = nFramesToRender;
        if (nFramesToRender > 1)
            bIsAnimation = true;
        YaLuxGlobal.RenderProgress->step();
        mesg = QString("Preparing to render: %1 to %2 (%3 frames)\n").arg(timeRenderingRange.getStart()).arg(timeRenderingRange.getEnd()).arg(nFramesToRender);
        YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
    } else {
        YaLuxGlobal.activeFrame = dzScene->getFrame();
        YaLuxGlobal.endFrame = YaLuxGlobal.activeFrame;
        YaLuxGlobal.totalFrames = 1;
    }

    YaLuxGlobal.RenderProgress->step();

///////////////////////////////

    emit aboutToRender(this);

//    connect(this, SIGNAL(updateData( DzRenderData &)),
//            handler, SLOT(passData( DzRenderData &)) );
    connect(this, SIGNAL(beginningFrame(int)),
            handler, SLOT(beginFrame(int)) );
    connect(this, SIGNAL(frameFinished() ),
            handler, SLOT(finishFrame()) );
    connect(this, SIGNAL(beginningRender() ),
            handler, SLOT(beginRender()) );
    connect(this, SIGNAL(renderFinished()),
            handler, SLOT(finishRender()) );
    connect(handler, SIGNAL(killRender()),
            this, SLOT(killRender()) );

    YaLuxGlobal.handler = handler;

    //////////////////////////
    // Set up external process
    ///////////////////////////
    QProcess *process = new QProcess(this);
    YaLuxGlobal.luxRenderProc = process;
    QString logFile = QString("%1/yaluxplug.log").arg(YaLuxGlobal.tempPath);
    process->setStandardErrorFile(logFile, QIODevice::Append);

    connect(process, SIGNAL( finished(int, QProcess::ExitStatus) ),
            this, SLOT( handleRenderProcessComplete(int, QProcess::ExitStatus) ) );

    QString file;
    if (bIsAnimation)
        file = YaLuxGlobal.LuxExecPath;
    else
    {
        if (YaLuxGlobal.bShowLuxRenderWindow)
        {
#if defined( Q_OS_WIN )
            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender.exe";
#elif defined( Q_WS_MAC )
            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender";
#endif
        }
        else
        {
            file = YaLuxGlobal.LuxExecPath;
        }
    }
    QStringList userargs = QStringList::split(" ", YaLuxGlobal.CmdLineArgs);
    QStringList args = QStringList() << "-l" << userargs << fullPathFileNameLXS;
    //DEBUG
    dzApp->log( args.join(","));
//    args << "-uphenom-ubuntu";
//    process->start(file, args);
//    dzApp->log( QString("yaluxplug: SPAWNING: %1 %2").arg(file).arg(args.join(" ")) );

    // calling this will open up a Daz render window
    //    handler->beginRender();
//    emit beginningFrame(YaLuxGlobal.activeFrame);
//    emit beginningRender();

    // DEBUG

    // Start a render loop...
    // one time through loop per frame

//    QImage qimg;
//    DzRenderData *data;
    int TimeOut = 5000;
    QTimer tmr;
    connect(&tmr, SIGNAL(timeout()),
            this, SLOT(updateData()) );
    handler->beginRender();
    while (YaLuxGlobal.frame_counter < YaLuxGlobal.totalFrames)
    {

        //DEBUG
        mesg = QString("Rendering frame #%1 (%2/%3)").arg(YaLuxGlobal.activeFrame).arg(YaLuxGlobal.frame_counter+1).arg(YaLuxGlobal.totalFrames);
        dzApp->log( mesg );
        YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);

        dzScene->setFrame(YaLuxGlobal.activeFrame);
        LuxMakeSceneFile(fullPathFileNameLXS, this, camera, opt);

        // DEBUG
        // can't do spotrenders yet (code incomplete) so abort if this is one
        if (YaLuxGlobal.bIsSpotRender == true)
        {
            YaLuxGlobal.RenderProgress->finish();
            YaLuxGlobal.inProgress = false;
            return false;
        }
        process->start(file, args);
        process->waitForStarted();

        tmr.start(1000);
        handler->beginFrame(YaLuxGlobal.frame_counter);
        while (process->state() == QProcess::Running)
        {
            // DEBUG
            //process->waitForFinished();
            // double check process
            // DEBUG
            if ( (process->state() != QProcess::Running) || (YaLuxGlobal.RenderProgress->isCancelled() == true) )
            {
                dzApp->log("yaluxplug: Renderer - time to break;");
                YaLuxGlobal.luxRenderProc->terminate();
                break;
            } else
            QCoreApplication::processEvents(QEventLoop::AllEvents,TimeOut);
            int timeout2 = 100;
#ifdef Q_OS_WIN
            Sleep(uint(timeout2));
#else
            struct timespec ts = { timeout2 / 1000, (timeout2 % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
#endif

        }
        tmr.stop();
        disconnect(&tmr, SIGNAL(timeout()),
                this, SLOT(updateData()) );

        if (YaLuxGlobal.RenderProgress->isCancelled() == true)
            break;
        YaLuxGlobal.frame_counter++;
        YaLuxGlobal.activeFrame++;

    }

    disconnect(process, SIGNAL( finished(int, QProcess::ExitStatus) ),
            this, SLOT( handleRenderProcessComplete(int, QProcess::ExitStatus) ) );
    YaLuxGlobal.luxRenderProc->deleteLater();

    YaLuxGlobal.inProgress = false;
//    handler->finishRender();
    YaLuxGlobal.RenderProgress->finish();


    return true;
}

void YaLuxRender::updateData()
{
    QImage *qimg = new QImage();
    DzRenderData *data;

    if (qimg->load(YaLuxGlobal.workingRenderFilename) == true)
    {
        data = new DzRenderData(YaLuxGlobal.cropWindow.top(), YaLuxGlobal.cropWindow.left(), qimg->convertToFormat(QImage::Format_ARGB32));
        YaLuxGlobal.handler->passData( (*data) );
    }
    delete qimg;
    // delete data;
}


void YaLuxRender::handleRenderProcessComplete( int exitCode, QProcess::ExitStatus status )
{
    QImage *qimg = new QImage();
    DzRenderData *data;

    if (YaLuxGlobal.inProgress == false)
        return;

    if (qimg->load(YaLuxGlobal.workingRenderFilename) == true)
    {
        data = new DzRenderData(YaLuxGlobal.cropWindow.top(), YaLuxGlobal.cropWindow.left(), qimg->convertToFormat(QImage::Format_ARGB32));
        YaLuxGlobal.handler->passData( (*data) );
        QString tempRenderName = dzApp->getTempRenderFilename() + ".png";
        qimg->save(tempRenderName);
        delete qimg;
    }

    emit frameFinished();

/*
    if (YaLuxGlobal.activeFrame <= YaLuxGlobal.endFrame)
    {
        emit frameFinished();
    }
    else
    {
        emit frameFinished();
        emit renderFinished(this);
//        YaLuxGlobal.inProgress = false;
        //    YaLuxGlobal.luxRenderProc->deleteLater();
        dzApp->log( QString("yaluxplug: RENDER PROCESS exited with %1 ").arg(exitCode) );
    }
*/
}

bool YaLuxRender::customRender(DzRenderHandler *handler, DzCamera *camera, DzLightList &lights, DzNodeList &nodes, const DzRenderOptions &opt)
{
    dzApp->log("yaluxplug: unimplemented call customRender()");
    
    return true;
}

DzOptionsFrame* YaLuxRender::getOptionsFrame() const
{
    YaLuxGlobal.optFrame = new YaLuxOptionsFrame();
    
    dzApp->log("yaluxplug: creating options frame");
    
    return YaLuxGlobal.optFrame;
}

DtFilterFunc YaLuxRender::getFilterFunction(DzRenderOptions::PixelFilter filterType) const
{
    DtFilterFunc fpResult = DI_NULL;
    dzApp->log("yaluxplug: unimplemented call getFilterFunction()");
        
    return fpResult;
}


///////////////////////////////////////////////////////////////////////
// public slots
///////////////////////////////////////////////////////////////////////
//
// MANIPULATORS (public slot)
//
bool YaLuxRender::render(DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions *opt)
{
    // This does not get caleld by clicking render button.... callback for message-passing backend
    return render(handler, camera, (DzRenderOptions &)opt);
}

bool YaLuxRender::customRender( DzRenderHandler *handler, DzCamera *camera, QObjectList lights, QObjectList nodes, const DzRenderOptions *opt )
{
    // This does not get caleld by clicking render button.... callback for message-passing backend
    return customRender(handler, camera, (DzLightList& ) lights, (DzNodeList& ) nodes, (DzRenderOptions &)opt);
}

void YaLuxRender::prepareImage(const DzTexture *img, const QString &filename)
{
    QSize imgSize;
    QImage qimg;
    int WidthResize = 2000;
    
    imgSize = img->getOriginalImageSize();
    if ( imgSize.width() <= WidthResize )
    {
        // this size is good, just keep it
        emit imagePrepared(img, filename);
        return;
    }

    // otherwise, spawn a thread to do it after loading completed

    dzApp->log("yaluxplug: prepareImage( " + filename + " ) - Starting New Thread" );    
//    QThread *thread = new QThread;
    WorkerPrepareImage *worker = new WorkerPrepareImage(img, filename);
    worker->myThread = new QThread;
    worker->moveToThread(worker->myThread);

    connect(worker->myThread, SIGNAL(started()), 
            worker, SLOT(doPrepareImage()) );

    connect(worker, SIGNAL(prepareImageComplete(WorkerPrepareImage *, const DzTexture *, const QString &) ), 
            this, SLOT(handlePrepareImageComplete(WorkerPrepareImage *, const DzTexture *, const QString &) ) );

    connect(worker, SIGNAL(finished()), worker->myThread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker->myThread, SIGNAL(finished()), worker->myThread, SLOT(deleteLater()));

    worker->myThread->start();
    
    return;
};

void YaLuxRender::handlePrepareImageComplete( WorkerPrepareImage *worker, const DzTexture *img, const QString &filename)
{
    emit imagePrepared(img, filename);
};


QString YaLuxRender::compileShader(const QString &shaderPath)
{
    QString sResult = shaderPath;
    dzApp->log("yaluxplug: compiling shader (" + shaderPath + ").");

    return sResult;
};

QString YaLuxRender::compileShader(const QString &shaderPath, QString &output)
{
    QString sResult = shaderPath;
    dzApp->log("yaluxplug: compiling shader (" + shaderPath + ").");
    output = "yaluxplug: compiling shader for " + shaderPath;
    
    return sResult;
}

DzShaderDescription* YaLuxRender::getShaderInfo(const QString &shaderPath)
{
    DzShaderDescription* oResult = NULL;
    dzApp->log("yaluxplug: getShaderInfo called (" + shaderPath + ").");   
    return oResult;
}

void YaLuxRender::killRender()
{
    // stop rendering now
    dzApp->log("yaluxplug: killRender was called.");
    // Kill Render process
    YaLuxGlobal.luxRenderProc->kill();

    return;
};

bool YaLuxRender::bake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt )
{
    dzApp->log("yaluxplug: unimplemented call bake().");
    return false;
}

bool YaLuxRender::autoBake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt )
{
    dzApp->log("yaluxplug: unimplemented call autoBake();");
    return false;
}

void YaLuxRender::stopBaking()
{
    dzApp->log("yaluxplug: unimplemented call stopBaking();");
    return;
}

void YaLuxRender::saveBakeImage( const DzBakerOptions &opt, bool wait )
{
    dzApp->log("yaluxplug: unimplemented call saveBakeImage()");
    return;
}

bool YaLuxRender::textureConvert( DzRenderHandler *handler, DzCamera *camera, const DzTextureConvertorOptions &opt )
{
    // convert a texture for rendering?
    dzApp->log("yaluxplug: unimplemented call textureConvert()");

    return false;
}    


//
// ACCESSORS (public slot)
//

QString YaLuxRender::getShaderCompilerPath()
{
    dzApp->log("yaluxplug: unimplemented call getShaderCompilerPath()");
    QString sResult = "";
    
    return sResult;
}
                                                                                                                                                  

QString YaLuxRender::getTextureUtilityPath()
{
    dzApp->log("yaluxplug: unimplemented call getTextureUtilityPath()");
    QString sResult = "";
    
    return sResult;
    
};

QStringList YaLuxRender::getShaderSearchPaths() const
{
    dzApp->log("yaluxplug: unimplemented call getShaderSearchPaths()");
    QStringList oResult = QStringList() << "";
    return oResult;
};

QString YaLuxRender::processShaderName( const QString &shaderName ) const
{
    dzApp->log("yaluxplug: unimplemented call processShaderName()");
    QString sResult = NULL;
    
    return sResult;
};


QString YaLuxRender::getShaderPath( const QString &shaderName, bool withExtension ) const
{
    dzApp->log("yaluxplug: unimplemented call getShaderPath()");
    QString sResult = NULL;
    
    return sResult;
    
}

QString YaLuxRender::getShaderFileName( const QString &shaderName ) const
{
    dzApp->log("yaluxplug: unimplemented call getShaderFileName()");
    QString sResult = NULL;
    
    return sResult;
};

QString YaLuxRender::getShaderExtension() const
{
    dzApp->log("yaluxplug: unimplemented call getShaderExtension()");
    QString sResult = NULL;
    
    return sResult;
    
}

bool YaLuxRender::isRendering() const
{
    dzApp->log( QString("yaluxplug: call to isRendering() - returning %1").arg(YaLuxGlobal.inProgress) );
    return YaLuxGlobal.inProgress;
};

QString YaLuxRender::getName() const
{
    dzApp->log("yaluxplug: unimplemented call getName()");
    QString sResult = "yaluxplug Render";
    
    return sResult;
    
}

DzNode* YaLuxRender::getCurrentNode() const
{    
    dzApp->log("yaluxplug: unimplemented call getCurrentNode()");
    return YaLuxGlobal.currentNode;
}



///////////////////////////////////////////////////////////////////////
// protected (Renderman)
///////////////////////////////////////////////////////////////////////
DtVoid YaLuxRender::DiBegin(DtToken name)
{
    // Create new RenderMan rendering context
    dzApp->log("yaluxplug: DiBegin called ( " + QString(name) + " ).");
    
};

DtVoid YaLuxRender::DiEnd()
{
    // Terminate the active rendering context
    dzApp->log("yaluxplug: DiEnd called.");
    
};

DtVoid YaLuxRender::DiFrameBegin(DtInt number)
{
    // Mark beginning of single frame of animated sequence
    // All information that is frame specific should be removed after DiFrameEnd()
    // Not needed for a single image
    dzApp->log("yaluxplug: DiFrameBegin called ( " + QString(number) + " .");

};

DtVoid YaLuxRender::DiFrameEnd()
{
    // Mark end of single frame of animated sequence
    dzApp->log("yaluxplug: DiFrameEnd called.");

};


//////////////////
// RenderMan API
//////////////////
DtToken YaLuxRender::DiDeclare( const char *name, const char *declaration ) 
{
    // RiDeclare() - declare the name and type of a variable.
    // syntax: [class] [type] ['['n']']
    //      [class] = {constant, uniform, varying, vertex}
    //      [type]  = {float, integer, string, color, point, vector, normal, matrix, hpoint}
    //      ['['n']'] = array of n type
    
    QString newToken = QString(name);
    YaLuxGlobal.tokenList.append(newToken);
    
    dzApp->log( QString("yaluxplug: DiDeclare( %1, \"%2\" ) == newToken: [%3]").arg(name).arg(declaration).arg((DtToken)newToken.toAscii()) );
    
    return newToken.toAscii();
};


DtVoid YaLuxRender::DiAttributeV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )
{
    int count = 0;
    while (count < n) {
        QString sToken = QString( (char*)tokens[count] );
        DtPointer ptr = params[count];
        if  ( sToken.contains("name") || sToken.contains("string") ) 
        {
            char *sPtr = (char*)ptr;
            //char *sPtr1 = (char*)params[0];
            //char *sPtr2 = (char*)&ptr;
            //char *sPtr3 = (char*)params;
            dzApp->log( QString("yaluxplug: DiAttributeV( %1, n=%2/%3, [%4], \"%5\" )").arg(name).arg(count+1).arg(n).arg(sToken).arg(sPtr) );
        } else if ( sToken.contains("integer") ) {
            int *iPtr = (int*)ptr;
            dzApp->log( QString("yaluxplug: DiAttributeV( %1, n=%2/%3, [%4], [%5] )").arg(name).arg(count+1).arg(n).arg(sToken).arg(*iPtr));        
        } else if ( sToken.contains("float") ) {
            float *fPtr = (float*)ptr;
            dzApp->log( QString("yaluxplug: DiAttributeV( %1, n=%2/%3, [%4], [%5] )").arg(name).arg(count+1).arg(n).arg(sToken).arg(*fPtr));            
        } else {
            dzApp->log ( QString("yaluxplug: DiAttributeV( %1, n=%2/%3, [%4], params[] )").arg(name).arg(count+1).arg(n).arg(sToken) );
        }
        count++;
    }

};


DtVoid YaLuxRender::DiColor( DtColor Cs )
{ 
    DtFloat *fltArray = Cs;
    dzApp->log( QString("yaluxplug: DiColor( [%1 %2 %3] )").arg( fltArray[0] ).arg( fltArray[1] ).arg( fltArray[2] ) ); 

};


DtVoid YaLuxRender::DiOpacity( DtColor Cs )
{ 
    DtFloat *fltArray = Cs;
    dzApp->log( QString("yaluxplug: DiOpacity( [%1 %2 %3] )").arg( fltArray[0] ).arg( fltArray[1] ).arg( fltArray[2] ) ); 
   
};


DtVoid YaLuxRender::DiTransform( DtMatrix transform )
{ 
    DtFloat *mat4 = (DtFloat*)&transform;
    dzApp->log( QString("yaluxplug: DiTransform( [%1 %2 %3 %4] )").arg(mat4[0]).arg(mat4[1]).arg(mat4[2]).arg(mat4[3]) ); 

};


DtVoid YaLuxRender::DiPointsPolygonsV( DtInt npolys, DtInt *nverts, DtInt *verts,
                                          DtInt n, const DtToken tokens[], DtPointer params[] )
{ 
    QString mesg = QString("yaluxplug: DiPointsPolygonsV( npolys=%1:\n\tnverts=[").arg(npolys); 
    int count = 0;
    int numVerts = 0;
    while (count < npolys) {
        mesg += QString("%1 ").arg(nverts[count]); 
        numVerts += nverts[count];
        count++;
    }
    mesg += "] verts=[";
    count = 0;
    while (count < numVerts) {
        mesg += QString("%1 ").arg(verts[count]);
        count++;
    }
    mesg += "]\n";
    count = 0;
    while (count < n) {
        QString sToken = QString( (char*)tokens[count] );
        DtPointer ptr = params[count];
        char* paramstring = (char*)ptr;
        if  ( sToken.contains("string") || sToken.contains("P") )
        {
            mesg += QString("\tn=%1, [%2] = \"%3\" )").arg(count).arg(sToken).arg(paramstring) ;
        } else {
            mesg += QString("\tn=%1, [%2], params[] )").arg(count).arg(sToken);
        }        
        if (count++ < n)
            mesg += "\n";
    }
    dzApp->log(mesg);
};
                                          
                                          
DtVoid YaLuxRender::DiHierarchicalSubdivisionMeshV(	DtToken scheme, 
                                                       DtInt nfaces, DtInt nvertices[], DtInt vertices[], 
                                                       DtInt ntags, const DtToken tags[], DtInt nargs[], 
                                                       DtInt intargs[], DtFloat floatargs[], const DtToken stringargs[], 
                                                       DtInt n, const DtToken tokens[], DtPointer params[])
{ 
    dzApp->log( QString("yaluxplug: DiHierarchicalSubdivisionMeshV( scheme=%1, nfaces=%2, nvertices[], vertices[], ntags=%3, tags[], nargs[], intargs[], floatargs[], stringargs[],").arg(scheme).arg(nfaces).arg(ntags) ); 
    int count = 0;
    while (count < n) {
        QString sToken = QString( (char*)tokens[count] );
        char* paramstring = (char*)(params[count]);
        if  ( sToken.contains("string") || sToken.contains("name") ) 
        {
            dzApp->log( QString("\tn=%1, [%2] = \"%3\" )").arg(count).arg(sToken).arg(paramstring) );        
        } else {
            dzApp->log( QString("\tn=%1, [%2], params[] )").arg(count).arg(sToken) );
        }        
        count++;
    }

};


DtVoid YaLuxRender::DiResourceV(DtToken handle, DtToken type,
                                    DtInt n, const DtToken tokens[], DtPointer params[])
{ 
    dzApp->log( QString("yaluxplug: DiResourceV( handle=\"%1\",\n\ttype=\"%2\",").arg(handle).arg(type) ); 
    int count = 0;
    while (count < n) {
        QString sToken = QString( (char*)tokens[count] );
        if  ( sToken.contains("string") || sToken.contains("name") ) 
        {
            dzApp->log( QString("\tn=%1, [%2] = \"%3\" )").arg(n).arg( sToken ).arg(QString((char*)params[count])) );        
        } else {
            dzApp->log( QString("\tn=%1, [%2], params[] )").arg(n).arg( sToken ) );
        }        
        count++;
    }

};







// End of File