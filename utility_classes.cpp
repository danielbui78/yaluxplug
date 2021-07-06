//
//  utility_classes.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 5/5/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#include "dzapp.h"
#include "dzfileio.h"
#include "dztexture.h"
#include "dzproperty.h"
#include "dzobject.h"
#include "dzshape.h"
#include "dzgeometry.h"
#include "dzmaterial.h"
#include "dzdefaultmaterial.h"
#include "dztarray.h"
#include "dzimagemgr.h"

#include "dzpropertygroup.h"

#include "dzstringproperty.h"
#include "dzcolorproperty.h"
#include "dzimageproperty.h"
#include "dzfloatproperty.h"
#include "dzintproperty.h"
#include "dznodeproperty.h"
#include "dznumericproperty.h"
#include "dzboolproperty.h"
#include "dzenumproperty.h"

#include "dzvertexmesh.h"
#include "dzfacetmesh.h"
#include "dzfacegroup.h"
#include "dzmatrix4.h"

#include "dzlight.h"
#include "dzdistantlight.h"
#include "dzspotlight.h"
#include "dzpointlight.h"

#include "dzscene.h"
#include "dzimagemgr.h"
#include "dzfigure.h"

#include "utility_classes.h"
#include "dazToPLY.h"
#include "plugin.h"

#include "luxcore/DzMaterialToLuxCoreMaterial.h"

struct G YaLuxGlobal;

bool operator==(const DzMaterialToLuxCoreMaterial& a, const DzMaterialToLuxCoreMaterial& b)
{
    bool result = false;

    result = (
        (a.m_DiffuseMap == b.m_DiffuseMap) &&
        (a.m_DiffuseColor == b.m_DiffuseColor) &&
        (a.m_SpecularMap == b.m_SpecularMap) &&
        (a.m_SpecularColor == b.m_SpecularColor) &&
        (a.m_BumpMap == b.m_BumpMap) &&
        (a.m_BumpStrength == b.m_BumpStrength) &&
        (a.m_Roughness == b.m_Roughness) &&
        (a.m_OpacityMap == b.m_OpacityMap) &&
        (a.m_OpacityValue == b.m_OpacityValue)
        );

    return result;
}

bool operator==(const VolumeData& a, const VolumeData& b)
{
    return (
        (a.type == b.type) &&
        (a.transmission_color == b.transmission_color) &&
        (a.transmission_distance == b.transmission_distance) &&
        (a.scattering_color == b.scattering_color) &&
        (a.scattering_distance == b.scattering_distance) &&
        (a.multiscattering == b.multiscattering)
        );
}

void Worker_UpdateInfoWindow::processCoreRenderLog()
{
    if (YaLuxGlobal.luxRenderProc == NULL) return;
    if (YaLuxGlobal.logWindow == NULL) return;
    if (YaLuxGlobal.logText == NULL) return;

    QProcess* process = YaLuxGlobal.luxRenderProc;

    // Read the remainder of the stdoutput to the logfile
    if (YaLuxGlobal.bShowLuxRenderWindow)
    {
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->setReadChannel(QProcess::StandardOutput);
    }
    while (process->canReadLine())
    {

        // NOTE: we don't need to process the "writing tonemapped PNG" because a
        //   final loading of the file was done when the process called the finish() signal.
        QByteArray qa = process->readLine();

        if (qa.contains("Outputting film:"))
        {
//            if (bUpdateRender)
//                updateData();
            emit updateData();
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
        else if (qa.contains("% T)") || qa.contains("% Thld)") || qa.contains("Elapsed time"))
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
                YaLuxGlobal.FrameProgress->update(percentString.toInt());
            }

        }
        //else if (qa.contains("INFO") && YaLuxGlobal.debugLevel >= 2)
        else if (YaLuxGlobal.debugLevel >= 2)
        {
            emit updateLogWindow(QString(qa.data()), QColor(255, 255, 255));
        }

    }

}

