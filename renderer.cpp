//
//  renderer.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
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

#include "renderoptions.h"

#include "dazToPLY.h"
#include "renderer.h"
#include "plugin.h"

const QStringList LXSfilm = QStringList() <<
    "Film \"fleximage\"\n" <<
    "\t\"integer outlierrejection_k\"\t[3]\n" <<
    "\t\"bool premultiplyalpha\"\t[\"false\"]\n" <<
    "\t\"float gamma\"\t[2.2]\n" <<
    "\t\"integer displayinterval\"\t[5]\n" <<
    "\t\"integer writeinterval\"\t[5]\n" <<
    "\t\"string tonemapkernel\"\t[\"autolinear\"]\n" <<
//png
    "\t\"bool write_png\"\t[\"true\"]\n" <<
    "\t\"string write_png_channels\"\t[\"RGB\"]\n" <<
    "\t\"bool write_png_16bit\"\t[\"false\"]\n" <<
    "\t\"bool write_png_gamutclamp\"\t[\"true\"]\n" <<
//tga...
//exr...
//flm...
//colorspace...
    "\t\"bool debug\"\t[\"true\"]\n";

const QStringList LXSpixelfilter = QStringList() <<
    "PixelFilter \"mitchell\"\n" <<
    "\t\"float xwidth\"\t[1.5]\n" <<
    "\t\"float ywidth\"\t[1.5]\n" <<
    "\t\"float B\"\t[0.3333]\n" <<
    "\t\"float C\"\t[0.3333]\n" <<
    "\t\"bool supersample\"\t[\"true\"]\n";
    
const QStringList LXSaccelerator = QStringList() <<
    "Accelerator \"qbvh\"\n" <<
    "\t\"integer maxprimsperleaf\"\t[4]\n" <<
    "\t\"integer fullsweepthreshold\"\t[16]\n" <<
    "\t\"integer skipfactor\"\t[1]\n";
    
const QStringList LXSsampler = QStringList() <<
    "Sampler \"metropolis\"\n" <<
    "\t\"float largemutationprob\"\t[0.40]\n" <<
    "\t\"bool noiseaware\"\t[\"false\"]\n" <<
    "\t\"bool usecooldown\"\t[\"false\"]\n";

const QStringList LXSsurfaceintegrator = QStringList() <<
    "SurfaceIntegrator \"path\"\n" <<
    "\t\"integer maxdepth\"\t[16]\n" <<
    "\t\"string lightstrategy\"\t[\"one\"]\n" <<
    "\t\"bool includeenvironment\"\t[\"true\"]\n";

const QStringList LXSvolumeintegrator = QStringList() <<
    "VolumeIntegrator \"multi\"\n" <<
    "\t\"float stepsize\"\t[1.0]\n";

///////////////////////////////////////////////////////////////////////
// yaluxplug - YaLuxRender class
///////////////////////////////////////////////////////////////////////

