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

#include <QtGui/QFrame>
#include <QtGuI/QLayout>
#include <QtGui/QBoxLayout>
#include <QtGUI/QTextEdit>
#include <QtCore/QByteArray>
#include <QtGui/QWidget>
#include <QtGui/QButtonGroup>

#include <qfile.h>
#include <qmessagebox.h>

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

#include "dzviewrenderhandler.h"
#include "dzrenderdata.h"
#include "dzmainwindow.h"

#include "optionsframe.h"

#include "dazToPLY.h"
#include "renderer.h"
#include "plugin.h"

#include "luxcore/DazToLuxCoreFile.h"
#include "luxrender/DazToLuxRenderFile.h"

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


    YaLuxGlobal.optFrame = NULL;

    ///////////
    // Create Log window
    ////////////
    if (YaLuxGlobal.logWindow == NULL)
    {
        YaLuxGlobal.logWindow = new QFrame();
        YaLuxGlobal.logWindow->setParent((QWidget*)dzApp->getInterface());
        YaLuxGlobal.logWindow->setWindowTitle("LogWindow");
        YaLuxGlobal.logWindow->setMinimumSize(800, 100);
        QVBoxLayout* layout = new QVBoxLayout(YaLuxGlobal.logWindow);
        YaLuxGlobal.logText = new QTextEdit(YaLuxGlobal.logWindow);
        layout->addWidget(YaLuxGlobal.logText);
        QHBoxLayout* buttonBar = new QHBoxLayout();
        //QPushButton *showLXS = new QPushButton("&Show Scenefile (.LXS)", YaLuxGlobal.logWindow);
        //buttonBar->addWidget(showLXS);
        QPushButton* showSCN = new QPushButton("&Show Scenefile (.SCN)", YaLuxGlobal.logWindow);
        buttonBar->addWidget(showSCN);
        QPushButton* previewCurrentFrame = new QPushButton("Pre&view current frame", YaLuxGlobal.logWindow);
        buttonBar->addWidget(previewCurrentFrame);
        QPushButton* stopRenderButton = new QPushButton("&Stop rendering", YaLuxGlobal.logWindow);
        buttonBar->addWidget(stopRenderButton);
        QPushButton* resumeRenderButton = new QPushButton("&Resume rendering", YaLuxGlobal.logWindow);
        buttonBar->addWidget(resumeRenderButton);
        QPushButton* nextFrameButton = new QPushButton("&Next frame", YaLuxGlobal.logWindow);
        buttonBar->addWidget(nextFrameButton);

        layout->addLayout(buttonBar);

        connect(resumeRenderButton, SIGNAL(clicked()),
            this, SLOT(handleResumeRender()));
        connect(stopRenderButton, SIGNAL(clicked()),
            this, SLOT(handleStopRender()));
        connect(nextFrameButton, SIGNAL(clicked()),
            this, SLOT(handleNextFrame()));
        connect(previewCurrentFrame, SIGNAL(clicked()),
            this, SLOT(handlePreviewCurrentFrame()));
        connect(showSCN, SIGNAL(clicked()),
            this, SLOT(handleShowSCN()));

        connect(
            this, SIGNAL(updateLogWindow(QString, QColor, bool)),
            this, SLOT(handleLogWindow(QString, QColor, bool))
            );

        // create thread to process corerenderlog....etc
        Worker_UpdateInfoWindow* worker = new Worker_UpdateInfoWindow();
        worker->myThread = new QThread;
        worker->moveToThread(worker->myThread);
        connect(worker->myThread, SIGNAL(started()),
            worker, SLOT(doUpdate()));
        worker->myThread->start();

        // same this->signal, modify this/that
        connect(
            worker, SIGNAL(updateLogWindow(QString, QColor, bool)),
            this, SLOT(handleLogWindow(QString, QColor, bool))
        );
        //connect(
        //    this, SIGNAL(updateLogWindow(QString, QColor, bool)),
        //    worker, SLOT(handleLogWindow(QString, QColor, bool))
        //);
        //connect(
        //    worker, SIGNAL(updateLogWindow(QString, QColor, bool)),
        //    worker, SLOT(worker->handleLogWindow(QString, QColor, bool))
        //);

        //// modify worker->signal, this/that
        //connect(
        //    this, SIGNAL(worker->updateLogWindow(QString, QColor, bool)),
        //    this, SLOT(handleLogWindow(QString, QColor, bool))
        //);
        //connect(
        //    worker, SIGNAL(worker->updateLogWindow(QString, QColor, bool)),
        //    this, SLOT(handleLogWindow(QString, QColor, bool))
        //);
        //connect(
        //    this, SIGNAL(worker->updateLogWindow(QString, QColor, bool)),
        //    worker, SLOT(handleLogWindow(QString, QColor, bool))
        //);
        //connect(
        //    worker, SIGNAL(worker->updateLogWindow(QString, QColor, bool)),
        //    worker, SLOT(handleLogWindow(QString, QColor, bool))
        //);

        connect(
            worker, SIGNAL(updateData()),
            this, SLOT(updateData())
        );

    }
    emit updateLogWindow(QString("YaluxRender initialized..."));

    return;
}

///////////////////////////////////////////////////////////////////////
// public
///////////////////////////////////////////////////////////////////////