void Worker_UpdateInfoWindow::doUpdate()
{
    while (true)
    {
        // wait state first, then process everything else
        int timeout2 = 50;
#ifdef Q_OS_WIN
        Sleep(uint(timeout2));
#else
        struct timespec ts = { timeout2 / 1000, (timeout2 % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
#endif

        if (YaLuxGlobal.inProgress == false) continue;
        if (YaLuxGlobal.RenderProgress == NULL) continue;
        if (YaLuxGlobal.handler == NULL) continue;
        if (YaLuxGlobal.luxRenderProc == NULL) continue;
        if (YaLuxGlobal.luxRenderProc->state() != QProcess::Running) continue;
        if (YaLuxGlobal.logWindow == NULL) continue;
        if (YaLuxGlobal.logWindow->isEnabled() == false) continue;

        int processState = -1;
        processState = YaLuxGlobal.luxRenderProc->state();
        if ((processState == QProcess::NotRunning) || (YaLuxGlobal.RenderProgress->isCancelled() == true))
            continue;

        processCoreRenderLog();

    }

}

void WorkerPrepareImage::doPrepareImage()
{
    // DO NOT CALL DzImageMgr in this thread ( DzImageMgr is not threadsafe!). Instead, use QImage calls.
    // 1. do a lookup to see if there is a cached texture
    // 2. if true, then return the img and filename of the cached texture
    // 3. if not, then continue
    // 4. request a new img texture
    // 5. convert filename into the new texture
    // 6. cache the new texture and put it in the lookup table
    //      6.1 generate filename
    //      6.2 place in lookup table
    // 7. emit signal and return

    int ResizeWidth;
    QImage qimg, qImgScaled;

    if (YaLuxGlobal.maxTextureSize == -1)
    {
        // ERROR: this code should not have been called if maxtexturesize is -1
        dzApp->log("yaluxplug: ERROR: workerPrepareImage() thread instantiated but maxTextureSize = no maximum. This is probably a bug in yaluxplug.");
        // return the original filename and exit
        emit prepareImageComplete(this, img, filename);
        emit finished();
        return;
    }

    ResizeWidth = YaLuxGlobal.maxTextureSize;

    // DEBUG
    if (qimg.load(filename) == false)
    {
        // DEBUG
        // issue error and cancel
        dzApp->log("yaluxplug: ERROR: Workerthread prepareImage() couldn't load original image for scaling: " + filename);
        emit prepareImageComplete(this, img, filename);
        emit finished();
        return;
    }
    
    // Do a sanity check, then resize
    if (qimg.width() > ResizeWidth)
    {
        qImgScaled = qimg.scaledToWidth(ResizeWidth);
    }
    else
    {
        // Failed sanity check.  This thread should not have been called.  Log the error and return.
        dzApp->log( QString("yaluxplug: ERROR: workerPrepareImage thread instantiated but image width (%1) is smaller than maxTexturesize(%2). This is probably a bug in yaluxplug.").arg(qimg.width()).arg(ResizeWidth) );
        emit prepareImageComplete(this, img, filename);
        emit finished();
        return;
    }

    // make cached name
    QString newName = MakeTempImgFilename(filename) + ".png";

    DzFileIO::pathExists(YaLuxGlobal.cachePath, true);
    if ( qImgScaled.save( YaLuxGlobal.cachePath+newName, "PNG") )
    {
        dzApp->log("yaluxplug: Worker prepareImage( " + filename + " ) changed to PNG" );
        //        DzTexture *tex = imgMgr->getImage(filename);
        //        img->setTempFilename(YaLuxGlobal.cachePath + newName);
        emit prepareImageComplete(this, img, YaLuxGlobal.cachePath + newName);
    }
    else 
    {
        dzApp->log("yaluxplug: ERROR: Workerthread prepareImage() couldn't save scaled image: " + YaLuxGlobal.cachePath + newName);
        emit prepareImageComplete(this, img, filename);
    }

    emit finished();
};


int whichClass(QObject *obj, const QStringList &classNames)
{
    int retval = -1;
    
    int i=0;
    while (i < classNames.count() )   
    {
        if ( obj->inherits(classNames[i].toAscii()) )
        {
            retval = (i+1);
            return retval;
        }
        i++;
    }
    
    return retval;
}




QString LuxUnsortedMaps(DzMaterial *pMaterial, QString matLabel)
{
    QString outstr;
    QString mesg;
    QObjectList texList;
    DzTexture *currentTex;
    // unsorted maps...
    texList = pMaterial->getAllMaps();
    int j = 0;
    QString imgFilename;
    while (j < texList.count() )
    {
        currentTex = (DzTexture*) texList[j];
        mesg += QString("\t\ttexList[%1] \"%2\" = [\"%3\"]").arg(j).arg(matLabel).arg(currentTex->getTempFilename() );
        outstr += QString("Texture \"%1.%2\" \"color\" \"imagemap\"\n").arg(matLabel).arg(j);
        
        outstr += "\t\"float uscale\" [1]\n\t\"float vscale\" [-1]\n";
        outstr += "\"string wrap\" [\"repeat\"]";
        
        // get image filename then convert to cached filename
        imgFilename = currentTex->getTempFilename();
        outstr += QString("  \"string filename\" [\"%1\"]\n\n").arg(imgFilename);
        
        if (j++ < texList.count() )
        {
            mesg += ", ";
        } else {
            outstr += "\n";
        }
    }
    // dzApp->log(mesg);
    return outstr;
}



QString propertyValuetoString(DzProperty *prop)
{
    DzTexture *propTex;
    QColor colorval;
    QString ret_str;
    int nClassType = whichClass(prop,classNamesProperties);
    switch (nClassType)
    {
        case 1: // DzStringProperty
            ret_str += QString("%1").arg(((DzStringProperty*)prop)->getValue() );                
            break;
        case 2: // DzColorProperty
            colorval = ((DzColorProperty*)prop)->getColorValue();
            ret_str += QString("R%1 G%2 B%3").arg(colorval.red() ).arg(colorval.green() ).arg(colorval.blue() );
            break;
        case 3: // DzFloatProperty
            ret_str += QString("%1").arg( ((DzFloatProperty*)prop)->getValue() );
            break;
        case 4: // DzIntProperty
            ret_str += QString("%1").arg( ((DzIntProperty*)prop)->getValue() );
            break;
        case 5: // DzNodeProperty
            ret_str += "(node)";
            break;
        case 6: // DzImageProperty
            propTex = ((DzImageProperty*)prop)->getValue();
            if (propTex != NULL)
                ret_str += QString("%1").arg (propTex->getTempFilename() );
            else
                ret_str += "";
            break;
        default:
            ret_str += "(no class) unimplemented\n";
            break;
            // none of the above
    }
    return ret_str;
}

// make a scaled temp image if needed, return a filename to use in luxrender
QString makeScaledTempImage(DzTexture *texture)
{
    DzImageMgr *imgMgr = dzApp->getImageMgr();
    QString origFilename, newFilename;
    QSize size;
    QImage originalImg, scaledImg;

    if (texture->getImageData(originalImg) == true)
    {
        scaledImg = originalImg.scaledToWidth(YaLuxGlobal.maxTextureSize);
        origFilename = texture->getFilename();
        newFilename = YaLuxGlobal.cachePath + MakeTempImgFilename(origFilename) + ".png";

        DzFileIO::pathExists(YaLuxGlobal.cachePath, true);
        scaledImg.save(newFilename);
        texture->setTempFilename(newFilename);
    } else {
        return origFilename;
    }

    return newFilename;
}


QString propertyNumericImagetoString(DzNumericProperty *prop)
{
    DzTexture *propTex;
    QString ret_str;
    QString tempFilename, newFilename;
    QSize size;
    QImage cachedImg;

    if ( ((DzNumericProperty*)prop)->isMappable() )
    {
        // get image name
        propTex = prop->getMapValue();
        if (propTex != NULL)
        {
            // if maxTextureSize == -1 (no maximum), then return the original filename
            if ( YaLuxGlobal.maxTextureSize == -1)
                return propTex->getFilename();
            // *** I will assume all instructions past this point are only reachable when maxTextureSize != -1 ***

            tempFilename = propTex->getTempFilename();
            if (tempFilename.contains(YaLuxGlobal.cachePath)==false)
            {
                // the original file has not been cached yet
                // check if the original image needs scaling
                size = propTex->getOriginalImageSize();
                if ( (size.width() > YaLuxGlobal.maxTextureSize) )
                    // yes, we do need to prepare image
                    ret_str = makeScaledTempImage(propTex);
                else
                    ret_str = propTex->getFilename();
            }
            else // the original file has already been cached
            {
                // If the cached tempFile size is not equal to the max_texturesize, check if the original
                // should be rescaled.
                ret_str = tempFilename; // prepare to return tempFilename... this filename should actually not change, even if we rescale below
                cachedImg.load(tempFilename);
                if (cachedImg.width() != YaLuxGlobal.maxTextureSize)
                {
                    // if the original is larger than maxTextureSize, rescale the image to maxTextureSize
                    // otherwise, just use the current tempfilename
                    if (propTex->getOriginalImageSize().width() > YaLuxGlobal.maxTextureSize)
                        ret_str = makeScaledTempImage(propTex);
                }
            }
        }
        else // there is no image file associated with this property
            ret_str= "";
    }
    
    return ret_str;
}












QString LuxGetImageMapProperty(DzElement* el, QString propertyName, QString& mesg)
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property

    DzProperty* currentProperty;
    QString propertyLabel;
    QString outstr = "";
    QColor colorval;
    int nClassType;
    QString extraMapFilename;
    DzTexture* propTex;
    int nBoolVal;

    // store up all the values then write out at once
    // ...........
    currentProperty = el->findProperty(propertyName);
    if (currentProperty != NULL)
    {
        // handle number properties with images
        if (currentProperty->inherits("DzNumericProperty"))
        {
            if (((DzNumericProperty*)currentProperty)->isMappable())
            {
                // get image name
                DzTexture *propTex = ((DzNumericProperty*)currentProperty)->getMapValue();
                if (propTex != NULL)
                    outstr = propTex->getTempFilename();
            }
        }
    }

    return outstr;
}

QString LuxGetStringProperty(DzElement *el, QString propertyName, QString &mesg)
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property

    DzProperty *currentProperty;
    QString propertyLabel;
    QString outstr = "";
    QColor colorval;
    int nClassType;
    QString extraMapFilename;
    DzTexture *propTex;
    int nBoolVal;

    // store up all the values then write out at once
    // ...........
    currentProperty = el->findProperty(propertyName);
    if (currentProperty != NULL)
    {
        propertyLabel = currentProperty->getLabel();

        mesg += QString("\tproperty [%1] = ").arg(propertyLabel);
        // find out what property type and get value
        nClassType = whichClass(currentProperty,classNamesProperties);
        switch (nClassType)
        {
            case 7: // DzImageProperty
                propTex = ((DzImageProperty*)currentProperty)->getValue();
                if (propTex != NULL)
                {
                    outstr = "";
                    mesg += QString("(image) tempname = [\"%1\"]\n").arg (propTex->getTempFilename() );
                }
                else
                {
                    outstr = "";
                    mesg += "(image) = NULL\n";
                }
                break;
            case 1: // DzStringProperty
                outstr = ((DzStringProperty*)currentProperty)->getValue();
                mesg += QString("string [\"%1\"]\n").arg(outstr);
                break;
            case 2: // DzBoolProperty
                nBoolVal = ((DzBoolProperty*)currentProperty)->getValue(dzScene->getTime());
                mesg += QString("bool [%1]\n").arg(nBoolVal);
                if ( nBoolVal == 1 )
                    outstr = "true";
                else
                    outstr = "false";
                break;
            case 3: // DzColorProperty
                colorval = ((DzColorProperty*)currentProperty)->getColorValue();
                outstr = QString("%1 %2 %3").arg(colorval.red() ).arg(colorval.green() ).arg(colorval.blue() );
                mesg += QString("color [R%1 G%2 B%3]\n").arg(colorval.red() ).arg(colorval.green() ).arg(colorval.blue() );
                break;
            case 4: // DzFloatProperty
                outstr = QString("%1").arg( ((DzFloatProperty*)currentProperty)->getValue() );
                mesg += QString("float [%1]\n").arg(outstr);
                break;
            case 5: // DzIntProperty
                outstr = QString("%1").arg( ((DzIntProperty*)currentProperty)->getValue() );
                mesg += QString("int [%1]\n").arg( outstr);
                break;
            case 6: // DzNodeProperty
                outstr = "";
                mesg += "(node) unimplemented\n";
                break;

            default:
                outstr = "";
                mesg += "unimplemented property type\n";
                break;
                // none of the above
        }

    }

    return outstr;
}