/**
**/
YaLuxRender::YaLuxRender() 
{
    
//    dzApp->log("yaluxplug: initializing options");
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


/*
void TreRenderProgress::cancel()
{
    this->finish();

    YaLuxGlobal.handler->finishRender();

}

TreRenderProgress::TreRenderProgress(QString string, int steps)
    : DzProgress(string, steps)
{

}
*/

bool YaLuxRender::render(DzRenderHandler *handler, DzCamera *camera, const DzRenderOptions &opt)
{
    QSize tresize;
    int ImgHeight, ImgWidth;
    QString mesg;
    QString fileName;
    QString fileNameLXS;
    QString tempPath;
    int steps = 10;
    int ncount = 0;

    dzApp->log("\nyaluxplug: render() called.");
    YaLuxGlobal.inProgress = true;

    YaLuxGlobal.handler = handler;

    YaLuxGlobal.RenderProgress = new DzProgress("yaluxplug Render Started", steps);
    YaLuxGlobal.RenderProgress->setShowTimeElapsed(DI_TRUE);
//    YaLuxGlobal.RenderProgress->setCloseOnFinish(DI_FALSE);
//    YaLuxGlobal.RenderProgress->setUseCloseCheckbox(DI_TRUE);

    fileName = dzApp->getTempFilename();
    YaLuxGlobal.pathTempName = fileName;
    // DEBUG
    dzApp->log("yaluxplug: pathTempName = [" + fileName + "]");
    fileNameLXS = fileName + ".lxs";
//    tempPath = dzApp->getTempPath();
//    fileName.replace(tempPath + "/","");
    fileName = DzFileIO::getBaseFileName( fileName );
    mesg = "Writing to LXS file = " + fileName;
//    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->step(10);
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
    
    // open stream to write to file
    dzApp->log("yaluxplug: opening LXSFile = " + fileNameLXS + " and BASEfilename = " + fileName );
    QFile outLXS(fileNameLXS);
    outLXS.open(QIODevice::ReadWrite);
    outLXS.write("# Generated by YaLuxRender \n");
    
    // 1. read render options to set up environment
    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->setCurrentInfo("LXS write buffer opened.");
 
    tresize = opt.getImageSize();
    YaLuxGlobal.RenderProgress->update(ncount++);
    ImgHeight = tresize.height();
    ImgWidth = tresize.width();

    // sampler settings
    outLXS.write("\n");
    outLXS.write(LXSsampler.join(""));
    
    // surface integrator settings
    outLXS.write("\n");
    outLXS.write(LXSsurfaceintegrator.join(""));
    
    // volume integrator settings
    outLXS.write("\n");
    outLXS.write(LXSvolumeintegrator.join(""));

    // pixelfilter settings
    outLXS.write("\n");
    outLXS.write(LXSpixelfilter.join(""));

    // accelerator settings
    outLXS.write("\n");
    outLXS.write(LXSaccelerator.join(""));
        
    // Lookat
    const float matrixLux[16] = {
        0.01, 0, 0, 0,
        0, 0, 0.01, 0,
        0, -0.01, 0, 0, 
        0, 0, 0, 1 };
    DzMatrix4 luxTransform( matrixLux );
    DzVec3 pos1 = camera->getWSPos();
    DzVec3 target1 = camera->getFocalPoint();
    DzQuat rot = camera->getWSRot();
    DzVec3 up1 = rot.multVec(DzVec3(0,100,0));
    outLXS.write("\n");
    DzVec3 pos = luxTransform.multVecMatrix(pos1);
    DzVec3 target = luxTransform.multVecMatrix(target1);
    DzVec3 up = luxTransform.multVecMatrix(up1);
    mesg = QString("LookAt %1 %2 %3 %4 %5 %6 %7 %8 %9\n").arg(pos.m_x).arg(pos.m_y).arg(pos.m_z).arg(target.m_x).arg(target.m_y).arg(target.m_z).arg(up.m_x).arg(up.m_y).arg(up.m_z);
    outLXS.write(mesg);

    // 2. set the camera vector and orientation
    // Camera
    outLXS.write("\nCamera \"perspective\"\n");
    // fov
    double f_fov = camera->getFieldOfView() * 57.295779513; // multiply by (180/pi)
    outLXS.write(QString("\t\"float fov\"\t[%1]\n").arg(f_fov));
    // screenwindow
    double aspectRatio = opt.getAspect();
    if (aspectRatio == 0)
        aspectRatio = ImgWidth/ImgHeight; 
    float screenWindow[4];
//    if (aspectRatio > 1)
//    {
        screenWindow[0] = -aspectRatio;
        screenWindow[1] = aspectRatio;
        screenWindow[2] = -1;
        screenWindow[3] = 1;
//    } 
/*    else
    {
        screenWindow[0] = -1;
        screenWindow[1] = 1;
        screenWindow[2] = -1/aspectRatio;
        screenWindow[3] = 1/aspectRatio;
    }
    */
    outLXS.write(QString("\t\"float screenwindow\"\t[%1 %2 %3 %4]\n").arg(screenWindow[0]).arg(screenWindow[1]).arg(screenWindow[2]).arg(screenWindow[3]));
    
    // Film image settings
    outLXS.write("\n");
    outLXS.write(LXSfilm.join(""));
    
    // Film resolution
    mesg = QString("\t\"integer xresolution\"\t[%1]\n").arg(ImgWidth);
    mesg += QString("\t\"integer yresolution\"\t[%1]\n").arg(ImgHeight);
    outLXS.write(mesg);
    mesg = QString("Size is %1 x %2").arg(ImgHeight).arg(ImgWidth);
    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
    
    // Film image filename
    mesg = QString("\t\"string filename\"\t[\"%1\"]\n").arg(fileName);
    outLXS.write(mesg);
    YaLuxGlobal.RenderProgress->update(ncount++);
    //    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
    
    // Renderer
    outLXS.write("\nRenderer \"slg\"\n");
    mesg = QString("\t\"string config\" [\"renderengine.type = %1\" \"opencl.cpu.use = 0\" \"opencl.kernelcache = %2\"]\n").arg("PATHOCL").arg("PERSISTENT");
    outLXS.write(mesg);
    
    // single image
    switch (opt.getRenderImgToId())
    {
    case DzRenderOptions::ActiveView:
    case DzRenderOptions::NewWindow:
    case DzRenderOptions::DirectToFile:        
            fileName = opt.getRenderImgFilename();
    }
//    fileName = dzApp->getTempRenderFilename();
//    fileName.replace(tempPath, "");
    mesg = "image filename = " + fileName;
    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
    
    // image series
    switch (opt.getRenderMovToId())
    {
    case DzRenderOptions::MovieFile:
            fileName = opt.getRenderMovFilename();
    case DzRenderOptions::ImageSeries:
            fileName = opt.getRenderSerFilename();
    }
    mesg = "movie filename = " + fileName;
    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);
        
    DzTimeRange timeRange;
    timeRange.setEnds(opt.getStartTime(),opt.getEndTime());
    DzTime timeStep = dzScene->getTimeStep();
    mesg = QString("TimeStep=%1.\nTimeStart = %2.\nTimeEnd = %3.").arg(timeStep).arg(timeRange.getStart()).arg(timeRange.getEnd());
    YaLuxGlobal.RenderProgress->update(ncount++);
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);

    // World Begin
    outLXS.write("\nWorldBegin\n");
    //    outLXS.write("\nCoordSysTransform \"camera\"\n");
    
    // LIGHTS
    if (dzScene->getNumLights() == 0)
    {
        // Create a default light
        // change light type to sun and sky
        outLXS.write("\nAttributeBegin\n");        
        outLXS.write("LightSource \"sun\"\n");
        outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1));
        outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-1).arg(0.25).arg(1));
        outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(1.0));
        
        outLXS.write("LightSource \"sky2\"\n");
        outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1));
        outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(0).arg(1).arg(0));
        outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(1.0));        
        outLXS.write("AttributeEnd\n");
    }
    DzLightListIterator lightList = dzScene->lightListIterator();
    DzLight *currentLight = NULL;
    while (lightList.hasNext())
    {
        currentLight = lightList.next();
        // DEBUG
        LuxProcessLight(currentLight, mesg);
        dzApp->log(mesg);
        mesg = currentLight->getLabel();
        outLXS.write("\nAttributeBegin\n");
        outLXS.write( QString("LightGroup\t\"%1\" # %2\n").arg(currentLight->getAssetId()).arg(mesg) );
        if (mesg.contains("Sun"))
        {
            // change light type to sun and sky
            outLXS.write("LightSource \"sun\"\n");
            outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1));
            target = currentLight->getWSDirection();
            outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-target.m_x).arg(target.m_z).arg(-target.m_y));
            outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity() ));
            
        }
        if (mesg.contains("Sky"))
        {
            outLXS.write("LightSource \"sky2\"\n");
            outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1));
            outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-target.m_x).arg(target.m_z).arg(-target.m_y));
            outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity() ));            
        }
        if ( !(mesg.contains("Sun")) && !(mesg.contains("Sky")) )
        {
            if (currentLight->inherits("DzSpotLight"))
            {
                mesg = "spot";
                outLXS.write("LightSource \"" + mesg + "\"\n");
                pos = currentLight->getWSPos();
                pos = luxTransform.multVecMatrix(pos);
                target = currentLight->getFocalPoint();
                target = luxTransform.multVecMatrix(target);
                outLXS.write(QString("\t\"point from\"\t[%1 %2 %3]\n").arg(pos.m_x).arg(pos.m_y).arg(pos.m_z));                
                outLXS.write(QString("\t\"point to\"\t[%1 %2 %3]\n").arg(target.m_x).arg(target.m_y).arg(target.m_z));                                     
            } else if (currentLight->inherits("DzPointLight")) {
                mesg = "point";
                outLXS.write("LightSource \"" + mesg + "\"\n");
                pos = currentLight->getWSPos();
                pos = luxTransform.multVecMatrix(pos);
                outLXS.write(QString("\t\"point from\"\t[%1 %2 %3]\n").arg(pos.m_x).arg(pos.m_y).arg(pos.m_z));                  
            } else if (currentLight->inherits("DzDistantLight")) {
                mesg = "distant";
                outLXS.write("LightSource \"" + mesg + "\"\n");
                pos = currentLight->getWSPos();
                pos = luxTransform.multVecMatrix(pos);
                outLXS.write(QString("\t\"point from\"\t[%1 %2 %3]\n").arg(pos.m_x).arg(pos.m_y).arg(pos.m_z));
                target = currentLight->getFocalPoint();
                target = luxTransform.multVecMatrix(target);                                  
                outLXS.write(QString("\t\"point to\"\t[%1 %2 %3]\n").arg(target.m_x).arg(target.m_y).arg(target.m_z)); 
                //target = currentLight->getWSDirection();
                //outLXS.write(QString("\t\"point to\"\t[%1 %2 %3]\n").arg(target.m_x).arg(-target.m_z).arg(target.m_y));                                                       
            }
            QColor lightColor = currentLight->getDiffuseColor();
            outLXS.write(QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF()));
            outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(1.0));
        }

        outLXS.write("AttributeEnd\n");
    }

    // end LIGHTS