void YaLuxRender::handleResumeRender()
{
    emit updateLogWindow(QString("Resume Clicked"), QColor(255, 0, 0), true);

    // 1. check if render was cancelled
//    if (YaLuxGlobal.bIsCancelled == false) return;
    //if (YaLuxGlobal.inProgress == true) return;
    if (YaLuxGlobal.luxRenderProc == NULL) return;
    if (YaLuxGlobal.luxRenderProc->state() == QProcess::Running) return;
    // 2. look for resume files

    // 3. set up luxrender process
    QStringList args;
    args << YaLuxGlobal.tempPath + "/" + YaLuxGlobal.tempFilenameBase + ".cfg";
    QString file = YaLuxGlobal.LuxExecPath;

    //////////////////////////////////////////////////
    // FORCE single frame render
    // FORCE other issues that can't be done without render() loop
    /////////////////////////////////////////////////
    YaLuxGlobal.activeFrame = dzScene->getFrame();
    YaLuxGlobal.endFrame = YaLuxGlobal.activeFrame;
    YaLuxGlobal.totalFrames = 1;

    // 4. restart it!    
    YaLuxGlobal.inProgress = true;
    YaLuxGlobal.RenderProgress->resume();
//    YaLuxGlobal.FrameProgress->resume();;
    YaLuxGlobal.handler->beginRender();

    YaLuxGlobal.luxRenderProc->start(file, args);

    bool result = YaLuxGlobal.luxRenderProc->waitForStarted();
    if (result == false)
    {
        // abort
        QStringList abortMessage;
        abortMessage << "yaluxplug: could not start process: " << file << " " << args << ". aborting.\n";
        dzApp->log(abortMessage.join(""));
        YaLuxGlobal.RenderProgress->setCurrentInfo(abortMessage.join(" "));
//        logFile.write(abortMessage.join("").toAscii());
        YaLuxGlobal.handler->finishRender();
        YaLuxGlobal.inProgress = false;
        YaLuxGlobal.RenderProgress->finish();
//        YaLuxGlobal.FrameProgress->finish();
//        logFile.close();
        return;
    }


}

void YaLuxRender::handleNextFrame()
{
    if (YaLuxGlobal.luxRenderProc->state() == QProcess::Running)
        killRender();
}

void YaLuxRender::handleStopRender()
{
    // kill current render and cancel all rendering
    if (YaLuxGlobal.luxRenderProc->state() == QProcess::Running)
    {
        killRender();
    }
    YaLuxGlobal.RenderProgress->cancel(); // this does not work
    YaLuxGlobal.bIsCancelled = true;
}

void YaLuxRender::handlePreviewCurrentFrame()
{
    QString file = YaLuxGlobal.workingRenderFilename;
    YaLuxGlobal.logText->setTextColor( QColor(255,255,255));
    YaLuxGlobal.logText->append( QString("Opening file: [%1]").arg( QUrl::fromLocalFile(file).toString() ) );
    QDesktopServices::openUrl( QUrl::fromLocalFile(file) );
}

void YaLuxRender::handleShowSCN()
{
    QString file =YaLuxGlobal.tempPath + "/" + YaLuxGlobal.tempFilenameBase + ".scn" ;
    YaLuxGlobal.logText->setTextColor( QColor(255,255,255));
    YaLuxGlobal.logText->append( QString("Opening file: [%1]").arg( QUrl::fromLocalFile(file).toString() ) );
    QDesktopServices::openUrl( QUrl::fromLocalFile(file) );
}

QString YaLuxRender::getLuxExecPath() const
{
    return YaLuxGlobal.LuxExecPath;
}

void YaLuxRender::setLuxExecPath(const QString &execPath)
{
    YaLuxGlobal.LuxExecPath = execPath;
}