bool LuxGetFloatProperty(DzElement* el, QString propertyName, float& prop_val, QString& mesg)
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property
    bool retval = false;

    DzProperty* currentProperty;
    QString propertyLabel;
    QString outstr = "";
    QColor colorval;
    int nClassType;
    QString extraMapFilename;
    DzTexture* propTex;

    // store up all the values then write out at once
    // ...........
    currentProperty = el->findProperty(propertyName);
    if (currentProperty != NULL)
    {
        propertyLabel = currentProperty->getLabel();

        mesg += QString("\tproperty [%1] = ").arg(propertyLabel);
        // find out what property type and get value
        nClassType = whichClass(currentProperty, classNamesProperties);
        switch (nClassType)
        {
        case 4: // DzFloatProperty
            prop_val = ((DzFloatProperty*)currentProperty)->getValue();
            retval = true;
            break;
        case 5: // DzIntProperty
            prop_val = ((DzIntProperty*)currentProperty)->getValue();
            retval = true;
            break;
        case 1: // DzStringProperty
        case 2: // DzBoolProperty
        case 3: // DzColorProperty
        case 6: // DzNodeProperty
        case 7: // DzImageProperty
        default:;
            // none of the above
        }

    }

    return retval;
}



bool LuxGetIntProperty(DzElement *el, QString propertyName, int &prop_val, QString &mesg)
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property
    bool retval=false;

    DzProperty *currentProperty;
    QString propertyLabel;
    QString outstr = "";
    QColor colorval;
    int nClassType;
    QString extraMapFilename;
    DzTexture *propTex;

    // store up all the values then write out at once
    // ...........
    currentProperty = el->findProperty(propertyName);
    if (currentProperty != NULL)
    {
        propertyLabel = currentProperty->getLabel();

        mesg += QString("\tproperty [%1] = ").arg(propertyLabel);
        // find out what property type and get value
        nClassType = whichClass(currentProperty,classNamesProperties);
        switch (nClassType)
        {
            case 5: // DzIntProperty
                prop_val = ((DzIntProperty*)currentProperty)->getValue();
                retval=true;
                break;
            case 1: // DzStringProperty
            case 2: // DzBoolProperty
            case 3: // DzColorProperty
            case 4: // DzFloatProperty
            case 6: // DzNodeProperty
            case 7: // DzImageProperty
            default:;
                // none of the above
        }

    }
    
    return retval;
}