/*
    // IMAGES TEXTURES
    // get list of texture files
    // print to Log
    QList<DzTexturePtr> textureList;
    DzTexture *textureptr;
    QSize size;

    dzScene->getActiveImages(textureList);
    int num_items = textureList.count();
    while (i < num_items)
    {
        textureptr = textureList[i];
        fileName = textureptr->getFilename();
        mesg = textureptr->getTempFilename();
        //dzApp->log( QString("yaluxplug: getActiveImages[%1] = \"%2\"\n\ttempfilename = \"%3\"").arg(i).arg(fileName).arg(mesg) );
        i++;
    }
*/
    DzImageMgr *imageMgr = dzApp->getImageMgr();

    // imageMgr->prepareAllImages(this);
    emit imageMgr->prepareAllImages(this);
    

    // NODES
    DzNodeListIterator nodeList = dzScene->nodeListIterator();
    DzNode *currentNode;
    QString label;
    int parentNodeIndex=0;
    while (nodeList.hasNext())
    {
        currentNode = nodeList.next();
        label = currentNode->getAssetId();
        
        // Process Node
        YaLuxGlobal.currentNode = currentNode;
        YaLuxGlobal.settings = new DzRenderSettings(this, &opt);
        dzApp->log("yaluxplug: Looking at Node: " + label + QString("(%1 of %2 Scene Level Nodes)").arg(parentNodeIndex++).arg(dzScene->getNumNodes()) );

        // FINALIZE Node's geometry cache for rendering
        currentNode->finalize(true,true);

        // Node -> Object
        DzObject *currentObject = currentNode->getObject();
        if (currentObject != NULL)
        {
            outLXS.write("\n# " + label +"\n");
            QString objectLabel = currentObject->getLabel();

            mesg = LuxProcessObject(currentObject);
            outLXS.write(mesg);
/*
            // Object -> Shape
            DzShape *currentShape = currentObject->getCurrentShape();
            QString shapeLabel = currentShape->getLabel();

            // Shape -> Geometry
            DzGeometry *currentGeometry = currentObject->getCachedGeom();
            QString geoLabel = QString("numvertices = %1").arg(currentGeometry->getNumVertices() );

            dzApp->log( QString("\tobject = [%1], shape = [%2], %3").arg(objectLabel).arg(shapeLabel).arg(geoLabel) ) ;
            
            // TRE --> This is all redundant!
            // ProcessEachFaceGroup of Geometry
            // change geo to a facetmesh
            if ( currentGeometry->inherits("DzFacetMesh") )
            {
                DzFacetMesh *mesh = (DzFacetMesh*)currentGeometry;
                DzFacet *ptrFace = mesh->getFacetsPtr();
                // get num facegroups in facetmesh
                int numFaceGroups = mesh->getNumFaceGroups();

                int indexCurrentFaceGroup=0;
                while (indexCurrentFaceGroup < numFaceGroups)
                {
                    DzFaceGroup *faceGroup = mesh->getFaceGroup(indexCurrentFaceGroup);
                    QString facegroupName = faceGroup->getName();
                    int numFaceVerts = -1;
                    int numFaces = faceGroup->count();
                    int indexCurrentFace = 0;
                    while (indexCurrentFace < 0)
                    {
                        int indexToMesh = faceGroup->getIndex(indexCurrentFace);
                        // look up index in the faceMesh
                        // ...
                        //ptrFace[indexToMesh].isTri();
                        indexCurrentFace++;
                    }
                    //numFaceVerts = faceGroup->getNumVerts();
                    dzApp->log( QString("\tshape[%1]: faceGroup(%2/%3) = [%4] has numfaceverts = [%5]").arg(shapeLabel).arg(indexCurrentFaceGroup+1).arg(numFaceGroups).arg(facegroupName).arg(numFaceVerts) );
                    indexCurrentFaceGroup++;
                }
                int numMatGroups = mesh->getNumMaterialGroups();
                indexCurrentFaceGroup = 0;
                while (indexCurrentFaceGroup < numMatGroups)
                {
                    DzMaterialFaceGroup *matGroup = mesh->getMaterialGroup(indexCurrentFaceGroup);
                    QString matgroupName = matGroup->getName();
                    int numFaceVerts = -1;
                    //numFaceVerts = faceGroup->getNumVerts();
                    dzApp->log( QString("\tshape[%1]: MaterialFaceGroup(%2/%3) = [%4] has numfaceverts = [%5]").arg(shapeLabel).arg(indexCurrentFaceGroup+1).arg(numMatGroups).arg(matgroupName).arg(numFaceVerts) );
                    indexCurrentFaceGroup++;
                }

            
            }
            
            // If no facegroups, then just run material list on this shape
            //LuxMakeTextureList(currentShape);
            mesg = LuxMakeMaterialList(currentShape);
            outLXS.write(mesg);
*/

        } else {
            dzApp->log("\tno object found.");
        }
        
        //mesg = "Properties for node = " + label + "\n";
        //LuxProcessProperties(currentNode, mesg);
        //dzApp->log(mesg);
        
        // Process child nodes
//        dzApp->log("**Processing child nodes for " + label + QString(" (%1 children total)").arg(currentNode->getNumNodeChildren()) + "***\n" );
//        mesg = processChildNodes(currentNode, mesg, label);
//        dzApp->log(mesg);
        
/*
        // Iterate through properties of Node
        DzPropertyListIterator propList = currentNode->propertyListIterator();
        DzProperty *currentProperty;
        QString propertyLabel;
        while (propList.hasNext())
        {
            currentProperty = propList.next();
            propertyLabel = currentProperty->getLabel();
            dzApp->log( QString("\tproperty: %1").arg(propertyLabel) );
        }

        // 1. get property
        // 2. print name of property
        // 3. next property
*/
        
        //dzApp->log("yaluxplug: Calling Node->render() # " + label);
        //currentNode->render(*YaLuxGlobal.settings);
        outLXS.flush();
    }

    outLXS.write("\nWorldEnd\n");
    
    // 3. get nodelist
    // 4. iterate through nodes:
        // 4.1 export geometry
        // 4.2 build lights
           // sun & sky
           // spotlights
           // area lights
        // 4.3 lookup/export/cache textures
           // diffuse
           // specular
           // alpha
           // bump/displacement
    // 5. render frame


    outLXS.close();
    emit aboutToRender(this);
    handler->beginRender();