bool YaLuxRender::render(DzRenderHandler *old_handler, DzCamera *camera, const DzRenderOptions &opt)
{
    /////////////////////////////////////
    // Apply any changes from options frame
    /////////////////////////////////////
    YaLuxGlobal.optFrame->applyChanges();

    /////////////////////////////////////
    // Add Tonemapper Options if not exists
    /////////////////////////////////////
    DzNode* tonemapper = dzScene->findNode("Tonemapper Options");
    if (tonemapper)
    {

    }

    /////////////////////////////////////
    // ?? Optional ??? Add IBL Environemnet if no lights
    /////////////////////////////////////
    DzNode* environment = dzScene->findNode("Environment Options");
    if (environment)
    {

    }


    QSize renderImageSize;
    QString mesg;
    QString fullPathFileNameLXS;
    QString fullPathTempFileNameNoExt;
    QString tempPath;
    int steps = 100;
    bool bIsAnimation=false;
    DzTimeRange timeRenderingRange;

    if (YaLuxGlobal.debugLevel >=2) // debugging data
    dzApp->log("\nyaluxplug: render() called.");

    /////////////////////////////////////
    // Double check external renderer exists
    ////////////////////////////////////

    QString file;
    if (bIsAnimation)
    {
        file = YaLuxGlobal.LuxExecPath;

    }
    else
    {
        if (YaLuxGlobal.bShowLuxRenderWindow)
        {
#if defined( Q_OS_WIN )
            //            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender.exe";
            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxcoreui.exe";
#elif defined( Q_WS_MAC )
            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender";
#endif
        }
        else
        {
            file = YaLuxGlobal.LuxExecPath;
        }
    }
    if (QFile(file).exists() != true)
    {
        // abort
        QMessageBox::critical(0, tr("Error"), tr("ERROR: Can not find Luxrender executable: ") + file, QMessageBox::Ok);
        return false;
    }



    //////////////////////////////////
    // Render Handler and Progress
    //////////////////////////////////
    old_handler->dumpObjectInfo();
    old_handler->dumpObjectTree();
    DzViewRenderHandler *handler = new DzViewRenderHandler(old_handler->getSize(), old_handler->getStartingTime(), QString(""), true);
//    DzImageRenderHandler *handler = new DzImageRenderHandler(old_handler->getSize(), old_handler->getStartingTime(), old_handler->getNumFrames(), QString(""), true);

    YaLuxGlobal.inProgress = true;
    YaLuxGlobal.bIsCancelled = false;

    YaLuxGlobal.optFrame->applyChanges();

    YaLuxGlobal.RenderProgress = new DzProgress("yaluxplug Render Started", steps, true, true);
    YaLuxGlobal.RenderProgress->setUseCloseCheckbox(true);
    YaLuxGlobal.RenderProgress->setCloseOnFinish(true);


    fullPathTempFileNameNoExt = dzApp->getTempFilename();
    YaLuxGlobal.workingRenderFilename = fullPathTempFileNameNoExt + ".png";
    YaLuxGlobal.tempPath = DzFileIO::getFilePath(fullPathTempFileNameNoExt);
    YaLuxGlobal.tempFilenameBase = DzFileIO::getBaseFileName( fullPathTempFileNameNoExt );
    fullPathFileNameLXS = fullPathTempFileNameNoExt + ".lxs";

    // DEBUG
    mesg = "Writing to LXS file = " + fullPathFileNameLXS;
    if (YaLuxGlobal.debugLevel >= 2) // debugging data
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
        if (YaLuxGlobal.debugLevel >=1) // user data
            dzApp->log( QString("number of frames to render is %1").arg(nFramesToRender));
        YaLuxGlobal.endFrame = YaLuxGlobal.activeFrame + nFramesToRender;
        YaLuxGlobal.totalFrames = nFramesToRender;
        if (nFramesToRender > 1)
            bIsAnimation = true;
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
//    QProcess *process = new QProcess(this);
//    YaLuxGlobal.luxRenderProc = process;
    YaLuxGlobal.luxRenderProc = new QProcess(this);
    YaLuxGlobal.luxRenderProc->setWorkingDirectory(YaLuxGlobal.tempPath);
    QString logFileName = QString("%1/yaluxplug.log").arg(YaLuxGlobal.tempPath);
//    process->setStandardErrorFile(logFile, QIODevice::Append);
    YaLuxGlobal.luxRenderProc->setReadChannel(QProcess::StandardError);

    connect(YaLuxGlobal.luxRenderProc, SIGNAL( finished(int, QProcess::ExitStatus) ),
            this, SLOT( handleRenderProcessComplete(int, QProcess::ExitStatus) ) );
    connect(YaLuxGlobal.luxRenderProc, SIGNAL( stateChanged( QProcess::ProcessState) ),
            this, SLOT( handleRenderProcessStateChange( QProcess::ProcessState)) );

//    QString file;
//    if (bIsAnimation)
//    {
//        file = YaLuxGlobal.LuxExecPath;
//
//    }
//    else
//    {
//        if (YaLuxGlobal.bShowLuxRenderWindow)
//        {
//#if defined( Q_OS_WIN )
////            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender.exe";
//            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxcoreui.exe";
//#elif defined( Q_WS_MAC )
//            file = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath) + "/luxrender";
//#endif
//        }
//        else
//        {
//            file = YaLuxGlobal.LuxExecPath;
//        }
//    }
    QStringList userargs = YaLuxGlobal.CmdLineArgs.split(" ", QString::SkipEmptyParts);
    if (YaLuxGlobal.bNetworkRenderOn)
    {
        for (int i=0; i<YaLuxGlobal.slaveNodeList.count(); i++)
        {
            userargs << QString("-u%1").arg(YaLuxGlobal.slaveNodeList[i]);
        }
    }
    //QStringList args = QStringList() << "-l" << userargs << fullPathFileNameLXS;
    QStringList args = QStringList() << userargs << fullPathTempFileNameNoExt + ".cfg";
    //DEBUG
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: DEBUG: render process argument list = [%1]").arg(args.join(",")) );
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
    int TimeOut = 500;
//    QTimer tmr;
//    connect(&tmr, SIGNAL(timeout()),
//            this, SLOT(updateData()) );
    handler->beginRender();
    QFile logFile(logFileName);
    logFile.open(QIODevice::WriteOnly);
    /////////////////////////////////////////
    //
    // Begin Animation Rendering loop.
    //    Single frame renders only go through once.
    //    A scene file for the current frame time is first composed and then a QProcess
    //    is prepared for execution by the frame rendering loop.
    //
    /////////////////////////////////////////
    while (YaLuxGlobal.frame_counter < YaLuxGlobal.totalFrames)
    {

        //DEBUG
        mesg = "Preparing new frame:";
        if (YaLuxGlobal.debugLevel >=1) // user info
            dzApp->log( mesg );
        YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
        emit updateLogWindow("yaluxplug: " + mesg, QColor(255, 255, 255), true);

        ////////////////////////
        // manage PLYgarbageCollectionList:
        //   If this is a single frame render, then keep the PLYs in the temp
        //      directory so that the LXS file can be run manually if desired.
        //   If this is a multiframe render, then delete the PLYs in the temp
        //      directory so that we don't run out of harddisk space if rendering
        //      infinite frames or something.
        ////////////////////////
        while ( YaLuxGlobal.PLYgarbageCollectionList.count() > 0)
        {
            QTemporaryFile *file = YaLuxGlobal.PLYgarbageCollectionList.last();
            file->setAutoRemove(bIsAnimation);
            delete file;
            YaLuxGlobal.PLYgarbageCollectionList.removeLast();
        }

        // DEBUG
        // can't do spotrenders yet (code incomplete) so abort if this is one
        if (YaLuxGlobal.bIsSpotRender == true)
        {
            YaLuxGlobal.RenderProgress->finish();
//            YaLuxGlobal.FrameProgress->finish();
            YaLuxGlobal.inProgress = false;
            return false;
        }

        ////////////////////////////
        //
        //  Generate Lux Scene File
        //
        /////////////////////////////
        dzScene->setFrame(YaLuxGlobal.activeFrame);
        //LuxMakeLXSFile(fullPathFileNameLXS, this, camera, opt);
//        LuxMakeCFGFile(fullPathTempFileNameNoExt + ".cfg", this, camera, opt);
//        LuxMakeSCNFile(fullPathTempFileNameNoExt + ".scn", this, camera, opt);
        DazToLuxCoreFile(this, camera, opt, fullPathTempFileNameNoExt).WriteRenderFiles();

        // Set up progress bar for the current frame
        YaLuxGlobal.FrameProgress = new DzProgress("Current Frame Progress", 100, true, true);
        YaLuxGlobal.FrameProgress->setUseCloseCheckbox(true);
        YaLuxGlobal.RenderProgress->setCloseOnFinish(true);

        //////////////////////////////
        //
        //  Start the luxrender/luxconsole process
        //
        //////////////////////////////
        YaLuxGlobal.luxRenderProc->start(file, args);
        bool result = YaLuxGlobal.luxRenderProc->waitForStarted();
        if (result == false)
        {
            // abort
            QStringList abortMessage;
            abortMessage << "yaluxplug: could not start process: " << file << " " << args << ". aborting.\n";
            dzApp->log(abortMessage.join(""));
            logFile.write(abortMessage.join("").toAscii());
            handler->finishRender();
            YaLuxGlobal.inProgress = false;
            YaLuxGlobal.RenderProgress->finish();
            YaLuxGlobal.FrameProgress->finish();
            logFile.close();
            return false;
        }

        // Update the progress window
        mesg = QString("Rendering frame #%1 (%2/%3)...").arg(YaLuxGlobal.activeFrame).arg(YaLuxGlobal.frame_counter+1).arg(YaLuxGlobal.totalFrames);
        YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
        emit updateLogWindow("yaluxplug: " + mesg, QColor(255, 255, 255), true);
        float progressFraction = ((float)YaLuxGlobal.frame_counter+1)/((float)YaLuxGlobal.totalFrames);
        int updateProgress = 0 + (progressFraction)*90;
        YaLuxGlobal.RenderProgress->update( updateProgress );
        YaLuxGlobal.FrameProgress->step();

//        tmr.start(1000);
        // Notify the render handler of the newly started frame -- it will respond by
        // opening a render preview window if this is a single frame render job.

        if ( YaLuxGlobal.bShowLuxRenderWindow == false || YaLuxGlobal.totalFrames > 1 ||
            YaLuxGlobal.debugLevel >= 3 )
        {
            if (YaLuxGlobal.bShowLuxRenderWindow == false) handler->beginFrame(YaLuxGlobal.frame_counter);
            YaLuxGlobal.logWindow->show();
            YaLuxGlobal.logWindow->activateWindow();
        }


        connect(dzApp->getInterface(), SIGNAL(aboutToClose()),
                YaLuxGlobal.logWindow, SLOT(close()) );

        ////////
        // Only do below if rendering animation, otherwise just return
        ///////
        //if (bIsAnimation == false)
        //{
        //    logFile.close();
        //    return true;
        //}

        // reset bFrameisFinished before we start the frame render loop
        YaLuxGlobal.bFrameisFinished = false;
        //////////////////////////////////////////////
        //
        // Begin frame rendering loop
        //    This loop contains starting the render process and monitoring its output.
        //    While the loop runs, it will also sleep 50ms and processEvents each cycle.
        //    When the process terminates or render is cancelled by the UI, the loop breaks.
        //
        //////////////////////////////////////////////
        while (YaLuxGlobal.bFrameisFinished == false)
        {
            // DEBUG
            //process->waitForFinished();
            // double check process
            // DEBUG
            int processState = -1;
            processState = YaLuxGlobal.luxRenderProc->state();
            if ( (processState == QProcess::NotRunning) || (YaLuxGlobal.RenderProgress->isCancelled() == true) )
//            if ( (YaLuxGlobal.RenderProgress->isCancelled() == true) )
            {
                if (YaLuxGlobal.debugLevel >= 2) // debugging data
                    dzApp->log("yaluxplug: Rendering, progress exited or cancelled.");
                if (YaLuxGlobal.luxRenderProc->state() == QProcess::Running)
                    YaLuxGlobal.luxRenderProc->terminate();
                // if network rendering, also send terminate signal to render servers
                if (YaLuxGlobal.bNetworkRenderOn)
                    resetRenderServers();
                // Set bFrameisFinished to true to indicate stopping the frame render loop
                YaLuxGlobal.bFrameisFinished = true;
                break;
            }
            else
            {
                // Update the log window

    //            process->waitForFinished();

    //            process->waitForReadyRead(5000);
                //processRenderLog(process, logFile, true);

//                processCoreRenderLog(YaLuxGlobal.luxRenderProc, logFile, true);

            }


            QCoreApplication::processEvents(QEventLoop::AllEvents);

            int timeout2 = 50;
#ifdef Q_OS_WIN
            Sleep(uint(timeout2));
#else
            struct timespec ts = { timeout2 / 1000, (timeout2 % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
#endif

        } // while (YaLuxGlobal.bFrameisFinished == false)
        //////////////////////////////////////////////
        //
        // End of frame rendering loop
        //
        /////////////////////////////////////////////

//        tmr.stop();
//        disconnect(&tmr, SIGNAL(timeout()),
//                this, SLOT(updateData()) );

        // Read the remainder of the stdoutput to the logfile
        // but don't update the image since this was already done when process sent the finish() signal.
        //processRenderLog(process, logFile, false);
        //processCoreRenderLog(YaLuxGlobal.luxRenderProc, logFile, false);

        // If Renderprogress is cancelled of bIsCancelled is true, then stop the animation rendering
        if ( (YaLuxGlobal.RenderProgress->isCancelled() == true) || (YaLuxGlobal.bIsCancelled == true) )
        {
            //YaLuxGlobal.luxRenderProc->deleteLater();
            YaLuxGlobal.inProgress = false;
            YaLuxGlobal.RenderProgress->finish();
            YaLuxGlobal.FrameProgress->finish();
            logFile.close();
            return false;
        }

        // if not cancelled, then update frame counts, progress and continue
        YaLuxGlobal.frame_counter++;
        YaLuxGlobal.activeFrame++;

        YaLuxGlobal.FrameProgress->finish();
        mesg = "Frame completed.";
        YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
        emit updateLogWindow("yaluxplug: " + mesg, QColor(255, 255, 255), true);
        progressFraction = ((float)YaLuxGlobal.frame_counter)/((float)YaLuxGlobal.totalFrames);
        updateProgress = 5 + (progressFraction)*90;
        YaLuxGlobal.RenderProgress->update( updateProgress );

    } // while (YaLuxGlobal.frame_counter < YaLuxGlobal.totalFrames)
    //////////////////////
    //
    // End of animation rendering loop
    //
    //////////////////////////

    emit killRender();
    handler->finishRender();
    YaLuxGlobal.RenderProgress->finish();
    YaLuxGlobal.FrameProgress->finish();
    YaLuxGlobal.inProgress = false;

    logFile.close();

//    disconnect(YaLuxGlobal.luxRenderProc, SIGNAL( finished(int, QProcess::ExitStatus) ),
//                this, SLOT( handleRenderProcessComplete(int, QProcess::ExitStatus) ) );
//    YaLuxGlobal.luxRenderProc->deleteLater();

    return true;
}

void YaLuxRender::resetRenderServers()
{
    QProcess cmdProc;
    QStringList terminateCommand;
    for (int i=0; i<YaLuxGlobal.slaveNodeList.count(); i++)
    {
        //                        terminateCommand << QString("--resetserver %1").arg(YaLuxGlobal.slaveNodeList[i]);
        terminateCommand << QString("--resetserver") << YaLuxGlobal.slaveNodeList[i];
    }
    // use YaLuxGlobal.LuxExecPath, since this will  be set to luxconsole
    cmdProc.start(YaLuxGlobal.LuxExecPath, terminateCommand);
    cmdProc.waitForFinished();
    QByteArray qa = cmdProc.readAllStandardError();
    YaLuxGlobal.logText->append( QString(qa) );
    //                    YaLuxGlobal.RenderProgress->setInfo( QString(qa) );
}

void YaLuxRender::processCoreRenderLog(QProcess* process, QFile& logFile, bool bUpdateRender)
{

    // Read the remainder of the stdoutput to the logfile
    if (YaLuxGlobal.bShowLuxRenderWindow)
    {
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setReadChannel(QProcess::StandardOutput);
    }
    while ( process->canReadLine() )
    {

        // NOTE: we don't need to process the "writing tonemapped PNG" because a
        //   final loading of the file was done when the process called the finish() signal.
        QByteArray qa = process->readLine();

        if (qa.contains("Outputting film:"))
        {
            if (bUpdateRender)
                updateData();
        }
        else if (qa.contains("ERROR"))
        {
            emit updateLogWindow(QString(qa.data()), QColor(255, 0, 0), true);
        }
        else if (qa.contains("Lux version"))
        {
            emit updateLogWindow(QString(qa.data()), QColor(0, 255, 0), true);
        }
        else if (qa.contains("rendering done"))
        {
            emit updateLogWindow(QString(qa.data()), QColor(0, 255, 0), true);
        }
        else if (qa.contains("server"))
        {
            emit updateLogWindow(QString(qa.data()), QColor(100, 200, 255), true);
        }
        else if (qa.contains("Tessellating"))
        {
            emit updateLogWindow(QString(qa.data()));
        }
        else if ( qa.contains("% T)") || qa.contains("% Thld)") || qa.contains("Elapsed time") )
        {
            if (YaLuxGlobal.debugLevel >= 1)
            {
                emit updateLogWindow(QString(qa.data()), QColor(100, 200, 255));
            }
            QRegExp regexp("\\(([\\d]*)% T\\)");
            if (regexp.indexIn(QString(qa)) != -1)
            {
                QString percentString = regexp.cap(1);
//                YaLuxGlobal.FrameProgress->setInfo( QString("Frame render: %1\% completed").arg(percentString));
//                YaLuxGlobal.FrameProgress->update(percentString.toInt());
            }

        }
        //else if (qa.contains("INFO") && YaLuxGlobal.debugLevel >= 2)
        else if ( YaLuxGlobal.debugLevel >= 2 )
        {
            emit updateLogWindow(QString(qa.data()), QColor(255, 255, 255));
        }
        logFile.write(qa);

    }
    logFile.flush();

}


void YaLuxRender::processRenderLog(QProcess *process, QFile &logFile, bool bUpdateRender)
{
//
//    // Read the remainder of the stdoutput to the logfile
//    while (process->canReadLine() )
//    {
//        // NOTE: we don't need to process the "writing tonemapped PNG" because a
//        //   final loading of the file was done when the process called the finish() signal.
//        QByteArray qa = process->readLine();
//
//        if (qa.contains("Writing Tonemapped"))
//        {
//            if (bUpdateRender)
//                updateData();
//        } 
//        else if (qa.contains("ERROR"))
//        {
//            logToWindow( QString(qa.data()), QColor(255,0,0), true);
//
//        } 
//        else if (qa.contains("Lux version"))
//        {
//            logToWindow( QString(qa.data()), QColor(0,255,0), true);
//        } 
//        else if (qa.contains("100% rendering done"))
//        {
//            logToWindow( QString(qa.data()), QColor(0,255,0), true);
//        } 
//        else if ( qa.contains("server"))
//        {
//            logToWindow( QString(qa.data()), QColor(100,200,255), true);
//        } 
//        else if ( qa.contains("Tessellating"))
//        {
//            logToWindow( QString(qa.data()) );
//        } 
//        else if ( (qa.contains("% T)") || qa.contains("% Thld)") ) )
//        {
//            if (YaLuxGlobal.debugLevel >= 1)
//            {
//                logToWindow( QString(qa.data()), QColor(100,200,255) );
//            }
//            QRegExp regexp("\\(([\\d]*)% T\\)");
//            if ( regexp.indexIn( QString(qa) ) != -1 )
//            {
//                QString percentString = regexp.cap(1);
////                YaLuxGlobal.FrameProgress->setInfo( QString("Frame render: %1\% completed").arg(percentString));
//                YaLuxGlobal.FrameProgress->update(percentString.toInt());
//            }
//
//        } else if (qa.contains("INFO") && YaLuxGlobal.debugLevel >= 2)
//        {
//            QString newInfo = QString( qa.data() );
//            newInfo = newInfo.replace("\n", "");
//            YaLuxGlobal.logText->setTextColor( QColor(255,255,255) );
//            YaLuxGlobal.logText->append( newInfo );
//        }
//        logFile.write(qa);
//    }
//    logFile.flush();

}

void YaLuxRender::handleLogWindow( QString data, QColor textcolor, bool bIsBold )
{
    QString formated = data.replace("\n", "").replace("\r", "");
    if (bIsBold) YaLuxGlobal.logText->setFontWeight(QFont::Bold);
    YaLuxGlobal.logText->setTextColor( textcolor );
    if (formated != "") YaLuxGlobal.logText->append( formated );
    YaLuxGlobal.logText->setTextColor( QColor(255,255,255) );
    if (bIsBold) YaLuxGlobal.logText->setFontWeight(QFont::Normal);

}


void YaLuxRender::updateData()
{
    int timeout = 5;
#ifdef Q_OS_WIN
    Sleep(uint(timeout));
#else
    struct timespec ts = { timeout / 1000, (timeout % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif

    QImage *qimg = new QImage();
    DzRenderData *data;
    QFile imgFile(YaLuxGlobal.workingRenderFilename);

    QByteArray qa;

    for (int attempts=0; attempts <= 2; attempts++)
    {
        if (imgFile.open(QIODevice::ReadOnly) == true)
        {
            qa = imgFile.readAll();
            imgFile.close();

            if (qimg->loadFromData(qa) == true)
            {
                break;
            }
        }

        if (attempts == 2)
        {
            QString mesg = "yaluxplug: timeout waiting for preview image from luxrender.";
            dzApp->log(mesg);
            YaLuxGlobal.logText->setTextColor(QColor(255, 255, 0));
            YaLuxGlobal.logText->append(mesg);
            return;
        }

#ifdef Q_OS_WIN
        Sleep(uint(timeout));
#else
        nanosleep(&ts, NULL);
#endif

    }

    data = new DzRenderData(YaLuxGlobal.cropWindow.top(), YaLuxGlobal.cropWindow.left(), qimg->convertToFormat(QImage::Format_ARGB32));
    YaLuxGlobal.handler->passData( (*data) );

    delete qimg;
    // delete data;
}

void YaLuxRender::handleRenderProcessStateChange( QProcess::ProcessState newstate )
{
    QString newStateString;
    if (newstate == QProcess::NotRunning)
    {
        YaLuxGlobal.bFrameisFinished = true;
        newStateString = "Not running";
    }
    else if (newstate == QProcess::Running)
    {
        newStateString = "Running";
    }
    else if (newstate == QProcess::Starting)
    {
        newStateString = "Starting";
    }

    emit updateLogWindow( "yaluxplug: process state changed: " + newStateString, QColor(0, 255, 0), true);
}

void YaLuxRender::handleRenderProcessComplete( int exitCode, QProcess::ExitStatus status )
{
    QImage *qimg = new QImage();
    DzRenderData *data;

    if (status == QProcess::CrashExit)
    {
        // ** TODO: detect intentional cancellation ** //
        // If Renderprogress is cancelled of bIsCancelled is true, then stop the animation rendering
        if ((YaLuxGlobal.RenderProgress->isCancelled() == true) || (YaLuxGlobal.bIsCancelled == true))
        {
            // Cancel message here
            QString error = QString("yaluxplug: INFO: luxrender process stopped: exitCode=%1").arg(exitCode);
            emit updateLogWindow(QString(error), QColor(0, 255, 0), true);
            dzApp->log(QString("yaluxplug: RENDER PROCESS exited with %1 ").arg(exitCode));
        }
        else
        {
            QString error = QString("yaluxplug: ERROR: luxrender process stopped unexpectedly: exitCode=%1").arg(exitCode);
            emit updateLogWindow(QString(error), QColor(255, 0, 0), true);
            dzApp->log(error);
        }
    }

    // HANDLE ANIMATION CASE, CONTINUE TO NEXT FRAME
    if (YaLuxGlobal.inProgress == false)
    {
        emit frameFinished();
        YaLuxGlobal.bFrameisFinished = true;
        return;
    }

    if (qimg->load(YaLuxGlobal.workingRenderFilename) == true)
    {
        data = new DzRenderData(YaLuxGlobal.cropWindow.top(), YaLuxGlobal.cropWindow.left(), qimg->convertToFormat(QImage::Format_ARGB32));
        YaLuxGlobal.handler->passData( (*data) );
        QString tempRenderName = dzApp->getTempRenderFilename() + ".png";
        qimg->save(tempRenderName);
        delete qimg;
    }

    if (YaLuxGlobal.bShowLuxRenderWindow == true && (YaLuxGlobal.totalFrames == 1) )
    {
        YaLuxGlobal.handler->beginFrame(YaLuxGlobal.frame_counter);
    }
    emit frameFinished();
    YaLuxGlobal.bFrameisFinished = true;

    if (YaLuxGlobal.activeFrame <= YaLuxGlobal.endFrame)
    {
        YaLuxGlobal.inProgress = false;
        emit renderFinished(this);
    }

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
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call customRender()");
    
    return true;
}

DzOptionsFrame* YaLuxRender::getOptionsFrame() const
{
    if (YaLuxGlobal.optFrame == NULL)
    {
        YaLuxGlobal.optFrame = new YaLuxOptionsFrame();
    }
    
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: creating options frame");
    
    return YaLuxGlobal.optFrame;
}

DtFilterFunc YaLuxRender::getFilterFunction(DzRenderOptions::PixelFilter filterType) const
{
    DtFilterFunc fpResult = DI_NULL;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
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
    int WidthResize = YaLuxGlobal.maxTextureSize;

    imgSize = img->getOriginalImageSize();
    if ( (YaLuxGlobal.maxTextureSize == -1) || (imgSize.width() <= WidthResize) )
    {
        // this size is good, just keep it
        emit imagePrepared(img, filename);
        return;
    }

    // otherwise, spawn a thread to do it after loading completed

    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: prepareImage( " + filename + " ) - Starting New Thread" );
//    QThread *thread = new QThread;

    // Update the BackgroundProgress
    if (YaLuxGlobal.backgroundProgress == NULL)
    {
        YaLuxGlobal.backgroundProgress = new DzBackgroundProgress("yaluxplug: Generating Image Cache...", 100, false);
    }
    // recalculate the progress bar
    YaLuxGlobal.currentBackgroundProgress = (YaLuxGlobal.currentBackgroundProgress * YaLuxGlobal.numBackgroundThreads) / (YaLuxGlobal.numBackgroundThreads+1);
    // ** make sure this is the only place that increments the numBackgroundThreads **
    YaLuxGlobal.numBackgroundThreads++;
    YaLuxGlobal.backgroundProgress->update( YaLuxGlobal.currentBackgroundProgress * 100);

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

    YaLuxGlobal.currentBackgroundProgress += 1/YaLuxGlobal.numBackgroundThreads;
    YaLuxGlobal.backgroundProgress->update( YaLuxGlobal.currentBackgroundProgress * 100 );

    // ** make sure this is the only place that decrements the numBackgroundThreads **
    YaLuxGlobal.numBackgroundThreads--;
    if (YaLuxGlobal.numBackgroundThreads == 0)
    {
        YaLuxGlobal.backgroundProgress->finish();
        delete YaLuxGlobal.backgroundProgress;
        YaLuxGlobal.backgroundProgress = NULL;
    }
};


QString YaLuxRender::compileShader(const QString &shaderPath)
{
    QString sResult = shaderPath;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DEBUG: unimplemented call: compileShader (" + shaderPath + ").");

    return sResult;
};

QString YaLuxRender::compileShader(const QString &shaderPath, QString &output)
{
    QString sResult = shaderPath;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DEBUG: compileshader (" + shaderPath + "," + output +").");
    output = "yaluxplug: compiling shader for " + shaderPath;
    
    return sResult;
}

DzShaderDescription* YaLuxRender::getShaderInfo(const QString &shaderPath)
{
    DzShaderDescription* oResult = NULL;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DEBUG: getShaderInfo called (" + shaderPath + ").");
    return oResult;
}

void YaLuxRender::killRender()
{
    // stop rendering now
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: killRender was called.");

    if (YaLuxGlobal.luxRenderProc != NULL)
    {
        if (YaLuxGlobal.luxRenderProc->state() == QProcess::Running)
        {
            // Kill Render process
            YaLuxGlobal.RenderProgress->cancel();
            YaLuxGlobal.bIsCancelled = true;
            YaLuxGlobal.luxRenderProc->kill();
        }
    }

    return;
};

bool YaLuxRender::bake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt )
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call bake().");
    return false;
}

bool YaLuxRender::autoBake( DzRenderHandler *handler, DzCamera *camera, DzLightListIterator &lights, DzNodeListIterator &nodes, const DzBakerOptions &opt )
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call autoBake();");
    return false;
}

void YaLuxRender::stopBaking()
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call stopBaking();");
    return;
}