QString processChildNodes(DzNode *parentNode, QString &mesg, QString parentLabel)
{
    DzNode *currentNode;
    QString label;
    QString ret_str = "";
    
    DzNodeListIterator nodeList = parentNode->nodeChildrenIterator();
//
//    int childNumber=0;
//    while (nodeList.hasNext() )
//    {
//        currentNode = nodeList.next();
//        label = currentNode->getAssetId();
//        
//        //Process Node
//        //YaLuxGlobal.currentNode = currentNode;
//        //YaLuxGlobal.settings = new DzRenderSettings(this, &opt);
//        mesg += QString("yaluxplug: DEBUG: processChildNode(#%1) - Looking at %2->%3:\n").arg(childNumber++).arg(parentLabel).arg(label);
//        
//        // Node -> Object
//        DzObject *currentObject = currentNode->getObject();
//        if (currentObject != NULL)
//        {
//            //outLXS.write("\n# " + label +"\n");
////            mesg += parentLabel + "." + label +"\n";
//            QString objectLabel = currentObject->getLabel();
//            
//            // Object -> Shape
//            DzShape *currentShape = currentObject->getCurrentShape();
//            QString shapeLabel = currentShape->getName();
//            
//            // Shape -> Geometry
//            DzGeometry *currentGeometry = currentShape->getGeometry();
//            QString geoLabel = QString("numvertices = %1").arg(currentGeometry->getNumVertices() );
//            
//            mesg += QString("\tobject = [%1], shape = [%2], %3\n").arg(parentLabel+"."+label+"."+objectLabel).arg(shapeLabel).arg(geoLabel) ;
//
//            //LuxMakeTextureList(currentShape);
//            //mesg += LuxMakeMaterialList(currentShape);
//            //outLXS.write(mesg);
//
//            // Call process object for this childnode
//            // currentObject->finalize(*parentNode, true, true);
//            // ret_str = LuxProcessObject(currentObject, mesg);
//
//        } else {
//            mesg += "no object found.\n";
//        }
//        
//        mesg += QString("\tProperties for childnode = [%1.%2]\n").arg(parentLabel).arg(label);
//        LuxProcessProperties(currentNode, mesg);
////        if (YaLuxGlobal.debugLevel >=2) // debugging data
////            dzApp->log(mesg);
//        // don't write to dzapp->log, the calling program can do that.
//
//    }
    
    return ret_str;
}