//    handler->beginFrame(0);

    // Spawn luxconsole!!!
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL( finished(int, QProcess::status) ),
            this, SLOT( handleRenderProcessComplete(int, QProcess::status) ) );

    QString file = QString("/Applications/LuxRender1.3.1/LuxRender.app/Contents/MacOS/luxrender");
//    QStringList args = QStringList() << "-l" << "-V" << "-t 4" << QString("\"%1\"").arg(fileNameLXS);
    QString cmdline = fileNameLXS;
    QStringList args = QStringList() << "-l" << "-V" << "-t4" << fileNameLXS;
//    QStringList args = QStringList() << QString("\"%1\"").arg(fileNameLXS);
    process->start(file, args);
    dzApp->log( QString("yaluxplug: SPAWNING: %1 %2").arg(file).arg(args.join(" ")) );

    YaLuxGlobal.RenderProgress->finish();
//    handler->finishFrame();
    handler->finishRender();
    YaLuxGlobal.inProgress = false;
    dzApp->log("yaluxplug: Render() DONE");
    emit renderFinished(this);
        
    return true;
}

void YaLuxRender::handleRenderProcessComplete( )
{
    dzApp->log( QString("yaluxplug: RENDER PROCESS exited ") );
    YaLuxGlobal.inProgress = false;
    emit renderFinished(this);
}