void YaLuxRender::saveBakeImage( const DzBakerOptions &opt, bool wait )
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call saveBakeImage()");
    return;
}

bool YaLuxRender::textureConvert( DzRenderHandler *handler, DzCamera *camera, const DzTextureConvertorOptions &opt )
{
    // convert a texture for rendering?
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call textureConvert()");

    return false;
}    


//
// ACCESSORS (public slot)
//

QString YaLuxRender::getShaderCompilerPath()
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getShaderCompilerPath()");
    QString sResult = "";
    
    return sResult;
}
                                                                                                                                                  

QString YaLuxRender::getTextureUtilityPath()
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getTextureUtilityPath()");
    QString sResult = "";
    
    return sResult;
    
};

QStringList YaLuxRender::getShaderSearchPaths() const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getShaderSearchPaths()");
    QStringList oResult = QStringList() << "";
    return oResult;
};

QString YaLuxRender::processShaderName( const QString &shaderName ) const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call processShaderName()");
    QString sResult = NULL;
    
    return sResult;
};


QString YaLuxRender::getShaderPath( const QString &shaderName, bool withExtension ) const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getShaderPath()");
    QString sResult = NULL;
    
    return sResult;
    
}

QString YaLuxRender::getShaderFileName( const QString &shaderName ) const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getShaderFileName()");
    QString sResult = NULL;
    
    return sResult;
};