//

//


















QString GenerateCoreTextureBlock3(QString textureName, QString mapName, float textureValue1, float textureValue2, float textureValue3,
    float uscale, float vscale, float uoffset, float voffset, float gamma,
    QString wrap, QString channel)
{
    QString ret_str;

    if (mapName != "")
    {

        QString realtextureName = textureName;
        QString scaletextureName;
        bool bMixTextures = false;
        if (textureValue1 != 1.0 || textureValue2 != 1.0 || textureValue3 != 1.0)
        {
            // set up mix texture
            bMixTextures = true;
            realtextureName = textureName + "_raw";
            //scaletextureName = textureName + "_1";
            //ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(scaletextureName);
            //ret_str += QString("scene.textures.%1.value = %2 %3 %4\n").arg(scaletextureName).arg(textureValue1).arg(textureValue2).arg(textureValue3);
        }
        ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(realtextureName);
        ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(realtextureName).arg(mapName);
        ret_str += QString("scene.textures.%1.mapping.type = \"uvmapping2d\"\n").arg(realtextureName);

        if (uscale != 1 || vscale != 1)
            ret_str += QString("scene.textures.%1.mapping.uvscale = %2 %3\n").arg(realtextureName).arg(uscale).arg(vscale);
        if (uoffset != 0 || voffset != 0)
            ret_str += QString("scene.textures.%1.mapping.uvdelta = %2 %3\n").arg(realtextureName).arg(uoffset).arg(voffset);
        if (gamma != 2.2)
            ret_str += QString("scene.textures.%1.gamma = %2\n").arg(realtextureName).arg(gamma);
        if (wrap != "")
            ret_str += QString("scene.textures.%1.wrap = \"%2\"\n").arg(realtextureName).arg(wrap);
        if (channel != "")
            ret_str += QString("scene.textures.%1.channel = \"%2\"\n").arg(realtextureName).arg(channel);

        if (bMixTextures)
        {
            //ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(textureName);
            //ret_str += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(textureName);
            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(textureName).arg(realtextureName);
            //ret_str += QString("scene.textures.%1.amount = \"%2\"\n").arg(textureName).arg(scaletextureName);
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(textureName);
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(textureName).arg(realtextureName);
            ret_str += QString("scene.textures.%1.texture2 = %2 %3 %4\n").arg(textureName).arg(textureValue1).arg(textureValue2).arg(textureValue3);
        }
    }
    else {
        ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(textureName);
        ret_str += QString("scene.textures.%1.value = %2 %3 %4\n").arg(textureName).arg(textureValue1).arg(textureValue2).arg(textureValue3);
    }

    return ret_str;

}