void YaLuxRender::handleRenderProcessComplete( int exitCode, QProcess::ExitStatus status )
{
    dzApp->log( QString("yaluxplug: RENDER PROCESS exited with %1 ").arg(exitCode) );
    YaLuxGlobal.inProgress = false;
    emit renderFinished(this);
}

bool YaLuxRender::customRender(DzRenderHandler *handler, DzCamera *camera, DzLightList &lights, DzNodeList &nodes, const DzRenderOptions &opt)
{
    dzApp->log("yaluxplug: unimplemented call customRender()");
    
    return true;
}

DzOptionsFrame* YaLuxRender::getOptionsFrame() const
{
    YaLuxGlobal.optFrame = new YaLuxRenderOptions();
    
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
    dzApp->log("yaluxplug: unimplemented call isRendering()");
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


/*
QString YaLuxRender::getShaderExtension()
{
    QString sResult = NULL;
    
    return sResult;
    
}

QStringList YaLuxRender::getShaderSearchPaths()
{
    QStringList oResult;

    return oResult;
}



///////////////////////////////////////////////////////////////////////
// signals
///////////////////////////////////////////////////////////////////////
void YaLuxRender::aboutToRender(DzRenderer *r)
{
        return;
}

void YaLuxRender::imagePrepared(const DzTexture *img, const QString &filename)
{
    return;
}

void YaLuxRender::renderFinished(DzRenderer *r)
{
    return;
}
*/


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