QString YaLuxRender::getShaderExtension() const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getShaderExtension()");
    QString sResult = NULL;
    
    return sResult;
    
}

bool YaLuxRender::isRendering() const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: call to isRendering() - returning %1").arg(YaLuxGlobal.inProgress) );
    return YaLuxGlobal.inProgress;
};

QString YaLuxRender::getName() const
{
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getName()");
    QString sResult = "yaluxplug Render";
    
    return sResult;
    
}

DzNode* YaLuxRender::getCurrentNode() const
{    
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: unimplemented call getCurrentNode()");
    return YaLuxGlobal.currentNode;
}



///////////////////////////////////////////////////////////////////////
// protected (Renderman)
///////////////////////////////////////////////////////////////////////
DtVoid YaLuxRender::DiBegin(DtToken name)
{
    // Create new RenderMan rendering context
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DiBegin called ( " + QString(name) + " ).");
    
};

DtVoid YaLuxRender::DiEnd()
{
    // Terminate the active rendering context
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DiEnd called.");
    
};

DtVoid YaLuxRender::DiFrameBegin(DtInt number)
{
    // Mark beginning of single frame of animated sequence
    // All information that is frame specific should be removed after DiFrameEnd()
    // Not needed for a single image
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log("yaluxplug: DiFrameBegin called ( " + QString(number) + " .");

};