QString GenerateCoreTextureBlock1(QString textureName, QString mapName, float textureValue,
    float uscale, float vscale, float uoffset, float voffset, float gamma,
    QString wrap, QString channel)
{
    QString ret_str;

    if (mapName != "")
    {
        QString realtextureName = textureName;
        bool bMixTextures = false;
        //if (textureValue != 1.0)
        //{
        //    // set up mix texture
        //    bMixTextures = true;
        //    realtextureName = textureName + "_0";
        //}
        ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(realtextureName);
        ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(realtextureName).arg(mapName);
        ret_str += QString("scene.textures.%1.mapping.type = \"uvmapping2d\"\n").arg(realtextureName);
        if (uscale != 1 || vscale != 1)
            ret_str += QString("scene.textures.%1.mapping.uvscale = %2 %3\n").arg(realtextureName).arg(uscale).arg(vscale);
        if (uoffset != 0 || voffset != 0)
            ret_str += QString("scene.textures.%1.mapping.uvdelta = %2 %3\n").arg(realtextureName).arg(uoffset).arg(voffset);
        if (gamma != 2.2)
            ret_str += QString("scene.textures.%1.gamma = %2\n").arg(realtextureName).arg(gamma);
        if (wrap != "")
            ret_str += QString("scene.textures.%1.wrap = \"%2\"\n").arg(realtextureName).arg(wrap);
        if (channel != "")
            ret_str += QString("scene.textures.%1.channel = \"%2\"\n").arg(realtextureName).arg(channel);
        if (textureValue != 1.0)
            ret_str += QString("scene.textures.%1.gain = \"%2\"\n").arg(realtextureName).arg(textureValue);

        //if (bMixTextures)
        //{
        //    ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(textureName);
        //    ret_str += QString("scene.textures.%1.texture1 = 0\n").arg(textureName);
        //    ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(textureName).arg(realtextureName);
        //    ret_str += QString("scene.textures.%1.amount = %2\n").arg(textureName).arg(textureValue);
        //}
    }
    else {
        //ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName).arg(textureType).arg("constant");
        //ret_str += QString("\t\"%1 value\" [%2]\n").arg(textureType).arg(textureValue);
        ret_str += QString("scene.textures.%1.type = \"constfloat1\"\n").arg(textureName);
        ret_str += QString("scene.textures.%1.value = %2\n").arg(textureName).arg(textureValue);
    }

    return ret_str;

}

QString GenerateCoreTextureBlock1_Grey(QString textureName, QString mapName, float textureValue,
    float uscale, float vscale, float uoffset, float voffset, float gamma,
    QString wrap, QString channel)
{
    QString ret_str;

    if (mapName != "")
    {
        QString realtextureName = textureName;
        bool bMixTextures = false;
        if (textureValue != 1.0)
        {
            // set up mix texture
            bMixTextures = true;
            realtextureName = textureName + "_0";
        }
        ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(realtextureName);
        ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(realtextureName).arg(mapName);
        ret_str += QString("scene.textures.%1.mapping.type = \"uvmapping2d\"\n").arg(realtextureName);
        if (uscale != 1 || vscale != 1)
            ret_str += QString("scene.textures.%1.mapping.uvscale = %2 %3\n").arg(realtextureName).arg(uscale).arg(vscale);
        if (uoffset != 0 || voffset != 0)
            ret_str += QString("scene.textures.%1.mapping.uvdelta = %2 %3\n").arg(realtextureName).arg(uoffset).arg(voffset);
        if (gamma != 2.2)
            ret_str += QString("scene.textures.%1.gamma = %2\n").arg(realtextureName).arg(gamma);
        if (wrap != "")
            ret_str += QString("scene.textures.%1.wrap = \"%2\"\n").arg(realtextureName).arg(wrap);
        if (channel != "")
            ret_str += QString("scene.textures.%1.channel = \"%2\"\n").arg(realtextureName).arg(channel);
        //if (textureValue != 1)
        //    ret_str += QString("scene.textures.%1.gain = \"%2\"\n").arg(realtextureName).arg(textureValue);

        if (bMixTextures)
        {
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(textureName);
            ret_str += QString("scene.textures.%1.texture1 = %2\n").arg(textureName).arg(textureValue);
            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(textureName).arg(realtextureName);
        }
    }
    else {
        //ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName).arg(textureType).arg("constant");
        //ret_str += QString("\t\"%1 value\" [%2]\n").arg(textureType).arg(textureValue);
        ret_str += QString("scene.textures.%1.type = \"constfloat1\"\n").arg(textureName);
        ret_str += QString("scene.textures.%1.value = %2\n").arg(textureName).arg(textureValue);
    }

    return ret_str;

}



QString SanitizeCoreLabel(QString label)
{
    QString retString;

    // SANITIZE LABEL: remove spaces, commas, semicolons, colons double quotes, single quotes, !@#$%^&*()
    QStringList blackListCharacters;
    blackListCharacters << " " << "," << ";" << ":" << "\"" << "'" << "!" << "@" << "#" << "$" << "%";
    blackListCharacters << "^" << "&" << "*" << "(" << ")" << "=" << "+" << "-" << ".";
    for (int i = 0; i < blackListCharacters.count(); i++)
    {
        retString = label.replace(blackListCharacters[i], "_");
    }

    return retString;
}

QString MakeTempImgFilename(QString origFilename)
{
    QString newFilename;

    QString path = DzFileIO::getFilePath(origFilename).replace(" ","_").replace("\\", "_").replace("/", "_");
    QString filename = DzFileIO::getBaseFileName(origFilename);
    
    int offset = path.indexOf("_textures_",0, Qt::CaseInsensitive);
    if (offset == -1)
    {
        offset = (path.length() <= 25) ? 0 : 25;
    }
    else
    {
        offset = (path.length() > 10) ? offset + 10 : offset;
    }
    path = path.right(path.length() - offset);
    newFilename = path + "_" + filename;

    return newFilename;
}


#include "moc_utility_classes.cpp"