DtVoid YaLuxRender::DiFrameEnd()
{
    // Mark end of single frame of animated sequence
    if (YaLuxGlobal.debugLevel >=2) // debugging data
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
    
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: DiDeclare( %1, \"%2\" ) == newToken: [%3]").arg(name).arg(declaration).arg((DtToken)newToken.toAscii()) );
    
    return newToken.toAscii();
};


DtVoid YaLuxRender::DiAttributeV( DtToken name, DtInt n, const DtToken tokens[], DtPointer params[] )
{
    int count = 0;

    if (YaLuxGlobal.debugLevel <2) // debugging data
        return;

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
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: DiColor( [%1 %2 %3] )").arg( fltArray[0] ).arg( fltArray[1] ).arg( fltArray[2] ) );

};


DtVoid YaLuxRender::DiOpacity( DtColor Cs )
{ 
    DtFloat *fltArray = Cs;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: DiOpacity( [%1 %2 %3] )").arg( fltArray[0] ).arg( fltArray[1] ).arg( fltArray[2] ) );
   
};


DtVoid YaLuxRender::DiTransform( DtMatrix transform )
{ 
    DtFloat *mat4 = (DtFloat*)&transform;
    if (YaLuxGlobal.debugLevel >=2) // debugging data
        dzApp->log( QString("yaluxplug: DiTransform( [%1 %2 %3 %4] )").arg(mat4[0]).arg(mat4[1]).arg(mat4[2]).arg(mat4[3]) );

};


DtVoid YaLuxRender::DiPointsPolygonsV( DtInt npolys, DtInt *nverts, DtInt *verts,
                                          DtInt n, const DtToken tokens[], DtPointer params[] )
{ 
    if (YaLuxGlobal.debugLevel <2) // debugging data
        return;

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
    if (YaLuxGlobal.debugLevel <2) // debugging data
        return;

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

#include "moc_renderer.cpp"

// End of File