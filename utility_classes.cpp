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

struct G YaLuxGlobal;

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
    QString newName = DzFileIO::getBaseFileName(filename);
    newName += ".png";
    
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

QString LuxMakeSingleTexture(DzTexture* currentTex, QString texname, QString maptype)
{
    QString outstr;
    
    outstr = QString("Texture \"%1\" \"%2\" \"imagemap\"\n").arg(texname).arg(maptype);
    outstr += "\t\"float uscale\" [1]\n\t\"float vscale\" [-1]\n";
    outstr += "\t\"string wrap\" [\"repeat\"]\n";
    // get image filename then convert to cached filename
    outstr += QString("\t\"string filename\" [\"%1\"]\n").arg(currentTex->getTempFilename());    
    
    return outstr;
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
        newFilename = YaLuxGlobal.cachePath + DzFileIO::getBaseFileName(origFilename) + ".png";
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

QString MixTextures(QString textureMixName, QString textureName1, QString textureName2, QString textureType, QString mixString)
{
    QString ret_str;

    ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureMixName).arg(textureType).arg("mix");
    ret_str += QString("\t\"texture tex1\" [\"%1\"]\n").arg(textureName1);
    ret_str += QString("\t\"texture tex2\" [\"%1\"]\n").arg(textureName2);
    ret_str += QString("\t\%1\n").arg(mixString);

    return ret_str;
}

QString ScaleTexture(QString textureScaleName, QString textureName1, QString scaleValue, QString textureType)
{
    QString ret_str;

    ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureScaleName).arg(textureType).arg("scale");
    ret_str += QString("\t\"texture tex1\" [\"%1\"]\n").arg(textureName1);
    ret_str += QString("\t\"%1 tex2\" [%2]\n").arg(textureType).arg(scaleValue);

    return ret_str;
}


QString GenerateTextureBlock(QString textureName, QString textureType, QString mapName, QString textureValue,
                             float uscale, float vscale, float uoffset, float voffset, float gamma, 
                             QString wrap, QString filtertype, QString channel)
{
    QString ret_str;
    
    if (mapName != "")
    {
        ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName+"map").arg(textureType).arg("imagemap");
        if (uscale != 1) ret_str += QString("\t\"float uscale\" [%1]\n").arg(uscale);
        if (vscale != 1) ret_str += QString("\t\"float vscale\" [%1]\n").arg(vscale);
        if (uoffset != 0) ret_str += QString("\t\"float udelta\" [%1]\n").arg(uoffset);
        if (voffset != 0) ret_str += QString("\t\"float vdelta\" [%1]\n").arg(voffset);
        if (gamma != 1) ret_str += QString("\t\"float gamma\" [%1]\n").arg(gamma);
        if (wrap != "repeat") ret_str += QString("\t\"string wrap\" [\"%1\"]\n").arg(wrap);
        if (filtertype != "bilinear") ret_str += QString("\t\"string filtertype\" [\"%1\"]\n").arg(filtertype);
        if (channel != "")
            ret_str += QString("\t\"string channel\" [\"%1\"]\n").arg(channel);
        ret_str += QString("\t\"string filename\" [\"%1\"]\n").arg(mapName);        
        
        ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName).arg(textureType).arg("scale");
        ret_str += QString("\t\"texture tex1\" [\"%1\"]\n").arg(textureName+"map");
        ret_str += QString("\t\"%1 tex2\" [%2]\n").arg(textureType).arg(textureValue);
    } else {
        ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName).arg(textureType).arg("constant");
        ret_str += QString("\t\"%1 value\" [%2]\n").arg(textureType).arg(textureValue);        
    }
    
    return ret_str;
    
}



QString LuxProcessProperties(DzMaterial *el, QString &mesg, QString &matDef, QString matLabel="")
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property
    
    // Iterate through properties of DzMaterial
    DzPropertyListIterator propList = el->propertyListIterator();
    DzProperty *currentProperty;
    QString propertyLabel;
    QString outstr;
    QColor colorval;
    int nClassType;
    QString extraMapFilename;
    
    // store up all the values then write out at once
    // ...........
    
    while (propList.hasNext())
    {
        DzTexture *propTex;
        currentProperty = propList.next();
        propertyLabel = currentProperty->getLabel();
        
        // process property name and use it to create Lux entry (ie, Texture "...")
        if (propertyLabel.contains("Diffuse Color") )
        {
            // process the propvalue to create diffuse color and diffuse map Texture entries
            // then add an additional entry into the material definition
            if (currentProperty->inherits("DzNumericProperty") )
                extraMapFilename = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
            if (extraMapFilename != "")
            {
                // create image map entry
                outstr += QString("Texture \"%1.diffuse_map\" \"color\" \"imagemap\"\n").arg(matLabel);
                outstr += QString("\t\"string filename\" [\"%1\"]\n").arg( extraMapFilename);
                outstr += "\n";
                // add entry material definition
                matDef += QString("\t\"texture Kd\" [\"%1.diffuse_map\"]").arg(matLabel);
            }
            // create non-image texture entry
            outstr += QString("Texture \"%1.diffuse_color\" \"color\" \"scale\"\n").arg(matLabel);
            
        }
        
        mesg += QString("\tproperty [%1] = ").arg(propertyLabel);
        // find out what property type and get value
        nClassType = whichClass(currentProperty,classNamesProperties);
        switch (nClassType)
        {
            case 1: // DzStringProperty
                mesg += QString("[\"%1\"]\n").arg(((DzStringProperty*)currentProperty)->getValue() );                
                break;
            case 2: // DzColorProperty
                colorval = ((DzColorProperty*)currentProperty)->getColorValue();
                mesg += QString("[R%1 G%2 B%3]\n").arg(colorval.red() ).arg(colorval.green() ).arg(colorval.blue() );
                break;
            case 3: // DzFloatProperty
                mesg += QString("[%1]\n").arg( ((DzFloatProperty*)currentProperty)->getValue() );
                break;
            case 4: // DzIntProperty
                mesg += QString("[%1]\n").arg( ((DzIntProperty*)currentProperty)->getValue() );
                break;
            case 5: // DzNodeProperty
                mesg += "(node) unimplemented\n";
                break;
            case 6: // DzImageProperty
                propTex = ((DzImageProperty*)currentProperty)->getValue();
                if (propTex != NULL)
                    mesg += QString("(image) tempname = [\"%1\"]\n").arg (propTex->getTempFilename() );
                else
                    mesg += "(image) = NULL\n";
                break;
            default:
                mesg += "unimplemented property type\n";
                break;
                // none of the above
        }
        // handle number properties with images
        if (currentProperty->inherits("DzNumericProperty"))
        {   
            if ( ((DzNumericProperty*)currentProperty)->isMappable() )
            {
                mesg += "\t\t++";
                // get image name
                propTex = ((DzNumericProperty*)currentProperty)->getMapValue();
                if (propTex != NULL)
                    mesg += QString("(image) tempname = [\"%1\"]\n").arg (propTex->getTempFilename() );
                else
                    mesg += "(image) = NULL\n";
            }
        } 
    }
    
    return outstr;
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



QString LuxProcessLight(DzLight *currentLight, QString &mesg)
{
    QString outstr;
    QString lightLabel, lightAssetId;
    DzVec3 lightVector;
    DzVec3 lightPos;

    if (currentLight->isVisible() == false)
        return "";

    lightLabel = currentLight->getLabel();
    lightAssetId = currentLight->getAssetId();
    lightVector = currentLight->getWSDirection();

    // Check to see if luxrender settings are present
    LuxProcessProperties( (DzElement*) currentLight, mesg);
    int lux_light_type;
    if (LuxGetIntProperty((DzElement*) currentLight, "LuxRender_light_type", lux_light_type, mesg) == true)
    {
        outstr = "\nAttributeBegin\n";
        switch (lux_light_type)
        {
            case 6: // sky2
                outstr += "LightSource \"sky2\"\n";
                outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
                outstr += QString("\t\"float gain\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));
                outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
                outstr += QString("\t\"float turbidity\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
                break;
            case 8: // Sun
                outstr += "LightSource \"sun\"\n";
                outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
                outstr += QString("\t\"float gain\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));
                outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
                outstr += QString("\t\"float turbidity\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
                outstr += QString("\t\"float relsize\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
                break;
            case 9: // Sun & sky2
                outstr += "LightSource \"sun\"\n";
                outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
                outstr += QString("\t\"float gain\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));
                outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
                outstr += QString("\t\"float turbidity\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
                outstr += QString("\t\"float relsize\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
                outstr += "LightSource \"sky2\"\n";
                outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
                outstr += QString("\t\"float gain\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));
                outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
                outstr += QString("\t\"float turbidity\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
                break;
            default:
                // not implemented, reset the string and break out to continue the normal pathway
                outstr = "";
                break;
                //add area light source reference line
        }
        // If the Lux_light_type was implemented, then this string has not been reset. Finish up and return str
        if (outstr != "" )
        {
            outstr += QString("\t\"float importance\"\t[%1]\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_importance", mesg));
            outstr += QString("%1\n").arg( LuxGetStringProperty(currentLight, "LuxRender_light_extrasettings", mesg));
            outstr += "\nAttributeEnd\n";
            YaLuxGlobal.bDefaultLightsOn = false;
            // don't need to continue
            return outstr;
        }
    }


    // If not present, then handle as a generic DazLight
    outstr = "\nAttributeBegin\n";
    outstr += QString("LightGroup\t\"%1\" # %2\n").arg(currentLight->getAssetId()).arg(lightLabel);
    // if IBL, create an infinite light
    if ( currentLight->getAssetId().contains("Image Based Light") )
    {
        QColor lightColor = currentLight->getDiffuseColor();
        outstr += "LightSource \"infinite\"\n";
        outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        QString mapname = propertyNumericImagetoString( (DzNumericProperty*) currentLight->findProperty("Color") );
        if (mapname != "")
            outstr += QString("\t\"string mapname\"\t[\"%1\"]\n").arg(mapname);
        outstr += "\nAttributeEnd\n";
        YaLuxGlobal.bDefaultLightsOn = false;

        return outstr;
    }
    if (lightLabel.contains("Sun"))
    {
        // change light type to sun and sky
        outstr += "LightSource \"sun\"\n";
        outstr += QString("\t\"float importance\"\t[%1]\n").arg(1);
        outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity() );

        YaLuxGlobal.bDefaultLightsOn = false;

    }
    if (lightLabel.contains("Sky"))
    {
        outstr += "LightSource \"sky2\"\n";
        outstr += QString("\t\"float importance\"\t[%1]\n").arg(1);
        outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity() );
        YaLuxGlobal.bDefaultLightsOn = false;
    }
    if (lightLabel.contains("Infinite"))
    {
        outstr += "LightSource \"infinite\"\n";
        QColor lightColor = currentLight->getDiffuseColor();
        outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        // TODO: implement HDRI map
//        QString mapname = propertyNumericImagetoString(...);
//        outstr += QString("\t\"string mapname\"\t[\"%1\"]\n").arg(mapname);
        outstr += QString("\t\"float gain\"\t[%1]\n").arg( ((DzDistantLight*)currentLight)->getIntensity() );
        YaLuxGlobal.bDefaultLightsOn = false;
    }
    if ( !(lightLabel.contains("Sun")) && !(lightLabel.contains("Sky")) && !(lightLabel.contains("Infinite")) )
    {
        YaLuxGlobal.bDefaultLightsOn = false;
        // convert everything else to a mesh light
        DzMatrix3 mat3;
        DzMatrix4 mat4;
        mat3 = currentLight->getWSTransform(dzScene->getTime());
        mat4 = mat3.matrix4();
        outstr += "Transform [";
        outstr += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
        outstr += QString("%1 %2 %3 %4 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);
        outstr += QString("%1 %2 %3 %4 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
        // row[3] needs to be scaled
        outstr += QString("%1 %2 %3 %4 ").arg(mat4[3][0]/100).arg(-mat4[3][2]/100).arg(mat4[3][1]/100).arg(mat4[3][3]);
        outstr += "]\n";

        outstr += "AreaLightSource \"area\"\n";
        QColor lightColor = currentLight->getDiffuseColor();
        outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        outstr += QString("\t\"float power\"\t[%1]\n").arg(100);
        outstr += QString("\t\"float efficacy\"\t[%1]\n").arg(17);

        if (currentLight->inherits("DzSpotLight"))
        {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg( ((DzSpotLight*)currentLight)->getIntensity() );
            outstr += spotLightPlane.join("");

        } else if (currentLight->inherits("DzPointLight")) {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg( ((DzPointLight*)currentLight)->getIntensity() );
            outstr += "Shape \"sphere\" \"float radius\" [0.5]\n";

        } else if (currentLight->inherits("DzDistantLight")) {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg( ((DzDistantLight*)currentLight)->getIntensity() );
            outstr += distantLightPlane.join("");

        }
/*
        if (currentLight->inherits("DzSpotLight"))
        {
            outstr += "LightSource \"spot\"\n";
            lightPos = currentLight->getWSPos();
//            lightPos = luxTransform.multVecMatrix(lightPos);
            lightVector = currentLight->getFocalPoint();
//            lightVector = luxTransform.multVecMatrix(target);
            outstr += QString("\t\"point from\"\t[%1 %2 %3]\n").arg(lightPos.m_x).arg(lightPos.m_y).arg(lightPos.m_z);
            outstr += QString("\t\"point to\"\t[%1 %2 %3]\n").arg(lightVector.m_x).arg(lightVector.m_y).arg(lightVector.m_z);
        } else if (currentLight->inherits("DzPointLight")) {
            outstr += "LightSource \"point\"\n";
            lightPos = currentLight->getWSPos();
//            lightPos = luxTransform.multVecMatrix(lightPos);
            outstr += QString("\t\"point from\"\t[%1 %2 %3]\n").arg(lightPos.m_x).arg(lightPos.m_y).arg(lightPos.m_z);
        } else if (currentLight->inherits("DzDistantLight")) {
            mesg = "distant";
            outstr += "LightSource \"" + mesg + "\"\n";
            lightPos = currentLight->getWSPos();
//            lightPos = luxTransform.multVecMatrix(pos);
            outstr += QString("\t\"point from\"\t[%1 %2 %3]\n").arg(lightPos.m_x).arg(lightPos.m_y).arg(lightPos.m_z);
            lightVector = currentLight->getFocalPoint();
//            lightVector = luxTransform.multVecMatrix(lightVector);
            outstr += QString("\t\"point to\"\t[%1 %2 %3]\n").arg(lightVector.m_x).arg(lightVector.m_y).arg(lightVector.m_z);
        }
*/

    }

    outstr += "AttributeEnd\n";

    return outstr;
}

QString LuxProcessProperties(DzElement *el, QString &mesg)
{
    // 1. get property
    // 2. print name of property and values
    // 3. next property

    // Iterate through properties of DzMaterial
    DzPropertyListIterator propList = el->propertyListIterator();
    DzProperty *currentProperty;
    QString propertyLabel;
    QString outstr;
    QColor colorval;
    QString propertyName;
    
    while (propList.hasNext())
    {
        DzTexture *propTex;
        currentProperty = propList.next();
        propertyLabel = currentProperty->getLabel();
        propertyName = currentProperty->getAssetId();
        mesg += QString("\tproperty [%1] (\"%2\") = ").arg(propertyName).arg(propertyLabel);
        // find out what property type and get value
        QStringList classNames = QStringList() << "DzStringProperty" << "DzColorProperty" << "DzFloatProperty" << "DzIntProperty" <<  "DzNodeProperty" << "DzImageProperty" ;
        int nClassType = whichClass(currentProperty,classNames);
        switch (nClassType)
        {
            case 0:
                mesg += "unimplemented property type\n";
                break;
            case 1: // DzStringProperty
                mesg += QString("[\"%1\"]\n").arg(((DzStringProperty*)currentProperty)->getValue() );                
                break;
            case 2: // DzColorProperty
                colorval = ((DzColorProperty*)currentProperty)->getColorValue();
                mesg += QString("[R%1 G%2 B%3]\n").arg(colorval.red() ).arg(colorval.green() ).arg(colorval.blue() );
                break;
            case 3: // DzFloatProperty
                mesg += QString("[%1]\n").arg( ((DzFloatProperty*)currentProperty)->getValue() );
                break;
            case 4: // DzIntProperty
                mesg += QString("[%1]\n").arg( ((DzIntProperty*)currentProperty)->getValue() );
                break;
            case 5: // DzNodeProperty
                mesg += "(node) unimplemented\n";
                break;
            case 6: // DzImageProperty
                propTex = ((DzImageProperty*)currentProperty)->getValue();
                if (propTex != NULL)
                    mesg += QString("(image) tempname = [\"%1\"]\n").arg (propTex->getTempFilename() );
                else
                    mesg += "(image) = NULL\n";
                break;
            default:
                mesg += "(no class) unimplemented\n";
                break;
                // none of the above
        }
        // handle number properties with images
        if (currentProperty->inherits("DzNumericProperty"))
        {   
            if ( ((DzNumericProperty*)currentProperty)->isMappable() )
            {
                mesg += "\t\t++";
                // get image name
                propTex = ((DzNumericProperty*)currentProperty)->getMapValue();
                if (propTex != NULL)
                    mesg += QString("(image) tempname = [\"%1\"]\n").arg (propTex->getTempFilename() );
                else
                    mesg += "(image) = NULL\n";
            }
        } 
    }
    
    return outstr;
}


QString LuxMakeSHAPE(DzFacetMesh *mesh, QString meshName)
{
    int numFaces;
    int numVerts;
    int numNormals;   
    int numUVpts; 
    DzPnt3 *vertexList;
    DzPnt3 *normalsList;
    DzFacet *facesList;
    DzPnt2 *uvList;
    
    QString ret_str ="";
    QString filenamePLY;
    
    numVerts = mesh->getNumVertices();
    vertexList = mesh->getVerticesPtr();
    numFaces = mesh->getNumFacets();
    facesList = mesh->getFacetsPtr();
    numNormals = mesh->getNumNormals();
    normalsList = mesh->getNormalsPtr();
    uvList = (mesh->getUVs())->getPnt2ArrayPtr();
    numUVpts = (mesh->getUVs())->getNumValues();
    
    if (numVerts == 0)
        return "";
    
    ret_str = QString("Shape \"mesh\"\n");
    int i = 0;
    QString vert_str;
    float *ptrF;
    ret_str += "\t\"point P\" [";
    while (i < numVerts)
    {
        ptrF = (float*) &vertexList[i];
        vert_str += QString("%1 %2 %3 ").arg( ptrF[0]/100).arg( -ptrF[2]/100).arg( ptrF[1]/100);
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    ret_str += "\t\"normal N\" [";
    vert_str = "";
    i = 0;
    while (i < numNormals)
    {
        ptrF = (float*) &normalsList[i];
        vert_str += QString("%1 %2 %3 ").arg( ptrF[0]).arg( -ptrF[2]).arg( ptrF[1]);
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    ret_str += "\t\"float uv\" [";
    vert_str = "";
    i = 0;
    while (i < numUVpts)
    {
        ptrF = (float*) &uvList[i];
        vert_str += QString("%1 %2 ").arg( ptrF[0]).arg( ptrF[1]);  
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    ret_str += "\t\"integer quadindices\" [";
    vert_str = "";
    i = 0;
    while (i < numFaces)
    {
        if (facesList[i].isTri())
            vert_str += QString("%1 %2 %3 ").arg(facesList[i].m_vertIdx[0]).arg(facesList[i].m_vertIdx[1]).arg(facesList[i].m_vertIdx[2]);
        else if (facesList[i].isQuad())
            vert_str += QString("%1 %2 %3 %4 ").arg(facesList[i].m_vertIdx[0]).arg(facesList[i].m_vertIdx[1]).arg(facesList[i].m_vertIdx[2]).arg(facesList[i].m_vertIdx[3]);
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    
    return ret_str;
}


QString LuxMakeTextureList(DzShape *currentShape)
{
    // Shape -> MaterialList
    DzMaterialPtrList materialList;
    QString matLabel;
    currentShape->getAllRenderPrioritizedMaterials(materialList);
    int i = 0;
    QString mesg;
    DzTexture *currentTex;
    DzDefaultMaterial *currentMat;
    QString outstr = "";
    QString materialDef = "";
    // TEXTURES 
    while (i < materialList.count() )
    {
        materialDef += " # MATERIAL\n";
        outstr += " # TEXTURES\n";
        matLabel = materialList[i]->getLabel();
        mesg += "  # MATERIAL -" + matLabel +"\n";
        
        materialDef += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        materialDef += "\t\"string type\" [\"glossy\"]\n";
        // named maps
        // diffuse
        currentTex = materialList[i]->getColorMap();
        if (currentTex != NULL)
        {
            outstr += LuxMakeSingleTexture(currentTex, matLabel+".diffuse_map", "color");
            // add respective line to Material Definition
            materialDef += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel+".diffuse_map");
        }
        if ( materialList[i]->inherits("DzDefaultMaterial") )
        {
            DzMaterial *matptr = materialList[i];
            currentMat = (DzDefaultMaterial *) matptr;
            currentTex = currentMat->getSpecularColorMap();
            if (currentTex != NULL)
            {
                outstr += LuxMakeSingleTexture(currentTex, matLabel+".specular_map", "color");            
                // add respective line to Material Definition
                materialDef += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel+".specular_map");
            }
            currentTex = currentMat->getBumpMap();
            if (currentTex != NULL)
            {
                outstr += LuxMakeSingleTexture(currentTex, matLabel+".bump_map", "float");
                // add respective line to Material Definition
                materialDef += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel+".bump_map");
            }
        }
        // Compose Material Definition with acrued definition lines
        materialDef += "\n" + outstr + "\n";
        outstr = "";
        
        LuxProcessProperties(materialList[i], mesg);
        
        //LuxUnsortedMaps(materialList[i], matLabel);
        
        i++;
        //outLXS.write(outstr);
    }
    //    dzApp->log(mesg);
    
    return materialDef;
}

QString LuxMakeMaterialList(DzShape *currentShape)
{
    // Shape -> MaterialList
    DzMaterialPtrList materialList;
    QString matLabel;
    currentShape->getAllRenderPrioritizedMaterials(materialList);
    int i = 0;
    QString mesg = "";
    QObjectList texList;

    QString outstr = "";
    QString attributeblock = "";
    // TEXTURES 
    while (i < materialList.count() )
    {
        attributeblock = "AttributeBegin\n";
        
        DzMatrix3 mat3 = (currentShape->getNode())->getWSTransform();
        DzMatrix4 mat4 = mat3.matrix4();
        attributeblock += "Transform [";
        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
        // row[1] and row[2] are switched
        attributeblock += QString("%1 %2 %3 %4 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);        
        // row[3] needs to be scaled
        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[3][0]/100).arg(-mat4[3][2]/100).arg(mat4[3][1]/100).arg(mat4[3][3]);
        attributeblock += "]\n";
        
        //        matLabel = materialList[i]->getMaterialName();
        QString shapeLabel = currentShape->getNode()->getAssetId();
        matLabel = materialList[i]->getLabel();
        attributeblock += LuxProcessGlossyMaterial(materialList[i], mesg, matLabel);
        
        // process related shapes for this material
        DzGeometry *mesh = currentShape->getGeometry();
        QString meshName = QString("%1.%2").arg(shapeLabel).arg( matLabel );
        
        QString plyFileName;
        if (mesh->inherits("DzFacetMesh"))
        {
            DazToPLY dzPLYexport((DzFacetMesh *)mesh, meshName, materialList[i]);
            plyFileName = dzPLYexport.LuxMakeAsciiPLY();
            //            plyFileName = dzPLYexport.LuxMakeBinPLY();
            //            plyFileName = LuxMakePLY((DzFacetMesh*)mesh, meshName, materialList[i]);
            //            outstr += LuxMakeSHAPE( (DzFacetMesh*)mesh, meshName);
        }
        if (plyFileName != "")
        {   
            // add in reference to plyFileName
            attributeblock += "Shape \"plymesh\"\n";
            attributeblock += QString("\t\"string filename\" [\"%1\"]\n").arg(plyFileName);
            
            attributeblock += "AttributeEnd\n\n";
        } else {
            // no shapes to render for this material, just scrap this attribute block and move on
            attributeblock = "";
        }
        outstr += attributeblock;
        i++;
    }
    //    dzApp->log(mesg);
    
    return outstr;
    
}


QString processChildNodes(DzNode *parentNode, QString &mesg, QString parentLabel)
{
    DzNode *currentNode;
    QString label;
    QString ret_str = "";
    
    DzNodeListIterator nodeList = parentNode->nodeChildrenIterator();

    int childNumber=0;
    while (nodeList.hasNext() )
    {
        currentNode = nodeList.next();
        label = currentNode->getAssetId();
        
        //Process Node
        //YaLuxGlobal.currentNode = currentNode;
        //YaLuxGlobal.settings = new DzRenderSettings(this, &opt);
        mesg += QString("yaluxplug: DEBUG: processChildNode(#%1) - Looking at %2->%3:\n").arg(childNumber++).arg(parentLabel).arg(label);
        
        // Node -> Object
        DzObject *currentObject = currentNode->getObject();
        if (currentObject != NULL)
        {
            //outLXS.write("\n# " + label +"\n");
//            mesg += parentLabel + "." + label +"\n";
            QString objectLabel = currentObject->getLabel();
            
            // Object -> Shape
            DzShape *currentShape = currentObject->getCurrentShape();
            QString shapeLabel = currentShape->getName();
            
            // Shape -> Geometry
            DzGeometry *currentGeometry = currentShape->getGeometry();
            QString geoLabel = QString("numvertices = %1").arg(currentGeometry->getNumVertices() );
            
            mesg += QString("\tobject = [%1], shape = [%2], %3\n").arg(parentLabel+"."+label+"."+objectLabel).arg(shapeLabel).arg(geoLabel) ;

            //LuxMakeTextureList(currentShape);
            //mesg += LuxMakeMaterialList(currentShape);
            //outLXS.write(mesg);

            // Call process object for this childnode
            // currentObject->finalize(*parentNode, true, true);
            // ret_str = LuxProcessObject(currentObject, mesg);

        } else {
            mesg += "no object found.\n";
        }
        
        mesg += QString("\tProperties for childnode = [%1.%2]\n").arg(parentLabel).arg(label);
        LuxProcessProperties(currentNode, mesg);
//        if (YaLuxGlobal.debugLevel >=2) // debugging data
//            dzApp->log(mesg);
        // don't write to dzapp->log, the calling program can do that.

    }
    
    return ret_str;
}

QString LuxProcessMatteMaterial(DzMaterial *material, QString &mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // process all material information and output a full material LXS description block


    // Lux Texture format
    // Texture "NAME" color|float|spectrum "TYPE" <values>
    // TYPE:
    //  imagemap
    //  mix
    //  scale (mulitply with color)
    // PARAMS
    //  mapping = [uv|spherical|cylindrical|planar]
    //  uscale/vscale = [float] scale texcoords
    //  udelta/vdelta = [float] offfset tex mappings
    //  v1/v2 = [vector] : define the plane for planar mapping
    //  constant::'value' = [color|float]
    //  scale::'tex1'/'tex2' = [color|float] (multiply 2 textures)
    //  imagemap::'filename' = [string] (with path)
    //  imagemap::'wrap' = [string] (repeat|black|clamp)
    //  imagemap::'filtertype' = [string] (bilinear|mipmap_trilinear|mipmap_ewa|nearest
    //  imagemap::'channel' = [string] (mean|red|green|blue|alpha|colored_mean)

    // Lux Material format
    //  bumpmap = [float texture]
    //  matte::Kd = [color texture]
    //  matte::sigma = [float texture] (sigma of Oren-Nayer shader in degrees)
    // --- translucent matte ----
    //  matte::Kr = [color texture] (reflectivity)
    //  matte::Kt = [color texture] (transmissivity)

    //  glossy::Kd = [color texture] (diffuse)
    //  glossy::Ks = [color texture] (specular)
    //  glossy::Ka = [color texture] (absorption)
    //  glossy::uroughness = [float texture] 0 to 1 (roughness in u direction)
    //  glossy::vroughness = [float texture] 0 to 1 (... in v direction)
    //  glossy::d = [float texture] (depth of absorption effects. 0=disable)
    //  glossy::index = [float texture] (IOR of coating)

    // diffuse image and color
    float diffuse_vscale =-1;
    float diffuse_uscale =1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset =0; // vdelta
    float diffuse_uoffset =0; // udelta
    QString diffuse_wrap ="repeat"; // repeat|black|clamp
    QString diffuse_filtertype ="bilinear";
    QString diffuse_channel ="";
    QString diffuse_mapfile =""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists =false;

    // specular image and color
    float spec_vscale =-1;
    float spec_uscale =1;
    float spec_gamma = 2.2;
    float spec_voffset =0;
    float spec_uoffset =0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel ="";
    QString spec_mapfile =""; // Specular Color
    QColor spec_value;
    bool spec_exists =false;

    // bump image and values
    float bump_vscale =-1;
    float bump_uscale =1;
    float bump_gamma =1;
    float bump_voffset =0;
    float bump_uoffset =0;
    QString bump_channel ="";
    QString bump_wrap ="repeat";
    QString bump_filtertype ="bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists =false;

    // transmission map
    QString opacity_mapfile ="";
    float opacity_value =1;
    bool opacity_exists =false;

    // material definition
    float uroughness=0.8;
    float vroughness=0.8;
    float index_refraction; // IOR


    // 1. search for specific properties and populate data
    // 2. generate full material block

    QString propertyLabel;
    DzProperty *currentProperty;
    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (diffuse_value != 1) || (diffuse_mapfile != "") )
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1/((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }

    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue();
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( bump_mapfile != "" )
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (opacity_value != 1) || (opacity_mapfile != "") )
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;
    }

    // Diffuse Texture Block
    if ( diffuse_exists )
        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile,
                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()),
                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);


    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateTextureBlock(matLabel + ".bump_texture", "float", bump_mapfile, QString("%1").arg(bump_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);

    // Opacity Block
    if ( opacity_exists )
        ret_str += GenerateTextureBlock(matLabel + ".opacity_texture", "float", opacity_mapfile, QString("%1").arg(opacity_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);

    // Material definition
    // decide what type of material...
    if ( !opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
    }

    if ( opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");

        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        ret_str += QString("\t\"string type\" [\"null\"]\n");

        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel+".NULL");
        ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel+".base");
    }
    
    ret_str += QString("NamedMaterial \"%1\"\n").arg(matLabel);
    
    return ret_str;
}

QString LuxProcessGenMaterial(DzMaterial *material, QString &mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";
    
    // process all material information and output a full material LXS description block
    
    
    // Lux Texture format
    // Texture "NAME" color|float|spectrum "TYPE" <values>
    // TYPE:
    //  imagemap
    //  mix
    //  scale (mulitply with color)
    // PARAMS
    //  mapping = [uv|spherical|cylindrical|planar]
    //  uscale/vscale = [float] scale texcoords
    //  udelta/vdelta = [float] offfset tex mappings
    //  v1/v2 = [vector] : define the plane for planar mapping
    //  constant::'value' = [color|float]
    //  scale::'tex1'/'tex2' = [color|float] (multiply 2 textures)
    //  imagemap::'filename' = [string] (with path)
    //  imagemap::'wrap' = [string] (repeat|black|clamp)
    //  imagemap::'filtertype' = [string] (bilinear|mipmap_trilinear|mipmap_ewa|nearest
    //  imagemap::'channel' = [string] (mean|red|green|blue|alpha|colored_mean)
    
    // Lux Material format
    //  bumpmap = [float texture]
    //  matte::Kd = [color texture]
    //  matte::sigma = [float texture] (sigma of Oren-Nayer shader in degrees)
    // --- translucent matte ----
    //  matte::Kr = [color texture] (reflectivity)
    //  matte::Kt = [color texture] (transmissivity)
    
    //  glossy::Kd = [color texture] (diffuse)
    //  glossy::Ks = [color texture] (specular)
    //  glossy::Ka = [color texture] (absorption)
    //  glossy::uroughness = [float texture] 0 to 1 (roughness in u direction)
    //  glossy::vroughness = [float texture] 0 to 1 (... in v direction)
    //  glossy::d = [float texture] (depth of absorption effects. 0=disable)
    //  glossy::index = [float texture] (IOR of coating)
    
    // diffuse image and color
    float diffuse_vscale =-1;
    float diffuse_uscale =1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset =0; // vdelta
    float diffuse_uoffset =0; // udelta
    QString diffuse_wrap ="repeat"; // repeat|black|clamp
    QString diffuse_filtertype ="bilinear";
    QString diffuse_channel ="";
    QString diffuse_mapfile =""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists =false;
    
    // specular image and color
    float spec_vscale =-1;
    float spec_uscale =1;
    float spec_gamma = 2.2;
    float spec_voffset =0;
    float spec_uoffset =0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel ="";
    QString spec_mapfile =""; // Specular Color
    QColor spec_value;
    bool spec_exists =false;
    
    // bump image and values
    float bump_vscale =-1;
    float bump_uscale =1;
    float bump_gamma =1;
    float bump_voffset =0;
    float bump_uoffset =0;
    QString bump_channel ="";
    QString bump_wrap ="repeat";
    QString bump_filtertype ="bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists =false;
    
    // transmission map
    QString opacity_mapfile ="";
    float opacity_value =1;
    bool opacity_exists =false;
    
    // material definition
    float uroughness=0.8;
    float vroughness=0.8;
    float index_refraction; // IOR
    
    
    // 1. search for specific properties and populate data
    // 2. generate full material block
    
    QString propertyLabel;
    DzProperty *currentProperty;
    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (diffuse_value != 1) || (diffuse_mapfile != "") )
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1/((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (spec_value != 1) || (spec_mapfile != "") )
            spec_exists = true;
    }
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue();
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( bump_mapfile != "" )
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (opacity_value != 1) || (opacity_mapfile != "") )
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;
    }

    // Diffuse Texture Block
    if ( diffuse_exists )
        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile, 
                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()),
                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset, 
                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);
    
    // Specular Block
    if (spec_exists)
    {
        ret_str += GenerateTextureBlock(matLabel + ".specular_color", "color", spec_mapfile, 
                                        QString("%1 %2 %3").arg(spec_value.redF()).arg(spec_value.greenF()).arg(spec_value.blueF()),
                                        spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
                                        spec_wrap, spec_filtertype, spec_channel);

        switch (YaLuxGlobal.specularMode)
        {
        case 0: // 90% Diffuse + 10% Specular
            ret_str += MixTextures(matLabel+".specular_mix", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.1]");
            break;
        case 1: // Specular * Glossiness
            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", QString("%1 %1 %1").arg(1-uroughness), "color");
            break;
        case 2: // (75% Diffuse + 25% Specular) * Glossiness
            ret_str += MixTextures(matLabel+".specular_mix1", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.25]");
            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_mix1", QString("%1 %1 %1").arg(1-uroughness), "color");
            break;
        case 3: // 10% Specular
            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "0.1 0.1 0.1", "color");
            break;
        case 4: // Full Specular
            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "1 1 1", "color");
            break;
        case 5: // Specular off
            // .specular_mix == 0
            ret_str += GenerateTextureBlock(matLabel+".specular_mix", "color", "", "0 0 0", 1, -1, 0, 0, 2.2, "", "", "");
            break;
        }


    }

    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateTextureBlock(matLabel + ".bump_texture", "float", bump_mapfile, QString("%1").arg(bump_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);
    
    // Opacity Block
    if ( opacity_exists )
        ret_str += GenerateTextureBlock(matLabel + ".opacity_texture", "float", opacity_mapfile, QString("%1").arg(opacity_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);
    
    // Material definition
    // decide what type of material...
    if ( !opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
    }
    
    if ( opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
        
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        ret_str += QString("\t\"string type\" [\"null\"]\n");
        
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel+".NULL");
        ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel+".base");
    }
    
    ret_str += QString("NamedMaterial \"%1\"\n").arg(matLabel);
    
    return ret_str;
}

QString LuxProcessGlossyMaterial(DzMaterial *material, QString &mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // process all material information and output a full material LXS description block


    // Lux Texture format
    // Texture "NAME" color|float|spectrum "TYPE" <values>
    // TYPE:
    //  imagemap
    //  mix
    //  scale (mulitply with color)
    // PARAMS
    //  mapping = [uv|spherical|cylindrical|planar]
    //  uscale/vscale = [float] scale texcoords
    //  udelta/vdelta = [float] offfset tex mappings
    //  v1/v2 = [vector] : define the plane for planar mapping
    //  constant::'value' = [color|float]
    //  scale::'tex1'/'tex2' = [color|float] (multiply 2 textures)
    //  imagemap::'filename' = [string] (with path)
    //  imagemap::'wrap' = [string] (repeat|black|clamp)
    //  imagemap::'filtertype' = [string] (bilinear|mipmap_trilinear|mipmap_ewa|nearest
    //  imagemap::'channel' = [string] (mean|red|green|blue|alpha|colored_mean)

    // Lux Material format
    //  bumpmap = [float texture]
    //  matte::Kd = [color texture]
    //  matte::sigma = [float texture] (sigma of Oren-Nayer shader in degrees)
    // --- translucent matte ----
    //  matte::Kr = [color texture] (reflectivity)
    //  matte::Kt = [color texture] (transmissivity)

    //  glossy::Kd = [color texture] (diffuse)
    //  glossy::Ks = [color texture] (specular)
    //  glossy::Ka = [color texture] (absorption)
    //  glossy::uroughness = [float texture] 0 to 1 (roughness in u direction)
    //  glossy::vroughness = [float texture] 0 to 1 (... in v direction)
    //  glossy::d = [float texture] (depth of absorption effects. 0=disable)
    //  glossy::index = [float texture] (IOR of coating)

    // diffuse image and color
    float diffuse_vscale =-1;
    float diffuse_uscale =1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset =0; // vdelta
    float diffuse_uoffset =0; // udelta
    QString diffuse_wrap ="repeat"; // repeat|black|clamp
    QString diffuse_filtertype ="bilinear";
    QString diffuse_channel ="";
    QString diffuse_mapfile =""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists =false;

    // specular image and color
    float spec_vscale =-1;
    float spec_uscale =1;
    float spec_gamma = 2.2;
    float spec_voffset =0;
    float spec_uoffset =0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel ="";
    QString spec_mapfile =""; // Specular Color
    QColor spec_value;
    bool spec_exists =false;

    // bump image and values
    float bump_vscale =-1;
    float bump_uscale =1;
    float bump_gamma =1;
    float bump_voffset =0;
    float bump_uoffset =0;
    QString bump_channel ="";
    QString bump_wrap ="repeat";
    QString bump_filtertype ="bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists =false;

    // transmission map
    QString opacity_mapfile ="";
    float opacity_value =1;
    bool opacity_exists =false;

    // material definition
    float uroughness=0.8;
    float vroughness=0.8;
    float index_refraction; // IOR


    // 1. search for specific properties and populate data
    // 2. generate full material block

    QString propertyLabel;
    DzProperty *currentProperty;
    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (diffuse_value != 1) || (diffuse_mapfile != "") )
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1/((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (spec_value != 1) || (spec_mapfile != "") )
            spec_exists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue()/100;
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( bump_mapfile != "" )
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ( (opacity_value != 1) || (opacity_mapfile != "") )
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;
    }

    // Diffuse Texture Block
    if ( diffuse_exists )
        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile,
                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()),
                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);

    // Specular Block
    if (spec_exists)
    {
        ret_str += GenerateTextureBlock(matLabel + ".specular_color", "color", spec_mapfile,
                                        QString("%1 %2 %3").arg(spec_value.redF()).arg(spec_value.greenF()).arg(spec_value.blueF()),
                                        spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
                                        spec_wrap, spec_filtertype, spec_channel);

        switch (YaLuxGlobal.specularMode)
        {
            case 0: // 90% Diffuse + 10% Specular
                ret_str += MixTextures(matLabel+".specular_mix", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.1]");
                break;
            case 1: // Specular * Glossiness
                ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", QString("%1 %1 %1").arg(1-uroughness), "color");
                break;
            case 2: // (75% Diffuse + 25% Specular) * Glossiness
                ret_str += MixTextures(matLabel+".specular_mix1", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.25]");
                ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_mix1", QString("%1 %1 %1").arg(1-uroughness), "color");
                break;
            case 3: // 10% Specular
                ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "0.1 0.1 0.1", "color");
                break;
            case 4: // Full Specular
                ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "1 1 1", "color");
                break;
            case 5: // Specular off
                    // .specular_mix == 0
                ret_str += GenerateTextureBlock(matLabel+".specular_mix", "color", "", "0 0 0", 1, -1, 0, 0, 2.2, "", "", "");
                break;
        }


    }

    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateTextureBlock(matLabel + ".bump_texture", "float", bump_mapfile, QString("%1").arg(bump_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);

    // Opacity Block
    if ( opacity_exists )
        ret_str += GenerateTextureBlock(matLabel + ".opacity_texture", "float", opacity_mapfile, QString("%1").arg(opacity_value),
                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
                                        bump_wrap, bump_filtertype, bump_channel);

    // Material definition
    // decide what type of material...
    if ( !opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
//        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
    }

    if ( opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
//        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);

        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        ret_str += QString("\t\"string type\" [\"null\"]\n");

        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel+".NULL");
        ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel+".base");
    }
    
    ret_str += QString("NamedMaterial \"%1\"\n").arg(matLabel);
    
    return ret_str;
}


QString LuxProcessObject(DzObject *daz_obj, QString &mesg)
{
    QString nodeAssetId;
    QString nodeLabel;
    DzGeometry *geo;
    DzShape *shape;
    QString outstr = "";
    QString attributeblock = "";

    // Object -> Shape
    shape = daz_obj->getCurrentShape();
    QString shapeLabel = shape->getLabel();
    
    nodeLabel = shape->getNode()->getLabel();
    nodeAssetId = shape->getNode()->getAssetId();

    // Shape -> Geometry
    geo = daz_obj->getCachedGeom();
    QString geoLabel = QString("numvertices = %1").arg(geo->getNumVertices() );

    // DEBUG
    if (YaLuxGlobal.debugLevel > 2)
        mesg += QString("\tobject(node label) = [%1], shape = [%2], %3\n").arg(nodeLabel).arg(shapeLabel).arg(geoLabel) ;
    //  dzApp->log( QString("\tobject(node label) = [%1], shape = [%2], %3").arg(nodeLabel).arg(shapeLabel).arg(geoLabel) ) ;

    // Shape -> MaterialList
    //DzMaterialPtrList materialList;
    QString matLabel;

    //shape->getAllRenderPrioritizedMaterials(materialList);
    //int numMaterials = shape->getNumAssemblyMaterials();
    int numMaterials = shape->getNumMaterials();
    QObjectList texList;

    // TEXTURES 
//    while (i < materialList.count() )
    for (int i=0; i < numMaterials; i++)
    {
        attributeblock = "AttributeBegin\n";
        //matLabel = materialList[i]->getLabel();
        DzMaterial *material = shape->getAssemblyMaterial(i);
        matLabel = material->getLabel();
        // DEBUG
//        if (YaLuxGlobal.debugLevel > 1)
//            mesg += QString("\t\tmaterial[%1] = getLabel[%2], getName[%3], getMaterialName[%4]").arg(i).arg(matLabel).arg(materialList[i]->getName()).arg(materialList[i]->getMaterialName()) ;
//            dzApp->log( QString("\t\tmaterial[%1] = getLabel[%2], getName[%3], getMaterialName[%4]").arg(i).arg(matLabel).arg(materialList[i]->getName()).arg(materialList[i]->getMaterialName()) );
        //////////////////////////////////
        // Read the Daz material properties and generate the luxrender material block
        ///////////////////////////////////
        //attributeblock += LuxProcessGlossyMaterial(materialList[i], mesg, matLabel);
        attributeblock += LuxProcessGlossyMaterial(material, mesg, matLabel);

        // process related vertex group for this material
        QString objMatName = QString("%1.%2").arg(nodeLabel).arg(matLabel);

        ////////////////////////////////////////
        // Convert meshlight object to luxrender area light as appropriate
        ////////////////////////////////////////
        bool bIsAreaLight = false;
        QString strColor= "";
        QString strGain = "1";
        QString strEfficacy = "17";
        QString strPower = "100";
        // Check to see if this is an area light material
        //DzPropertyGroupTree *propertygrouptree = materialList[i]->getPropertyGroups();
        DzPropertyGroupTree* propertygrouptree = material->getPropertyGroups();
        if ( matLabel.contains("RealityLight") )
        {
//            strColor = LuxGetStringProperty(materialList[i], "Diffuse Color", mesg);
            strColor = LuxGetStringProperty(material, "Diffuse Color", mesg);
//            strGain = LuxGetStringProperty(materialList[i], "Diffuse Strength", mesg);
            strGain = LuxGetStringProperty(material, "Diffuse Strength", mesg);
            bIsAreaLight = true;
        }
        else if ( propertygrouptree->findChild("LuxRender") != NULL)
        {
//            if ( LuxGetStringProperty(materialList[i], "LuxRender_material_enablelight", mesg) == "true")
            if ( LuxGetStringProperty(material, "LuxRender_material_enablelight", mesg) == "true")
            {
                bIsAreaLight = true;
//                strColor = LuxGetStringProperty(materialList[i], "LuxRender_matte_Kd", mesg);
                strColor = LuxGetStringProperty(material, "LuxRender_matte_Kd", mesg);
            }
            // DEBUG
            if (YaLuxGlobal.debugLevel > 3)
                dzApp->log("yaluxplug: is Luxus area light? =[" + mesg + "]\n");
        }
        else if ( propertygrouptree->findChild("Light") != NULL)
        {
//            strColor = LuxGetStringProperty(materialList[i], "Color", mesg);
            strColor = LuxGetStringProperty(material, "Color", mesg);
//            strGain = LuxGetStringProperty(materialList[i], "Intensity", mesg);
            strGain = LuxGetStringProperty(material, "Intensity", mesg);
            bIsAreaLight = true;
        }
        if (bIsAreaLight)
        {
            YaLuxGlobal.bDefaultLightsOn = false;
            attributeblock += "AreaLightSource \"area\"\n";
            attributeblock += QString("\t\"color L\"\t[%1]\n").arg(strColor);
            attributeblock += QString("\t\"float gain\"\t[%1]\n").arg(strGain);
            attributeblock += QString("\t\"float power\"\t[%1]\n").arg(strPower);
            attributeblock += QString("\t\"float efficacy\"\t[%1]\n").arg(strEfficacy);
        }

/*
        DzMatrix3 mat3 = (shape->getNode())->getWSTransform();
        DzMatrix4 mat4 = mat3.matrix4();

         attributeblock += "Transform [";
         attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
         // row[1] and row[2] are switched
         attributeblock += QString("%1 %2 %3 %4 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
         attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);        
         // row[3] needs to be scaled
         attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[3][0]/100).arg(-mat4[3][2]/100).arg(mat4[3][1]/100).arg(mat4[3][3]);
         attributeblock += "]\n";
*/        

        QString plyFileName;
        if ( geo->inherits("DzFacetMesh") )
        {
//            DazToPLY dzPLYexport((DzFacetMesh *)geo, objMatName, materialList[i]);
            DazToPLY dzPLYexport((DzFacetMesh*)geo, objMatName, material);
            //            plyFileName = dzPLYexport.LuxMakeAsciiPLY();
            plyFileName = dzPLYexport.LuxMakeBinPLY();
            //            plyFileName = LuxMakePLY((DzFacetMesh*)mesh, meshName, materialList[i]);
            //            outstr += LuxMakeSHAPE( (DzFacetMesh*)mesh, meshName);
        }
        if (plyFileName != "")
        {   
            // add in reference to plyFileName
            attributeblock += "Shape \"plymesh\"\n";
            attributeblock += QString("\t\"string filename\" [\"%1\"]\n").arg(plyFileName);

            attributeblock += "AttributeEnd\n\n";
        } else {
            // no shapes to render for this material, just scrap this attribute block and move on
            attributeblock = "";
        }
        outstr += attributeblock;
        //i++;
    }
    // don't call dzapp->log because mesg is passed back to the calling function
    // DEBUG
//    if (YaLuxGlobal.debugLevel > 2)
//        dzApp->log(mesg);
    
    return outstr;

}


bool LuxMakeLXSFile(QString fileNameLXS, DzRenderer *r, DzCamera *camera, const DzRenderOptions &opt)
{
    YaLuxGlobal.bDefaultLightsOn = true;
    QSize renderImageSize;
    int ImgWidth, ImgHeight;
    QString mesg;

    // open stream to write to file
    dzApp->log("yaluxplug: opening LXSFile = " + fileNameLXS );
    QFile outLXS(fileNameLXS);
    outLXS.open(QIODevice::WriteOnly | QIODevice::Truncate);
    outLXS.write("# Generated by yaluxplug \n");

    // 1. read render options to set up environment
//    YaLuxGlobal.RenderProgress->step();
    YaLuxGlobal.RenderProgress->setCurrentInfo("Creating LXS scene file...");

    renderImageSize = opt.getImageSize();
    ImgHeight = renderImageSize.height();
    ImgWidth = renderImageSize.width();


    QRect cropWindow;
    cropWindow = YaLuxGlobal.handler->getCropWindow();
    if ( (cropWindow.top()!= 0) && (cropWindow.bottom()!=1) && (cropWindow.left()!=0) && cropWindow.right()!=1)
        YaLuxGlobal.bIsSpotRender=true;
    else
        YaLuxGlobal.bIsSpotRender=false;

    //////////////////////
    // Renderer
    ////////////////////
    if (YaLuxGlobal.bIsSpotRender)
    {
        outLXS.write("\nRenderer \"sampler\"\n");
    }
    else
    {
        switch (YaLuxGlobal.renderMode)
        {
        case 0: // Software
            mesg = "\nRenderer \"sampler\"\n";
            break;
        case 1: // Hybrid
                mesg = "\nRenderer \"hybrid\"\n";
                mesg += "\t\"string localconfigfile\" [\"local.cfg\"]\n";
                break;
        case 2: // OpenCL GPU only
            mesg = "\nRenderer \"slg\"\n";
            mesg += QString("\t\"string config\" [\"renderengine.type = %1\" \"opencl.cpu.use = 0\" \"opencl.kernelcache = %2\"]\n").arg("PATHOCL").arg("PERSISTENT");
            break;
        case 3: // OpenCL CPU only
            mesg = "\nRenderer \"slg\"\n";
            mesg += QString("\t\"string config\" [\"renderengine.type = %1\" \"opencl.gpu.use = 0\" \"opencl.kernelcache = %2\"]\n").arg("PATHOCL").arg("PERSISTENT");
            break;
        case 4: // OpenCL GPU+CPU
            mesg = "\nRenderer \"slg\"\n";
            mesg += QString("\t\"string config\" [\"renderengine.type = %1\" \"opencl.cpu.use = 1\" \"opencl.gpu.use = 1\" \"opencl.kernelcache = %2\"]\n").arg("PATHOCL").arg("PERSISTENT");
            break;
        case 5: // custom
            mesg = "\n" + YaLuxGlobal.customRenderString + "\n";
            break;
        }
        outLXS.write(mesg.toAscii());
    }


    ///////////////////////////
    // Fleximage Film settings
    //////////////////////////
    outLXS.write("\n");
    outLXS.write(LXSfilm.join("").toAscii());

    QString colorChannels;
    if (YaLuxGlobal.bSaveAlphaChannel)
        colorChannels = "RGBA";
    else
        colorChannels = "RGB";
    outLXS.write( QString("\t\"string write_png_channels\"\t[\"%1\"]\n").arg(colorChannels).toAscii() );
    //    outLXS.write( QString("\t\"string write_tga_channels\"\t[\"%1\"]\n").arg(colorChannels) );
    outLXS.write( QString("\t\"string write_exr_channels\"\t[\"%1\"]\n").arg(colorChannels).toAscii() );
    outLXS.write( QString("\t\"integer haltspp\"\t[%1]\n").arg(YaLuxGlobal.haltAtSamplesPerPixel).toAscii() );
    outLXS.write( QString("\t\"integer halttime\"\t[%1]\n").arg(YaLuxGlobal.haltAtTime).toAscii() );
    outLXS.write( QString("\t\"float haltthreshold\"\t[%1]\n").arg(1.0-YaLuxGlobal.haltAtThreshold).toAscii() );
    outLXS.write( QString("\t\"string tonemapkernel\"\t[\"%1\"]\n").arg(YaLuxGlobal.LuxToneMapper).toAscii() );
    if (YaLuxGlobal.LuxToneMapper == "linear")
    {
        outLXS.write( QString("\t\"float linear_exposure\"\t[%1]\n").arg(YaLuxGlobal.tonemapExposureTime).toAscii() );
        outLXS.write( QString("\t\"float linear_gamma\"\t[%1]\n").arg(YaLuxGlobal.tonemapGamma).toAscii() );
        outLXS.write( QString("\t\"float linear_fstop\"\t[%1]\n").arg(YaLuxGlobal.tonemapFstop).toAscii() );
        outLXS.write( QString("\t\"float linear_sensitivity\"\t[%1]\n").arg(YaLuxGlobal.tonemapISO).toAscii() );
    }

    // Film resolution
    mesg = QString("\t\"integer xresolution\"\t[%1]\n").arg(ImgWidth);
    mesg += QString("\t\"integer yresolution\"\t[%1]\n").arg(ImgHeight);
    cropWindow = YaLuxGlobal.handler->getCropWindow();
    if ( YaLuxGlobal.bIsSpotRender )
    {
        mesg += QString("\t\"float cropwindow\"\t[%1 %2 %3 %4]\n").arg((float)cropWindow.top()/ImgHeight).arg((float)cropWindow.bottom()/ImgHeight).arg((float)cropWindow.left()/ImgWidth).arg((float)cropWindow.right()/ImgWidth);
    }
    YaLuxGlobal.cropWindow = cropWindow;
    outLXS.write(mesg.toAscii());

    ///////////////////////////////////////////////
    // Film image filename
    ///////////////////////////////////////////////
    mesg = QString("\t\"string filename\"\t[\"%1\"]\n").arg(DzFileIO::getBaseFileName(YaLuxGlobal.workingRenderFilename));
    outLXS.write(mesg.toAscii());
    //    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);


    // sampler settings
    outLXS.write("\n");
    outLXS.write(LXSsampler.join("").toAscii());

    // surface integrator settings
    outLXS.write("\n");
    outLXS.write(LXSsurfaceintegrator.join("").toAscii());

    // volume integrator settings
    outLXS.write("\n");
    outLXS.write(LXSvolumeintegrator.join("").toAscii());

    // pixelfilter settings
    outLXS.write("\n");
    outLXS.write(LXSpixelfilter.join("").toAscii());

    // accelerator settings
    outLXS.write("\n");
    outLXS.write(LXSaccelerator.join("").toAscii());

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
    outLXS.write(mesg.toAscii());

    // 2. set the camera vector and orientation
    // Camera
    outLXS.write("\nCamera \"perspective\"\n");
    // fov
    double f_fov = camera->getFieldOfView() * 57.295779513; // multiply by (180/pi)
    outLXS.write(QString("\t\"float fov\"\t[%1]\n").arg(f_fov).toAscii());
    // screenwindow
    double aspectRatio = opt.getAspect();
    if (aspectRatio == 0)
        aspectRatio = ImgWidth/ImgHeight;
    float screenWindow[4];
    //    if (aspectRatio > 1) {
    screenWindow[0] = -aspectRatio;
    screenWindow[1] = aspectRatio;
    screenWindow[2] = -1;
    screenWindow[3] = 1;
    //    }
    /*    else {
     screenWindow[0] = -1;
     screenWindow[1] = 1;
     screenWindow[2] = -1/aspectRatio;
     screenWindow[3] = 1/aspectRatio;
     }
     */
    outLXS.write(QString("\t\"float screenwindow\"\t[%1 %2 %3 %4]\n").arg(screenWindow[0]).arg(screenWindow[1]).arg(screenWindow[2]).arg(screenWindow[3]).toAscii() );


    // single image
    switch (opt.getRenderImgToId())
    {
        case DzRenderOptions::ActiveView:
        case DzRenderOptions::NewWindow:
        case DzRenderOptions::DirectToFile: ;
//            fileName = opt.getRenderImgFilename();
    }
    //    fileName = dzApp->getTempRenderFilename();
    //    fileName.replace(tempPath, "");
//    mesg = "image filename = " + fileName;
//    YaLuxGlobal.RenderProgress->step();
//    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);

    // image series
    switch (opt.getRenderMovToId())
    {
        case DzRenderOptions::MovieFile:
//            fileName = opt.getRenderMovFilename();
        case DzRenderOptions::ImageSeries:;
//            fileName = opt.getRenderSerFilename();
    }
//    mesg = "movie filename = " + fileName;
//    YaLuxGlobal.RenderProgress->step();
    //    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);


//////////////////////////////////////////////////////////

    // World Begin
    outLXS.write("\nWorldBegin\n");
    //    outLXS.write("\nCoordSysTransform \"camera\"\n");

    ///////////////////////////////////////////////
    // LIGHTS
    ///////////////////////////////////////////////
    DzLightListIterator lightList = dzScene->lightListIterator();
    DzLight *currentLight = NULL;
    QString outstr;
    mesg = "";
    while (lightList.hasNext())
    {
        currentLight = lightList.next();
        // DEBUG
        mesg = QString("yaluxplug: Processing Light: AssetId[%1], Label[%2]\n").arg(currentLight->getAssetId()).arg(currentLight->getLabel());
        outstr = LuxProcessLight(currentLight, mesg);
        outLXS.write(outstr.toAscii());
        dzApp->log(mesg);

    }
    mesg = "";

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

    imageMgr->prepareAllImages(r);
    //emit imageMgr->prepareAllImages(r);


    ///////////////////////////////////////////////
    // NODES
    ///////////////////////////////////////////////
    DzNodeListIterator nodeList = dzScene->nodeListIterator();
    DzNode *currentNode;
    QString nodeAssetId;
    int parentNodeIndex=-1;
    while (nodeList.hasNext())
    {
        parentNodeIndex++;
        currentNode = nodeList.next();
        nodeAssetId = currentNode->getAssetId();

        // Process Node
        YaLuxGlobal.currentNode = currentNode;
        YaLuxGlobal.settings = new DzRenderSettings(r, &opt);
        dzApp->log("yaluxplug: Looking at Node: AssetId=[" + nodeAssetId + QString("] (%1 of %2 Scene Level Nodes)").arg(1+parentNodeIndex).arg(dzScene->getNumNodes()) );

        // Node -> Object
        DzObject *currentObject = currentNode->getObject();
        if ( (currentObject != NULL) && (currentNode->isVisible()) )
        {
            if ( currentNode->getLabel().contains("EnvironmentSphere") )
            {
                // DEBUG
                if (YaLuxGlobal.debugLevel >=1) // debugging data
                    dzApp->log("yaluxplug: DEBUG: Skip EnvironmentSphere");
            }
            else if (currentNode->getLabel().contains("Genitalia") )
            {
                if (YaLuxGlobal.debugLevel >=2) // debugging data
                    dzApp->log("yaluxplug: DEBUG: Skipping geografted genitalia");
            }
            else
            {
                outLXS.write( QString("\n# AssetId=[" + nodeAssetId +"],nodeLabel=[" + currentNode->getLabel() + "]\n").toAscii() );
                QString objectLabel = currentObject->getLabel();

                // DB (2021-06-15) This is probably not needed and may be introducing bad rener data
                //// FINALIZE Node's geometry cache for rendering
                ////currentNode->finalize(true,true);
                //currentObject->finalize(*currentNode, true, true);

                QString output;
                output = LuxProcessObject(currentObject, mesg);
                outLXS.write(output.toAscii());
            }
        } else {
            if ( currentNode->isHidden() )
                dzApp->log("\tnode is hidden.");
            else if ( currentNode == NULL )
                dzApp->log("\tno object found.");
        }

        // DEBUG
        if ( YaLuxGlobal.debugLevel >= 2 ) // debug data
        {
            mesg = "Properties for node = " + nodeAssetId + "\n";
            LuxProcessProperties(currentNode, mesg);
            dzApp->log(mesg);
            mesg = "";

            // Process child nodes
            QString output;
            dzApp->log("**Processing child nodes for " + nodeAssetId + QString(" (%1 children total)").arg(currentNode->getNumNodeChildren()) + "***" );
            output = processChildNodes(currentNode, mesg, nodeAssetId);
            dzApp->log(mesg);

            if (output != "")
                outLXS.write(output.toAscii());
        }

        // Call the RenderMan pathway on this node
        //dzApp->log("yaluxplug: Calling Node->render() # " + label);
        //currentNode->render(*YaLuxGlobal.settings);

        outLXS.flush();
    }

    if (YaLuxGlobal.bDefaultLightsOn == true)
    {
        // Create a DEFAULT SUN if no light exists
        outLXS.write("\nAttributeBegin\n");
        outLXS.write("LightSource \"sun\"\n");
        outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1).toAscii() );
        outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(0).arg(-0.8).arg(0.2).toAscii() );
        outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(0.0003).toAscii() );

        outLXS.write("LightSource \"sky2\"\n");
        outLXS.write(QString("\t\"float importance\"\t[%1]\n").arg(1).toAscii() );
        outLXS.write(QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(0).arg(-0.8).arg(0.2).toAscii() );
        outLXS.write(QString("\t\"float gain\"\t[%1]\n").arg(0.0001).toAscii() );
        outLXS.write("AttributeEnd\n");
    }


    outLXS.write("\nWorldEnd\n");
    outLXS.close();

    mesg = "LXS creation complete.";
//    YaLuxGlobal.RenderProgress->step();
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);


    return true;
}

bool LuxMakeCFGFile(QString filenameCFG, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt)
{
    YaLuxGlobal.bDefaultLightsOn = true;
    QSize renderImageSize;
    int ImgWidth, ImgHeight;
    QString mesg;

    // open stream to write to file
    dzApp->log("yaluxplug: writing CFGFile = " + filenameCFG);
    QFile outCFG(filenameCFG);
    outCFG.open(QIODevice::WriteOnly | QIODevice::Truncate);
    outCFG.write("# Generated by yaluxplug \n");

    // 1. read render options to set up environment
//    YaLuxGlobal.RenderProgress->step();
    YaLuxGlobal.RenderProgress->setCurrentInfo("Creating CFG file...");

    renderImageSize = opt.getImageSize();
    ImgHeight = renderImageSize.height();
    ImgWidth = renderImageSize.width();


    QRect cropWindow;
    cropWindow = YaLuxGlobal.handler->getCropWindow();
    if ((cropWindow.top() != 0) && (cropWindow.bottom() != 1) && (cropWindow.left() != 0) && cropWindow.right() != 1)
        YaLuxGlobal.bIsSpotRender = true;
    else
        YaLuxGlobal.bIsSpotRender = false;

    //////////////////////
    // Renderer
    ////////////////////
    if (YaLuxGlobal.bIsSpotRender)
    {
        //outLXS.write("\nRenderer \"sampler\"\n");
        outCFG.close();
        return false;
    }
    else
    {
        mesg = "\nrenderengine.type = ";
        switch (YaLuxGlobal.renderMode)
        {
        case 0: // Software
            mesg += "\"PATHCPU\"\n";
            break;
        case 1: // Hybrid
//            return false;
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 32\n";
            mesg += "opencl.gpu.use = 1\n";
            break;
        case 2: // OpenCL GPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 0\n";
            mesg += "opencl.gpu.use = 1\n";
            mesg += "native.threads.count = 0\n";
            mesg += "opencl.native.threads.count = 0\n";
            break;
        case 3: // OpenCL CPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 32\n";
            mesg += "opencl.gpu.use = 0\n";
            break;
        case 4: // OpenCL GPU+CPU
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 32\n";
            mesg += "opencl.gpu.use = 1\n";
            break;
        case 5: // custom
            mesg = "\n" + YaLuxGlobal.customRenderString + "\n";
            break;
        }
    }
    outCFG.write(mesg.toAscii());

    /////////////////////////////////////////
    // Search for Tonemapper and Environment Nodes
    /////////////////////////////////////////
    DzNode* tonemapper = dzScene->findNode("Tonemapper Options");
    if (tonemapper)
    {
        float floatval = 0;
        YaLuxGlobal.LuxToneMapper = "linear";
        if (LuxGetFloatProperty(tonemapper, "Film ISO", floatval, mesg))
            YaLuxGlobal.tonemapISO = floatval;
        if (LuxGetFloatProperty(tonemapper, "Gamma", floatval, mesg))
            YaLuxGlobal.tonemapGamma = floatval;
        if (LuxGetFloatProperty(tonemapper, "Shutter Speed", floatval, mesg))
            YaLuxGlobal.tonemapExposureTime = 1/(floatval > 0 ? floatval : 0.00000001);
        if (LuxGetFloatProperty(tonemapper, "Aperture", floatval, mesg))
            YaLuxGlobal.tonemapFstop = floatval;
        if (LuxGetFloatProperty(tonemapper, "Film ISO", floatval, mesg))
            YaLuxGlobal.tonemapISO = floatval;
    }

    // sampler settings
    outCFG.write("\nsampler.type = \"METROPOLIS\"");
    //outLXS.write(LXSsampler.join("").toAscii());

    //// surface integrator settings
    //outLXS.write("\n");
    //outLXS.write(LXSsurfaceintegrator.join("").toAscii());

    //// volume integrator settings
    //outLXS.write("\n");
    //outLXS.write(LXSvolumeintegrator.join("").toAscii());

    //// pixelfilter settings
    outCFG.write("\nfilm.filter.type = \"MITCHELL\"\n");
    //outLXS.write("\n");
    //outLXS.write(LXSpixelfilter.join("").toAscii());

    //// accelerator settings
    //outLXS.write("\n");
    //outLXS.write(LXSaccelerator.join("").toAscii());

    ///////////////////////////
    // Film settings
    //////////////////////////
    mesg = "\n";
    mesg += "film.safesave = 1\n";
    mesg += "film.outputs.safesave = 1\n";
    mesg += "periodicsave.film.outputs.period = 3\n";
    mesg += "film.imagepipelines.0.0.type = \"NOP\"\n";
    if (YaLuxGlobal.LuxToneMapper == "linear")
    {
        mesg += "film.imagepipelines.0.1.type = \"TONEMAP_LUXLINEAR\"\n";
        //mesg += "film.imagepipelines.0.1.scale = 1\n";
        mesg += QString("film.imagepipelines.0.1.sensitivity = %1\n").arg(YaLuxGlobal.tonemapISO); // ISO
        mesg += QString("film.imagepipelines.0.1.exposure = %1\n").arg(YaLuxGlobal.tonemapExposureTime);
        mesg += QString("film.imagepipelines.0.1.fstop = %1\n").arg(YaLuxGlobal.tonemapFstop);

        mesg += "film.imagepipelines.0.2.type = \"GAMMA_CORRECTION\"\n";
        mesg += QString("film.imagepipelines.0.2.value = %1\n").arg(YaLuxGlobal.tonemapGamma);
    }
    else if (YaLuxGlobal.LuxToneMapper == "reinhard")
    {
        mesg += "film.imagepipelines.0.1.type = \"TONEMAP_REINHARD02\"\n";
    }
    else if (YaLuxGlobal.LuxToneMapper == "autolinear")
    {
        mesg += "film.imagepipelines.0.1.type = \"TONEMAP_AUTOLINEAR\"\n";
    }
    else
    {
        mesg += "film.imagepipelines.0.1.type = \"TONEMAP_AUTOLINEAR\"\n";
        //outCFG.close();
        //return false;
    }
    outCFG.write(mesg.toAscii());

    mesg = "\n";
    if (YaLuxGlobal.bSaveAlphaChannel)
        mesg += "film.outputs.0.type = \"RGBA_IMAGEPIPELINE\"\n";
    else
        mesg += "film.outputs.0.type = \"RGB_IMAGEPIPELINE\"\n";
    mesg += "film.outputs.0.index = 0\n";
    //mesg += "film.outputs.0.filename = \"RGB_IMAGEPIPELINE.png\"\n";
    mesg += QString("film.outputs.0.filename = \"%1.png\"\n").arg(DzFileIO::getBaseFileName(YaLuxGlobal.workingRenderFilename));
    outCFG.write(mesg.toAscii());

    // Film resolution
    mesg = QString("film.width = %1\n").arg(ImgWidth);
    mesg += QString("film.height = %1\n").arg(ImgHeight);
    outCFG.write(mesg.toAscii());


    ///////////////////////////
    // HALT settings
    //////////////////////////
    mesg = "\n";
    mesg += QString("batch.haltspp = %1\n").arg(YaLuxGlobal.haltAtSamplesPerPixel);
    mesg += QString("batch.halttime = %1\n").arg(YaLuxGlobal.haltAtTime);
    mesg += QString("batch.haltthreshold = %1\n").arg(1.0 - YaLuxGlobal.haltAtThreshold);
    outCFG.write(mesg.toAscii());

    mesg = QString("\nscene.file = \"%1.scn\"\n").arg(DzFileIO::getBaseFileName(YaLuxGlobal.workingRenderFilename));
    outCFG.write(mesg.toAscii());
    outCFG.close();

    return false;
}

bool LuxMakeSCNFile(QString filenameSCN, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt)
{
    YaLuxGlobal.bDefaultLightsOn = true;
    QSize renderImageSize;
    int ImgWidth, ImgHeight;
    QString mesg;
    QString outstr;

    // open stream to write to file
    dzApp->log("yaluxplug: opening SCNFile = " + filenameSCN);
    QFile outSCN(filenameSCN);
    outSCN.open(QIODevice::WriteOnly | QIODevice::Truncate);
    outSCN.write("# Generated by yaluxplug \n");

    // 1. read render options to set up environment
//    YaLuxGlobal.RenderProgress->step();
    YaLuxGlobal.RenderProgress->setCurrentInfo("Creating SCN scene file...");

    renderImageSize = opt.getImageSize();
    ImgHeight = renderImageSize.height();
    ImgWidth = renderImageSize.width();


    QRect cropWindow;
    cropWindow = YaLuxGlobal.handler->getCropWindow();
    if ((cropWindow.top() != 0) && (cropWindow.bottom() != 1) && (cropWindow.left() != 0) && cropWindow.right() != 1)
        YaLuxGlobal.bIsSpotRender = true;
    else
        YaLuxGlobal.bIsSpotRender = false;


    //////////////////////
    // Camera
    ////////////////////
    // Lookat
    const float matrixLux[16] = {
        0.01, 0, 0, 0,
        0, 0, 0.01, 0,
        0, -0.01, 0, 0,
        0, 0, 0, 1 };
    DzMatrix4 luxTransform(matrixLux);
    DzVec3 pos1 = camera->getWSPos();
    DzVec3 target1 = camera->getFocalPoint();
    DzQuat rot = camera->getWSRot();
    DzVec3 up1 = rot.multVec(DzVec3(0, 100, 0));
    outSCN.write("\n");
    DzVec3 pos = luxTransform.multVecMatrix(pos1);
    DzVec3 target = luxTransform.multVecMatrix(target1);
    DzVec3 up = luxTransform.multVecMatrix(up1);
    mesg = "scene.camera.type = \"perspective\"\n";
    mesg += QString("scene.camera.lookat.orig = %1 %2 %3 \n").arg(pos.m_x).arg(pos.m_y).arg(pos.m_z);
    mesg += QString("scene.camera.lookat.target = %1 %2 %3 \n").arg(target.m_x).arg(target.m_y).arg(target.m_z);
    mesg += QString("scene.camera.up = %1 %2 %3 \n").arg(up.m_x).arg(up.m_y).arg(up.m_z);
    // fov
    double f_fov = camera->getFieldOfView() * 57.295779513; // multiply by (180/pi)
    //outLXS.write(QString("\t\"float fov\"\t[%1]\n").arg(f_fov).toAscii());
    mesg += QString("scene.camera.fieldofview = %1\n").arg(f_fov);
    //////////////////////
    // Spotrender???
    ////////////////////
    if (YaLuxGlobal.bIsSpotRender)
    {
        return false;
    }
    else
    {
        // screenwindow
        double aspectRatio = opt.getAspect();
        if (aspectRatio == 0)
            aspectRatio = ImgWidth / ImgHeight;
        float screenWindow[4];
        screenWindow[0] = -aspectRatio;
        screenWindow[1] = aspectRatio;
        screenWindow[2] = -1;
        screenWindow[3] = 1;
        //outLXS.write(QString("\t\"float screenwindow\"\t[%1 %2 %3 %4]\n").arg(screenWindow[0]).arg(screenWindow[1]).arg(screenWindow[2]).arg(screenWindow[3]).toAscii());
        mesg += QString("scene.camera.screenwindow = %1 %2 %3 %4\n").arg(screenWindow[0]).arg(screenWindow[1]).arg(screenWindow[2]).arg(screenWindow[3]);
    }
    outSCN.write(mesg.toAscii());


    ///////////////////////////////////////////////
    // LIGHTS
    ///////////////////////////////////////////////
    DzNode* environment = dzScene->findNode("Environment Options");
    if (environment)
    {
        YaLuxGlobal.bDefaultLightsOn = false;
        float floatval = 0;
        QString stringval = "";
        outstr = "scene.lights.environment_options.type = \"infinite\"\n";
        stringval = LuxGetImageMapProperty(environment, "Environment Map", mesg);
        if (stringval != "")
        {
            outstr += QString("scene.lights.environment_options.file = \"%1\"\n").arg(stringval);
        }
        if (LuxGetFloatProperty(environment, "Environment Intensity", floatval, mesg))
            outstr += QString("scene.lights.environment_options.gain = %1 %1 %1\n").arg(floatval * 10000);
        outstr += "scene.lights.environment_options.gamma = 1.0\n";
        if (LuxGetFloatProperty(environment, "Dome Rotation", floatval, mesg))
        {
            DzMatrix4 mat4(true);
            mat4.rotateZ( (floatval + 165) * 0.0174533); // convert degree to radian
            outstr += QString("scene.lights.environment_options.transformation = ");
            outstr += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(mat4[0][1]).arg(mat4[0][2]).arg(mat4[0][3]);
            outstr += QString("%5 %6 %7 %8 ").arg(mat4[1][0]).arg(mat4[1][1]).arg(mat4[1][2]).arg(mat4[1][3]);
            outstr += QString("%9 %10 %11 %12 ").arg(mat4[2][0]).arg(mat4[2][1]).arg(mat4[2][2]).arg(mat4[2][3]);
            outstr += QString("%13 %14 %15 %16\n").arg(mat4[3][0]).arg(mat4[3][1]).arg(mat4[3][2]).arg(mat4[3][3]);
        }
        outSCN.write(outstr.toAscii());
    }


    DzLightListIterator lightList = dzScene->lightListIterator();
    DzLight* currentLight = NULL;
    mesg = "";
    while (lightList.hasNext())
    {
        currentLight = lightList.next();
        // DEBUG
        mesg = QString("yaluxplug: Processing Light: AssetId[%1], Label[%2]\n").arg(currentLight->getAssetId()).arg(currentLight->getLabel());
        outstr = LuxCoreProcessLight(currentLight, mesg);
        outSCN.write(outstr.toAscii());
        dzApp->log(mesg);

    }
    mesg = "";

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
    DzImageMgr* imageMgr = dzApp->getImageMgr();

    imageMgr->prepareAllImages(r);
    //emit imageMgr->prepareAllImages(r);


    ///////////////////////////////////////////////
    // NODES
    ///////////////////////////////////////////////
    DzNodeListIterator nodeList = dzScene->nodeListIterator();
    DzNode* currentNode;
    QString nodeAssetId;
    int parentNodeIndex = -1;
    while (nodeList.hasNext())
    {
        parentNodeIndex++;
        currentNode = nodeList.next();
        nodeAssetId = currentNode->getAssetId();

        // Process Node
        YaLuxGlobal.currentNode = currentNode;
        YaLuxGlobal.settings = new DzRenderSettings(r, &opt);
        dzApp->log("yaluxplug: Looking at Node: AssetId=[" + nodeAssetId + QString("] (%1 of %2 Scene Level Nodes)").arg(1 + parentNodeIndex).arg(dzScene->getNumNodes()));

        // Node -> Object
        DzObject* currentObject = currentNode->getObject();

        if (currentObject == NULL || currentNode->isHidden())
        {
            if (currentObject == NULL)
                dzApp->log("\tno object found.");
            else if (currentNode->isHidden())
                dzApp->log("\tnode is hidden.");
            continue;
        }

        if (currentNode->getLabel().contains("EnvironmentSphere"))
        {
            // DEBUG
            if (YaLuxGlobal.debugLevel >= 1) // debugging data
                dzApp->log("yaluxplug: DEBUG: Skip EnvironmentSphere");
            continue;
        }
        //else if (currentNode->getLabel().contains("Genitalia"))
        //{
        //    if (YaLuxGlobal.debugLevel >= 2) // debugging data
        //        dzApp->log("yaluxplug: DEBUG: Skipping geografted genitalia");
        //}
        if ( currentNode->inherits("DzFigure") )
        {
            DzFigure* figure = dynamic_cast<DzFigure*>(currentNode);
            if (figure->isGraftFollowing())
            {
                DzSkeleton* target = figure->getFollowTarget();
                if (target && dynamic_cast<DzNode*>(target)->isVisible() && target->isVisibileInRender())
                {
//                    dzApp->log("yaluxplug: DEBUG: skipping geograft node: " + currentNode->getName());
//                    continue;
                }

            }

        }

        outSCN.write(QString("\n# AssetId=[" + nodeAssetId + "],nodeLabel=[" + currentNode->getLabel() + "]\n").toAscii());
        QString objectLabel = currentObject->getLabel();

        //// DB (2021-06-15) This is probably not needed and may be introducing bad rener data
        //// FINALIZE Node's geometry cache for rendering
        ////currentNode->finalize(true,true);
        //currentObject->finalize(*currentNode, true, true);

        QString output;
        output = LuxCoreProcessObject(currentObject, mesg);
        outSCN.write(output.toAscii());


        // DEBUG
        if (YaLuxGlobal.debugLevel >= 2) // debug data
        {
            mesg = "Properties for node = " + nodeAssetId + "\n";
            LuxProcessProperties(currentNode, mesg);
            dzApp->log(mesg);
            mesg = "";

            // Process child nodes
            QString output;
            dzApp->log("**Processing child nodes for " + nodeAssetId + QString(" (%1 children total)").arg(currentNode->getNumNodeChildren()) + "***");
//            output = processChildNodes(currentNode, mesg, nodeAssetId);
            dzApp->log(mesg);

//            if (output != "")
//                outSCN.write(output.toAscii());
        }

        // Call the RenderMan pathway on this node
        //dzApp->log("yaluxplug: Calling Node->render() # " + label);
        //currentNode->render(*YaLuxGlobal.settings);

        outSCN.flush();
    }

    if (YaLuxGlobal.bDefaultLightsOn == true)
    {
        outstr = "scene.lights.DEFAULT_SUN.type = \"sun\"\n";
        outstr += "scene.lights.DEFAULT_SUN.dir = 0.8 -0.2 0.5\n";
        outstr += "scene.lights.DEFAULT_SUN.turbidity = 4\n";

        outstr += "scene.lights.DEFAULT_SKY.type = \"sky2\"\n";
        outstr += "scene.lights.DEFAULT_SKY.turbidity = 4\n";
        outstr += "scene.lights.DEFAULT_SKY.dir = 0.8 -0.2 0.5\n";
        outstr += "scene.lights.sun_sky_sky.gain = 0.5 0.5 0.5\n";
//        outstr += "scene.lights.DEFAULT_SKY.ground.enable = 1\n";
//        outstr += "scene.lights.DEFAULT_SKY.ground.color = 0.5 0.5 0.5\n";

        outSCN.write(outstr.toAscii());
    }


    //outLXS.write("\nWorldEnd\n");
    outSCN.close();

    mesg = "SCN creation complete.";
    //    YaLuxGlobal.RenderProgress->step();
    YaLuxGlobal.RenderProgress->setCurrentInfo(mesg);


    return true;
}

QString LuxCoreProcessLight(DzLight* currentLight, QString& mesg)
{
    QString outstr;
    QString lightLabel, lightAssetId;
    DzVec3 lightVector;
    DzVec3 lightPos;
    QColor lightColor;
    double lightIntensity = 1.0;

    if (currentLight->isVisible() == false)
        return "";

    lightLabel = SanitizeCoreLabel(currentLight->getLabel());
    lightAssetId = currentLight->getAssetId();
    lightVector = currentLight->getWSDirection();
    lightColor = currentLight->getDiffuseColor();
    if (currentLight->inherits("DzDistantLight"))
    {
        float scale = 1.0;
        LuxGetFloatProperty(currentLight, "Intensity Scale", scale, mesg);
        lightIntensity = dynamic_cast<DzDistantLight*>(currentLight)->getIntensity() * scale;
    }

    // Check to see if luxrender settings are present
    LuxProcessProperties((DzElement*)currentLight, mesg);
    int lux_light_type;
    float floatval = 0.0;
    if (LuxGetIntProperty((DzElement*)currentLight, "LuxRender_light_type", lux_light_type, mesg) == true)
    {
//        outstr = "\nAttributeBegin\n";
        outstr = "\n";
        switch (lux_light_type)
        {
        case 6: // sky2
            //outstr += "LightSource \"sky2\"\n";
            //outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
            //outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            //outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));
            //outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
            outstr += QString("scene.lights.%1.type = \"sky2\"\n").arg(lightLabel);
            outstr += QString("scene.lights.%1.turbidity = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel).arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);

            LuxGetFloatProperty(currentLight, "LuxRender_light_sky2_gain", floatval, mesg);
            outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(floatval * 1000);

            outstr += QString("scene.lights.%1.samples = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
            break;
        case 8: // Sun
            //outstr += "LightSource \"sun\"\n";
            //outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
            //outstr += QString("\t\"float relsize\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
            //outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            //outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));
            //outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
            outstr += QString("scene.lights.%1.type = \"sun\"\n").arg(lightLabel);
            outstr += QString("scene.lights.%1.turbidity = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
            outstr += QString("scene.lights.%1.relsize = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel).arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);

            LuxGetFloatProperty(currentLight, "LuxRender_light_sun_gain", floatval, mesg);
            outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(floatval * 1000);

            outstr += QString("scene.lights.%1.samples = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
            break;
        case 9: // Sun & sky2
            // sun
            outstr += QString("scene.lights.%1.type = \"sun\"\n").arg(lightLabel);
            outstr += QString("scene.lights.%1.turbidity = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
            outstr += QString("scene.lights.%1.relsize = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel).arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);

            LuxGetFloatProperty(currentLight, "LuxRender_light_sun_gain", floatval, mesg);
            outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(floatval * 1000);
            //outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));

            outstr += QString("scene.lights.%1.samples = %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
            // sky 2
            outstr += QString("scene.lights.%1.type = \"sky2\"\n").arg(lightLabel+"_sky");
            outstr += QString("scene.lights.%1.turbidity = %2\n").arg(lightLabel+"_sky").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel+"_sky").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);

            LuxGetFloatProperty(currentLight, "LuxRender_light_sky2_gain", floatval, mesg);
            outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel+"_sky").arg(floatval * 1000);
            //outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));

            outstr += QString("scene.lights.%1.samples = %2\n").arg(lightLabel+"_sky").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
            break;
        default:
            // not implemented, reset the string and break out to continue the normal pathway
            outstr = "";
            break;
            //add area light source reference line
        }
        // If the Lux_light_type was implemented, then this string has not been reset. Finish up and return str
        if (outstr != "")
        {
            //outstr += QString("\t\"float importance\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_importance", mesg));
            //outstr += QString("%1\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_extrasettings", mesg));
            //outstr += "\nAttributeEnd\n";
            YaLuxGlobal.bDefaultLightsOn = false;
            // don't need to continue
            return outstr;
        }
    }


    // If not present, then handle as a generic DazLight
    //outstr = "\nAttributeBegin\n";
    outstr = "";
    //outstr += QString("LightGroup\t\"%1\" # %2\n").arg(currentLight->getAssetId()).arg(lightLabel);
    // if IBL, create an infinite light
    if (currentLight->getAssetId().toLower().contains("image based light") || lightLabel.toLower().contains("infinite"))
    {
        //QColor lightColor = currentLight->getDiffuseColor();
        //outstr += "LightSource \"infinite\"\n";
        //outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        //QString mapname = propertyNumericImagetoString((DzNumericProperty*)currentLight->findProperty("Color"));
        //if (mapname != "")
        //    outstr += QString("\t\"string mapname\"\t[\"%1\"]\n").arg(mapname);
        //outstr += "\nAttributeEnd\n";
        outstr += QString("scene.lights.%1.type = \"infinite\"\n").arg(lightLabel);
        QString mapname = propertyNumericImagetoString((DzNumericProperty*)currentLight->findProperty("Color"));
        if (mapname != "")
            outstr += QString("scene.lights.%1.file = \"%2\"\n").arg(lightLabel).arg(mapname);
        outstr += QString("scene.lights.%1.gain = %2 %3 %4\n").arg(lightLabel).arg(lightColor.redF() * lightIntensity).arg(lightColor.greenF() * lightIntensity).arg(lightColor.blueF() * lightIntensity);
        YaLuxGlobal.bDefaultLightsOn = false;

        return outstr;
    }
    if (lightLabel.toLower().contains("sun"))
    {
        // change light type to sun and sky
        //outstr += "LightSource \"sun\"\n";
        //outstr += QString("\t\"float importance\"\t[%1]\n").arg(1);
        //outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        //outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity());
        outstr += QString("scene.lights.%1.type = \"sun\"\n").arg(lightLabel);
        outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel).arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("scene.lights.%1.gain = %2 %3 %4\n").arg(lightLabel).arg(lightColor.redF() * lightIntensity).arg(lightColor.greenF() * lightIntensity).arg(lightColor.blueF() * lightIntensity);
        YaLuxGlobal.bDefaultLightsOn = false;
    }
    if (lightLabel.toLower().contains("sky"))
    {
//        outstr += "LightSource \"sky2\"\n";
        //outstr += QString("\t\"float importance\"\t[%1]\n").arg(1);
        //outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        //outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity());
        outstr += QString("scene.lights.%1.type = \"sky2\"\n").arg(lightLabel+"_sky");
        outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel+"_sky").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("scene.lights.%1.gain = %2 %3 %4\n").arg(lightLabel+ "_sky").arg(lightColor.redF() * lightIntensity * 0.5).arg(lightColor.greenF() * lightIntensity * 0.5).arg(lightColor.blueF() * lightIntensity * 0.5);
        YaLuxGlobal.bDefaultLightsOn = false;
    }
    if (!(lightLabel.toLower().contains("sun")) && !(lightLabel.toLower().contains("sky")) && !(lightLabel.toLower().contains("infinite")))
    {
        YaLuxGlobal.bDefaultLightsOn = false;
        //// convert everything else to a mesh light
        DzMatrix3 mat3;
        DzMatrix4 mat4;
        mat3 = currentLight->getWSTransform(dzScene->getTime());
        mat4 = mat3.matrix4();
        //outstr += "Transform [";
        //outstr += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
        //outstr += QString("%1 %2 %3 %4 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);
        //outstr += QString("%1 %2 %3 %4 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
        //// row[3] needs to be scaled
        //outstr += QString("%1 %2 %3 %4 ").arg(mat4[3][0] / 100).arg(-mat4[3][2] / 100).arg(mat4[3][1] / 100).arg(mat4[3][3]);
        //outstr += "]\n";
        outstr += QString("scene.lights.%1.transformation = ").arg(lightLabel);
        outstr += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
        outstr += QString("%5 %6 %7 %8 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);
        outstr += QString("%9 %10 %11 %12 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
        outstr += QString("%13 %14 %15 %16\n").arg(mat4[3][0] / 100).arg(-mat4[3][2] / 100).arg(mat4[3][1] / 100).arg(mat4[3][3]);

        //outstr += "AreaLightSource \"area\"\n";
        lightPos = currentLight->getWSPos();
        //outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        //outstr += QString("\t\"float power\"\t[%1]\n").arg(100);
        //outstr += QString("\t\"float efficacy\"\t[%1]\n").arg(17);
        outstr += QString("scene.lights.%1.color = %2 %3 %4\n").arg(lightLabel).arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(lightIntensity);
        outstr += QString("scene.lights.%1.position = %2 %3 %4\n").arg(lightLabel).arg(lightPos.m_x).arg(lightPos.m_y).arg(lightPos.m_z);

        if (currentLight->inherits("DzSpotLight"))
        {
            //outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzSpotLight*)currentLight)->getIntensity());
            //outstr += spotLightPlane.join("");
            outstr += QString("scene.lights.%1.type = \"spot\"\n").arg(lightLabel);
            lightVector = currentLight->getFocalPoint();
            outstr += QString("scene.lights.%1.target = %2 %3 %4\n").arg(lightLabel).arg(lightVector.m_x).arg(lightVector.m_y).arg(lightVector.m_z);
        }
        else if (currentLight->inherits("DzPointLight")) {
            //outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzPointLight*)currentLight)->getIntensity());
            //outstr += "Shape \"sphere\" \"float radius\" [0.5]\n";
            outstr += QString("scene.lights.%1.type = \"point\"\n").arg(lightLabel);
        }
        else if (currentLight->inherits("DzDistantLight")) {
            //outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzDistantLight*)currentLight)->getIntensity());
            //outstr += distantLightPlane.join("");
            outstr += QString("scene.lights.%1.type = \"distant\"\n").arg(lightLabel);
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel).arg(lightVector.m_x).arg(lightVector.m_y).arg(lightVector.m_z);
        }
    }

    //outstr += "AttributeEnd\n";

    return outstr;
}

QString LuxCoreProcessObject(DzObject* daz_obj, QString& mesg)
{
    QString nodeAssetId;
    QString nodeLabel;
    DzGeometry* geo;
    DzShape* shape;
    QString outstr = "";
    QString attributeblock = "";

    if (daz_obj->getName().contains("Tonemapper Options"))
        return "";
    if (daz_obj->getName().contains("Environment Options"))
        return "";

    // Object -> Shape
    shape = daz_obj->getCurrentShape();
    QString shapeLabel = SanitizeCoreLabel(shape->getLabel());

    nodeLabel = SanitizeCoreLabel(shape->getNode()->getLabel());
    nodeAssetId = shape->getNode()->getAssetId();

    // Shape -> Geometry
    geo = daz_obj->getCachedGeom();
    QString geoLabel = SanitizeCoreLabel(QString("numvertices = %1").arg(geo->getNumVertices()));

    // DEBUG
    if (YaLuxGlobal.debugLevel > 2)
        mesg += QString("\tobject(node label) = [%1], shape = [%2], %3\n").arg(nodeLabel).arg(shapeLabel).arg(geoLabel);

    // Shape -> MaterialList
    QString matLabel;

    int numMaterials = shape->getNumMaterials();
    QObjectList texList;

    // TEXTURES 
    for (int i = 0; i < numMaterials; i++)
    {
        //attributeblock = "AttributeBegin\n";
        DzMaterial* material = shape->getAssemblyMaterial(i);
        if (material == NULL) continue;

        matLabel = SanitizeCoreLabel(material->getLabel());
        // DEBUG
//        if (YaLuxGlobal.debugLevel > 1)
//            mesg += QString("\t\tmaterial[%1] = getLabel[%2], getName[%3], getMaterialName[%4]").arg(i).arg(matLabel).arg(materialList[i]->getName()).arg(materialList[i]->getMaterialName()) ;
        //////////////////////////////////
        // Read the Daz material properties and generate the luxrender material block
        ///////////////////////////////////
        
        if (material->getMaterialName().toLower().contains("daz studio default"))
        {
            // go here
            attributeblock += LuxCoreProcessDazDefaultMaterial(material, mesg, nodeLabel + matLabel);
        }
        else if (material->getMaterialName().toLower().contains("omubersurface"))
        {
            attributeblock += LuxCoreProcessOmUberSurfaceMaterial(material, mesg, nodeLabel + matLabel);
        }
        else if (material->getMaterialName().toLower().contains("iray uber"))
        {
            attributeblock += LuxCoreProcessIrayUberMaterial(material, mesg, nodeLabel + matLabel);
        }
        else if (material->getMaterialName().toLower().contains("pbrskin"))
        {
            attributeblock += LuxCoreProcessIrayUberMaterial(material, mesg, nodeLabel + matLabel);
        }
        else
        {
            attributeblock += LuxCoreProcessDazDefaultMaterial(material, mesg, nodeLabel + matLabel);
        }

        // process related vertex group for this material
        QString objMatName = QString("%1_%2").arg(nodeLabel).arg(matLabel);

        ////////////////////////////////////////
        // Convert meshlight object to luxrender area light as appropriate
        ////////////////////////////////////////
        bool bIsAreaLight = false;
        QString lightTexture = "";
        QString strColor = "";
        float gain = 1;
        float efficacy = 17;
        float power = 100;
        float importance = 1;
        // Check to see if this is an area light material
        DzPropertyGroupTree* propertygrouptree = material->getPropertyGroups();
        if (matLabel.contains("RealityLight"))
        {
            strColor = LuxGetStringProperty(material, "Diffuse Color", mesg);
            LuxGetFloatProperty(material, "Diffuse Strength", gain, mesg);
            bIsAreaLight = true;
        }
        else if (propertygrouptree->findChild("LuxRender") != NULL)
        {
            //// LuxRender_matte_Kd (color)
            //// LuxRender_matte_sigma (float)
            // LuxRender_material_extrasettings (string)
            // LuxRender_material_opacity (float)
            // LuxRender_material_enablelight (bool)
            // LuxRender_light_L (color)
            // LuxRender_light_nsamples (int)
            // LuxRender_light_power (float)
            // LuxRender_light_efficacy (float)
            // LuxRender_light_importance (float)
            if (LuxGetStringProperty(material, "LuxRender_material_enablelight", mesg) == "true")
            {
                bIsAreaLight = true;
                strColor = LuxGetStringProperty(material, "LuxRender_light_L", mesg);
                LuxGetFloatProperty(material, "LuxRender_light_power", power, mesg);
                LuxGetFloatProperty(material, "LuxRender_light_efficacy", efficacy, mesg);
                LuxGetFloatProperty(material, "LuxRender_light_efficacy", importance, mesg);
            }

            // DEBUG
            if (YaLuxGlobal.debugLevel > 3)
                dzApp->log("yaluxplug: is Luxus area light? =[" + mesg + "]\n");
        }
        else if (propertygrouptree->findChild("Light") != NULL)
        {
            strColor = LuxGetStringProperty(material, "Color", mesg);
            LuxGetFloatProperty(material, "Intensity", gain, mesg);
            bIsAreaLight = true;
        }
        if (bIsAreaLight)
        {
            YaLuxGlobal.bDefaultLightsOn = false;
            //attributeblock += "AreaLightSource \"area\"\n";
            //attributeblock += QString("\t\"color L\"\t[%1]\n").arg(strColor);
            //attributeblock += QString("\t\"float gain\"\t[%1]\n").arg(strGain);
            //attributeblock += QString("\t\"float power\"\t[%1]\n").arg(strPower);
            //attributeblock += QString("\t\"float efficacy\"\t[%1]\n").arg(strEfficacy);

            if (lightTexture != "")
                attributeblock += QString("scene.materials.%1.emission = \"%2\"\n").arg(nodeLabel + matLabel).arg(lightTexture);
            else
                attributeblock += QString("scene.materials.%1.emission = %2\n").arg(nodeLabel + matLabel).arg(strColor);
            attributeblock += QString("scene.materials.%1.emission.gain = %2 %2 %2\n").arg(nodeLabel + matLabel).arg(gain);
            attributeblock += QString("scene.materials.%1.emission.power = %2\n").arg(nodeLabel + matLabel).arg(power);
            attributeblock += QString("scene.materials.%1.emission.efficency = %2\n").arg(nodeLabel + matLabel).arg(efficacy);
            attributeblock += QString("scene.materials.%1.emission.importance = %2\n").arg(nodeLabel + matLabel).arg(importance);

        }

        QString plyFileName;
        if (geo->inherits("DzFacetMesh"))
        {
            DazToPLY dzPLYexport((DzFacetMesh*)geo, objMatName, material);
            plyFileName = dzPLYexport.LuxMakeBinPLY();
        }
        if (plyFileName != "")
        {
            // add in reference to plyFileName
            //attributeblock += "Shape \"plymesh\"\n";
            //attributeblock += QString("\t\"string filename\" [\"%1\"]\n").arg(plyFileName);

            attributeblock += QString("scene.objects.%1.material = \"%2\"\n").arg(nodeLabel + matLabel).arg(nodeLabel + matLabel);
            attributeblock += QString("scene.objects.%1.shape = \"%2\"\n").arg(nodeLabel + matLabel).arg(nodeLabel + matLabel + "_shape");
            attributeblock += QString("scene.shapes.%1.type = \"mesh\"\n").arg(nodeLabel + matLabel + "_shape");
            attributeblock += QString("scene.shapes.%1.ply = \"%2\"\n").arg(nodeLabel + matLabel + "_shape").arg(plyFileName);

            //attributeblock += "AttributeEnd\n\n";
        }
        else {
            // no shapes to render for this material, just scrap this attribute block and move on
            attributeblock = "";
        }
        outstr += attributeblock;
    }

    return outstr;

}

QString LuxCoreProcessGlossyMaterial(DzMaterial* material, QString& mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // diffuse image and color
    float diffuse_vscale = -1;
    float diffuse_uscale = 1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset = 0; // vdelta
    float diffuse_uoffset = 0; // udelta
    QString diffuse_wrap = "repeat"; // repeat|black|clamp
    QString diffuse_filtertype = "bilinear";
    QString diffuse_channel = "";
    QString diffuse_mapfile = ""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists = false;

    // specular image and color
    float spec_vscale = -1;
    float spec_uscale = 1;
    float spec_gamma = 2.2;
    float spec_voffset = 0;
    float spec_uoffset = 0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel = "";
    QString spec_mapfile = ""; // Specular Color
    QColor spec_value;
    bool spec_exists = false;

    // bump image and values
    float bump_vscale = -1;
    float bump_uscale = 1;
    float bump_gamma = 1;
    float bump_voffset = 0;
    float bump_uoffset = 0;
    QString bump_channel = "";
    QString bump_wrap = "repeat";
    QString bump_filtertype = "bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists = false;

    // transmission map
    QString opacity_mapfile = "";
    float opacity_value = 1;
    bool opacity_exists = false;

    // material definition
    float uroughness = 0.8;
    float vroughness = 0.8;
    float index_refraction = 0.0; // IOR


    // 1. search for specific properties and populate data
    // 2. generate full material block

    QString propertyLabel;
    DzProperty* currentProperty;
    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((diffuse_value != 1) || (diffuse_mapfile != ""))
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((spec_value != 1) || (spec_mapfile != ""))
            spec_exists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 100;
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (bump_mapfile != "")
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((opacity_value != 1) || (opacity_mapfile != ""))
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;
    }

    // Diffuse Texture Block
    if (diffuse_exists)
        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
            diffuse_gamma, diffuse_wrap, diffuse_channel);

    // Specular Block
    if (spec_exists)
    {
        ret_str += GenerateCoreTextureBlock3(matLabel + "_s", spec_mapfile,
            spec_value.redF(), spec_value.greenF(), spec_value.blueF(),
            spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
            spec_wrap, spec_channel);

        //switch (YaLuxGlobal.specularMode)
        //{
        //case 0: // 90% Diffuse + 10% Specular
        //    ret_str += MixTextures(matLabel + ".specular_mix", matLabel + ".diffuse_color", matLabel + ".specular_color", "color", "\"float amount\" [0.1]");
        //    break;
        //case 1: // Specular * Glossiness
        //    ret_str += ScaleTexture(matLabel + ".specular_mix", matLabel + ".specular_color", QString("%1 %1 %1").arg(1 - uroughness), "color");
        //    break;
        //case 2: // (75% Diffuse + 25% Specular) * Glossiness
        //    ret_str += MixTextures(matLabel + ".specular_mix1", matLabel + ".diffuse_color", matLabel + ".specular_color", "color", "\"float amount\" [0.25]");
        //    ret_str += ScaleTexture(matLabel + ".specular_mix", matLabel + ".specular_mix1", QString("%1 %1 %1").arg(1 - uroughness), "color");
        //    break;
        //case 3: // 10% Specular
        //    ret_str += ScaleTexture(matLabel + ".specular_mix", matLabel + ".specular_color", "0.1 0.1 0.1", "color");
        //    break;
        //case 4: // Full Specular
        //    ret_str += ScaleTexture(matLabel + ".specular_mix", matLabel + ".specular_color", "1 1 1", "color");
        //    break;
        //case 5: // Specular off
        //        // .specular_mix == 0
        //    ret_str += GenerateTextureBlock(matLabel + ".specular_mix", "color", "", "0 0 0", 1, -1, 0, 0, 2.2, "", "", "");
        //    break;
        //}
    }

    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateCoreTextureBlock1(matLabel + "_b", bump_mapfile, bump_value,
            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
            bump_wrap, bump_channel);

    // Opacity Block
    if (opacity_exists && opacity_mapfile != "")
        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);

    // Material definition
    // decide what type of material...
    if (!opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);

        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(index_refraction);
    }

    if (opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string type\" [\"null\"]\n");
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        //ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        //ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel + ".base");

        ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(matLabel);
        if (opacity_mapfile != "")
            ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
        else
            ret_str += QString("scene.materials.%1.kt = %2 %2 %2\n").arg(matLabel).arg(opacity_value);
        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(index_refraction);
    }


    return ret_str;
}


QString LuxCoreProcessDazDefaultMaterial(DzMaterial* material, QString& mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // diffuse image and color
    float diffuse_vscale = -1;
    float diffuse_uscale = 1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset = 0; // vdelta
    float diffuse_uoffset = 0; // udelta
    QString diffuse_wrap = "repeat"; // repeat|black|clamp
    QString diffuse_filtertype = "bilinear";
    QString diffuse_channel = "";
    QString diffuse_mapfile = ""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists = false;

    // specular image and color
    float spec_vscale = -1;
    float spec_uscale = 1;
    float spec_gamma = 2.2;
    float spec_voffset = 0;
    float spec_uoffset = 0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel = "";
    QString spec_mapfile = ""; // Specular Color
    QColor spec_value;
    bool spec_exists = false;

    // bump image and values
    float bump_vscale = -1;
    float bump_uscale = 1;
    float bump_gamma = 1;
    float bump_voffset = 0;
    float bump_uoffset = 0;
    QString bump_channel = "";
    QString bump_wrap = "repeat";
    QString bump_filtertype = "bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists = false;

    // transmission map
    QString opacity_mapfile = "";
    float opacity_value = 1;
    bool opacity_exists = false;

    // material definition
    float uroughness = 0.8;
    float vroughness = 0.8;
    float index_refraction = 0.0; // IOR

    QString propertyLabel;
    DzProperty* currentProperty;

    // Multiply Specular Through Opacity
    bool bMultiplySpecularThroughOpacity = false;
    currentProperty = material->findProperty("Multiply Specular Through Opacity");
    if (currentProperty != NULL)
    {
        bMultiplySpecularThroughOpacity = ((DzBoolProperty*)currentProperty)->getValue(dzScene->getTime());
    }

    // material types
    enum { glossy, matte, plastic, metal } material_type = glossy;
    currentProperty = material->findProperty("Lighting Model");
    if (currentProperty != NULL)
    {
        QString lighting_model = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
        if (lighting_model.contains("glossy"))
        {
            material_type = glossy;
        }
        else if (lighting_model.contains("matte"))
        {
            material_type = matte;
        }
        else if (lighting_model.contains("plastic"))
        {
            material_type = plastic;
        }
        else if (lighting_model.contains("metal"))
        {
            material_type = metal;
        }
    }

    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((diffuse_value != 1) || (diffuse_mapfile != ""))
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((spec_value != 1) || (spec_mapfile != ""))
            spec_exists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 100;
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (bump_mapfile != "")
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((opacity_value != 1) || (opacity_mapfile != ""))
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;

        if (material_type == plastic)
        {
            uroughness += 0.5;
            uroughness = (uroughness > 1.0) ? 1.0 : uroughness;
            vroughness = uroughness;
        }

    }

    // Opacity Block
    if (opacity_exists && opacity_mapfile != "")
        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);

    // Diffuse Texture Block
    if (diffuse_exists)
        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
            diffuse_gamma, diffuse_wrap, diffuse_channel);

    // Specular Block
    if (spec_exists)
    {
        // check specular strength!!!
        QString realSpecularLabel = matLabel + "_s";
        bool bDoMixtureTexture = false;
        float spec_strength = 1.0;
        if ( bMultiplySpecularThroughOpacity || 
            ( LuxGetFloatProperty(material, "Specular Strength", spec_strength, mesg) && spec_strength < 1.0 ) )
        {
            bDoMixtureTexture = true;
            realSpecularLabel = matLabel + "_s" + "_0";
        }
        ret_str += GenerateCoreTextureBlock3(realSpecularLabel, spec_mapfile,
            spec_value.redF(), spec_value.greenF(), spec_value.blueF(),
            spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
            spec_wrap, spec_channel);
        if (bDoMixtureTexture)
        {
            QString specularScaleLabel = matLabel + "_s";
            if (bMultiplySpecularThroughOpacity)
            {
                specularScaleLabel = matLabel + "_s" + "_opacity0";
            }
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(specularScaleLabel);
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specularScaleLabel).arg(realSpecularLabel);
            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(specularScaleLabel).arg(spec_strength);
            if (bMultiplySpecularThroughOpacity)
            {
                ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(matLabel + "_s");
                ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(matLabel + "_s").arg(specularScaleLabel);
                if (opacity_exists && opacity_mapfile != "")
                    ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(matLabel + "_s").arg(matLabel + "_o");
                else
                    ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(matLabel + "_s").arg(opacity_value);
            }

        }
    }

    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateCoreTextureBlock1(matLabel + "_b", bump_mapfile, bump_value,
            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
            bump_wrap, bump_channel);


    // Material definition
    // decide what type of material...
    if (!opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);


        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(index_refraction);
    }

    if (opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string type\" [\"null\"]\n");
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        //ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        //ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel + ".base");

        // setup mix material
        QString realmatLabel = matLabel + "_0";
        QString nullmatLabel = matLabel + "_1";
        ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(nullmatLabel);

        //ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(realmatLabel);
        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(realmatLabel);

        //if (opacity_mapfile != "")
        //    ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_o");
        //else
        //    ret_str += QString("scene.materials.%1.kt = %2 %2 %2\n").arg(realmatLabel).arg(opacity_value);

        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(realmatLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(realmatLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(realmatLabel).arg(index_refraction);

        ret_str += QString("scene.materials.%1.type = \"mix\"\n").arg(matLabel);
        ret_str += QString("scene.materials.%1.material2 = \"%2\"\n").arg(matLabel).arg(realmatLabel);
        ret_str += QString("scene.materials.%1.material1 = \"%2\"\n").arg(matLabel).arg(nullmatLabel);

        if (opacity_mapfile != "")
            ret_str += QString("scene.materials.%1.amount = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
        else
            ret_str += QString("scene.materials.%1.amount = %2 %2 %2\n").arg(matLabel).arg(opacity_value);

    }


    return ret_str;
}


QString LuxCoreProcessOmUberSurfaceMaterial(DzMaterial* material, QString& mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // diffuse image and color
    float diffuse_vscale = -1;
    float diffuse_uscale = 1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset = 0; // vdelta
    float diffuse_uoffset = 0; // udelta
    QString diffuse_wrap = "repeat"; // repeat|black|clamp
    QString diffuse_filtertype = "bilinear";
    QString diffuse_channel = "";
    QString diffuse_mapfile = ""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists = false;

    // specular image and color
    float spec_vscale = -1;
    float spec_uscale = 1;
    float spec_gamma = 2.2;
    float spec_voffset = 0;
    float spec_uoffset = 0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel = "";
    QString spec_mapfile = ""; // Specular Color
    QColor spec_value;
    bool spec_exists = false;

    // bump image and values
    float bump_vscale = -1;
    float bump_uscale = 1;
    float bump_gamma = 1;
    float bump_voffset = 0;
    float bump_uoffset = 0;
    QString bump_channel = "";
    QString bump_wrap = "repeat";
    QString bump_filtertype = "bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists = false;

    // transmission map
    QString opacity_mapfile = "";
    float opacity_value = 1;
    bool opacity_exists = false;

    // material definition
    float uroughness = 0.8;
    float vroughness = 0.8;
    float index_refraction = 0.0; // IOR

    //if (material->getMaterialName().toLower().contains("daz studio default"))
    //{
    //    // go here
    //}

    // 1. search for specific properties and populate data
    // 2. generate full material block

    enum { glossy, matte, plastic, metal } material_type = glossy;

    QString propertyLabel;
    DzProperty* currentProperty;

    // Matte vs Glossy
    currentProperty = material->findProperty("Lighting Model");
    if (currentProperty != NULL)
    {
        QString lighting_model = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
        if (lighting_model.contains("glossy"))
        {
            material_type = glossy;
        }
        else if (lighting_model.contains("matte"))
        {
            material_type = matte;
        }
        else if (lighting_model.contains("plastic"))
        {
            material_type = plastic;
        }
        else if (lighting_model.contains("metal"))
        {
            material_type = metal;
        }
    }

    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((diffuse_value != 1) || (diffuse_mapfile != ""))
            diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((spec_value != 1) || (spec_mapfile != ""))
            spec_exists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 100;
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (bump_mapfile != "")
            bump_exists = true;
    }
    currentProperty = material->findProperty("eta"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((opacity_value != 1) || (opacity_mapfile != ""))
            opacity_exists = true;
    }
    currentProperty = material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (uroughness > 0.8) uroughness = 0.8;
        vroughness = uroughness;

        if (material_type == plastic)
        {
            uroughness += 0.5;
            uroughness = (uroughness > 1.0) ? 1.0 : uroughness;
            vroughness = uroughness;
        }

    }

    // Diffuse Texture Block
    if (diffuse_exists)
        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
            diffuse_gamma, diffuse_wrap, diffuse_channel);

    // Specular Block
    if (spec_exists)
    {
        // check specular strength!!!
        QString realSpecularLabel = matLabel + "_s";
        bool bDoMixtureTexture = false;
        float spec_strength = 1.0;

        // Always mix down by 25%
        LuxGetFloatProperty(material, "Specular Strength", spec_strength, mesg);
        realSpecularLabel = matLabel + "_s" + "_0";
        spec_strength *= 0.25;

        ret_str += GenerateCoreTextureBlock3(realSpecularLabel, spec_mapfile,
            spec_value.redF(), spec_value.greenF(), spec_value.blueF(),
            spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
            spec_wrap, spec_channel);

        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(matLabel + "_s");
        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(matLabel + "_s").arg(realSpecularLabel);
        ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(matLabel + "_s").arg(spec_strength);

    }

    // Bumpmap Block
    if (bump_exists)
        ret_str += GenerateCoreTextureBlock1(matLabel + "_b", bump_mapfile, bump_value,
            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
            bump_wrap, bump_channel);


    // Opacity Block
    if (opacity_exists && opacity_mapfile != "")
        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);

    // Material definition
    // decide what type of material...
    if (!opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);


        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(index_refraction);
    }

    if (opacity_exists)
    {
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        //if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        //if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        //if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
        //ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        //ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ////        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string type\" [\"null\"]\n");
        //ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        //ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
        //ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
        //ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel + ".NULL");
        //ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel + ".base");

        // setup mix material
        QString realmatLabel = matLabel + "_0";
        QString nullmatLabel = matLabel + "_1";
        ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(nullmatLabel);

        //ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(realmatLabel);
        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(realmatLabel);

        //if (opacity_mapfile != "")
        //    ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_o");
        //else
        //    ret_str += QString("scene.materials.%1.kt = %2 %2 %2\n").arg(realmatLabel).arg(opacity_value);

        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_d");
        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_s");
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_b");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(realmatLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(realmatLabel).arg(vroughness);
        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(realmatLabel).arg(index_refraction);

        ret_str += QString("scene.materials.%1.type = \"mix\"\n").arg(matLabel);
        ret_str += QString("scene.materials.%1.material2 = \"%2\"\n").arg(matLabel).arg(realmatLabel);
        ret_str += QString("scene.materials.%1.material1 = \"%2\"\n").arg(matLabel).arg(nullmatLabel);

        if (opacity_mapfile != "")
            ret_str += QString("scene.materials.%1.amount = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
        else
            ret_str += QString("scene.materials.%1.amount = %2 %2 %2\n").arg(matLabel).arg(opacity_value);

    }


    return ret_str;
}


QString LuxCoreProcessIrayUberMaterial(DzMaterial* material, QString& mesg, QString matLabel)
{
    QString ret_str = "# MATERIAL " + matLabel + "\n";

    // diffuse image and color
    float diffuse_vscale = -1;
    float diffuse_uscale = 1;
    float diffuse_gamma = 2.2;
    float diffuse_voffset = 0; // vdelta
    float diffuse_uoffset = 0; // udelta
    QString diffuse_wrap = "repeat"; // repeat|black|clamp
    QString diffuse_filtertype = "bilinear";
    QString diffuse_channel = "";
    QString diffuse_mapfile = ""; // Diffuse Color
    QColor diffuse_value;
    bool diffuse_exists = false;

    // specular image and color
    float spec_vscale = -1;
    float spec_uscale = 1;
    float spec_gamma = 2.2;
    float spec_voffset = 0;
    float spec_uoffset = 0;
    QString spec_wrap = "repeat"; // repeat|black|clamp
    QString spec_filtertype = "bilinear";
    QString spec_channel = "";
    QString specweight_mapfile = ""; // Specular [Dual Lobe Specular]
    bool spec_exists = false;

    // bump image and values
    float bump_vscale = -1;
    float bump_uscale = 1;
    float bump_gamma = 1;
    float bump_voffset = 0;
    float bump_uoffset = 0;
    QString bump_channel = "";
    QString bump_wrap = "repeat";
    QString bump_filtertype = "bilinear";
    QString bump_mapfile = ""; // "Bump Strength"
    float bump_value;
    bool bump_exists = false;

    // transmission map
    QString opacity_mapfile = "";
    float opacity_value = 1;
    bool opacity_exists = false;

    // material definition
    float uroughness = 0.8;
    float vroughness = 0.8;
    float index_refraction = 0.0; // IOR

    float metallic_weight = 0.0;
    QString metallic_mapfile = ""; // "Metallic Weight"

    float spec_weight = 0;
    float refraction_weight = 0;
    float translucency_weight = 0;
    float glossy_layered_weight = 0;

    // translucency
    QColor translucency_color;
    QString translucency_mapfile = ""; // "Translucency Color"
    bool translucency_exists = false;

    // normal map
    float normal_strength = 0; // "Normal Map"
    QString normal_mapfile = "";

    // more maps
    float glossy_roughness = 0;
    QString glossy_roughness_mapfile = "";
    float glossy_reflectivity = 0;

    // cheats
    float glossy_anisotropy = 0;


    enum { metal_roughness, specular_glossy } material_type = specular_glossy;

    QString propertyLabel;
    DzProperty* currentProperty;

    // Matte vs Glossy
    currentProperty = material->findProperty("Base Mixing");
    if (currentProperty != NULL)
    {
        QString base_mixing = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
        if (base_mixing.contains("metal"))
        {
            material_type = metal_roughness;
        }
        else if (base_mixing.contains("specular"))
        {
            material_type = specular_glossy;
        }
    }

    currentProperty = material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        diffuse_exists = true;
    }
    currentProperty = material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_uscale = diffuse_uscale;
        bump_uscale = diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = diffuse_vscale;
        bump_vscale = diffuse_vscale;
    }
    currentProperty = material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_uoffset = diffuse_uoffset;
        bump_uoffset = diffuse_uoffset;
    }
    currentProperty = material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
        spec_voffset = diffuse_voffset;
        bump_voffset = diffuse_voffset;
    }
    currentProperty = material->findProperty("Dual Lobe Specular Weight");
    if (currentProperty != NULL)
    {
        spec_weight = ((DzFloatProperty*)currentProperty)->getValue();
        specweight_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((spec_weight != 0) || (specweight_mapfile != ""))
            spec_exists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 200;
        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (bump_mapfile != "")
            bump_exists = true;
    }
    currentProperty = material->findProperty("Normal Map");
    if (currentProperty != NULL)
    {
        normal_strength = ((DzFloatProperty*)currentProperty)->getValue();
        normal_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    currentProperty = material->findProperty("Refraction Weight");
    if (currentProperty != NULL)
    {
        refraction_weight = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Refraction Index"); // index of refreaction
    if (currentProperty != NULL)
    {
        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Cutout Opacity"); // cutout opacity
    if (currentProperty != NULL)
    {
        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((opacity_value != 1) || (opacity_mapfile != ""))
            opacity_exists = true;
    }
    ////////  Glossy Layer //////////////
    currentProperty = material->findProperty("Glossy Layered Weight"); // glossy layered weight
    if (currentProperty != NULL)
    {
        glossy_layered_weight = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = material->findProperty("Glossy Reflectivity"); // glossy reflectivity
    if (currentProperty != NULL)
    {
        glossy_reflectivity = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
    }
    currentProperty = material->findProperty("Glossy Roughness"); // glossy roughness
    if (currentProperty != NULL)
    {
        glossy_roughness= ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
        glossy_roughness_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    if (material_type == metal_roughness)
    {
        currentProperty = material->findProperty("Metallic Weight"); // metallicity
        if (currentProperty != NULL)
        {
            metallic_weight = ((DzFloatProperty*)currentProperty)->getValue();
            metallic_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        }
    }
    if (material_type == metal_roughness)
    {
        currentProperty = material->findProperty("Glossy Roughness");
        if (currentProperty != NULL)
        {
            uroughness = 1*(1-glossy_layered_weight) + ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
            if (uroughness > 0.8) uroughness = 0.8;
            vroughness = uroughness;

            if (material_type == specular_glossy)
            {
                uroughness = (1 - uroughness > 0.8) ? 0.8 : (1 - uroughness);
                vroughness = uroughness;
            }

        }
    }
    else
    {
        currentProperty = material->findProperty("Glossiness");
        if (currentProperty != NULL)
        {
            uroughness = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
            uroughness = (1 - uroughness > 0.8) ? 0.8 : (1 - uroughness);
            vroughness = uroughness;

        }
    }
    currentProperty = material->findProperty("Glossy Anisotropy"); // index of refreaction
    if (currentProperty != NULL)
    {
        glossy_anisotropy = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
        if (glossy_anisotropy > 0)
        {
            if (material_type == metal_roughness)
            {
                uroughness = (0.8 * glossy_anisotropy) + (uroughness * (1 - glossy_anisotropy));
                vroughness = uroughness;
            }
            else if (material_type == specular_glossy)
            {
                uroughness = (uroughness * (1 - glossy_anisotropy));
                vroughness = uroughness;
            }
        }
    }
    currentProperty = material->findProperty("Translucency Weight"); 
    if (currentProperty != NULL)
    {
        translucency_weight = ((DzFloatProperty*)currentProperty)->getValue();
        if (translucency_weight != 0)
        {
            translucency_exists = true;
            currentProperty = material->findProperty("Translucency Color");
            if (currentProperty != NULL)
            {
                translucency_color = ((DzColorProperty*)currentProperty)->getColorValue();
                translucency_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
            }
        }
    }


    ///////////////////////////////////////////////
    // Special Handling for Wet / Moisture
    ///////////////////////////////////////////////
    bool isWet = false;
    QString checkString = material->getName().toLower();
    if (checkString.contains("cornea") || checkString.contains("eyemoisture") || checkString.contains("eyereflection"))
    {
//        isWet = true;
//        opacity_exists = true;
//        opacity_value = 0.0;
    }

    // Diffuse Texture Block
    if (diffuse_exists)
        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
            diffuse_gamma, diffuse_wrap, diffuse_channel);

    // Specular Block (DUAL LOBE)
    float spec1_float = 0;
    float spec2_float = 0;
    float spec_ratio = 0;
    float spec_reflectivity = 0;
    QString spec1_mapfile = "";
    QString spec2_mapfile = "";
    QString specratio_mapfile = "";
    QString specref_mapfile = "";

    QString mainSpec = matLabel + "_s";
    QString rawDualLobe = mainSpec + "_raw";
    QString spec1_label = rawDualLobe + "_spec1";
    QString spec2_label = rawDualLobe + "_spec2";
    QString specratio_label = rawDualLobe + "_specratio";

    if (spec_weight != 0)
    {

        LuxGetFloatProperty(material, "Specular Lobe 1 Roughness", spec1_float, mesg);
        spec1_mapfile = LuxGetImageMapProperty(material, "Specular Lobe 1 Roughness", mesg);
        LuxGetFloatProperty(material, "Specular Lobe 2 Roughness", spec2_float, mesg);
        spec2_mapfile = LuxGetImageMapProperty(material, "Specular Lobe 2 Roughness", mesg);
        LuxGetFloatProperty(material, "Dual Lobe Specular Ratio", spec_ratio, mesg);
        specratio_mapfile = LuxGetImageMapProperty(material, "Dual Lobe Specular Ratio", mesg);
        LuxGetFloatProperty(material, "Dual Lobe Specular Reflectivity", spec_reflectivity, mesg);

        // generate texture 1, 2, ratio
        if (spec1_mapfile != "")
            ret_str += GenerateCoreTextureBlock1(spec1_label, spec1_mapfile, spec1_float);
        if (spec2_mapfile != "")
            ret_str += GenerateCoreTextureBlock1(spec2_label, spec1_mapfile, spec2_float);
        if (specratio_mapfile != "")
            ret_str += GenerateCoreTextureBlock1(specratio_label, specratio_mapfile, spec_ratio);
        
        ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(rawDualLobe);
        if (spec1_mapfile != "")
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(rawDualLobe).arg(spec1_label);
        else
            ret_str += QString("scene.textures.%1.texture1 = %2\n").arg(rawDualLobe).arg(spec1_float);
        if (spec2_mapfile != "")
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(rawDualLobe).arg(spec2_label);
        else
            ret_str += QString("scene.textures.%1.texture1 = %2\n").arg(rawDualLobe).arg(spec2_float);
        if (specratio_mapfile != "")
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(rawDualLobe).arg(specratio_label);
        else
            ret_str += QString("scene.textures.%1.texture1 = %2\n").arg(rawDualLobe).arg(spec_ratio);

        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(mainSpec);
        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(mainSpec).arg(rawDualLobe);
        if (specweight_mapfile != "")
        {
            ret_str += GenerateCoreTextureBlock1(mainSpec + "_weight", specweight_mapfile, spec_weight);
            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(mainSpec).arg(mainSpec + "_weight");
        }
        else
        {
            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(mainSpec).arg(spec_weight);
        }

    }

    // Bumpmap Block
    QString bumpMapName = matLabel + "_b";
    if (bump_exists)
        ret_str += GenerateCoreTextureBlock1(bumpMapName, bump_mapfile, bump_value,
            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
            bump_wrap, bump_channel);

    // Normalmap Block
    QString normalMapName = matLabel + "_n";
    QString imageMapName = normalMapName + "_t";
    if (normal_mapfile != "")
    {
        float scale = normal_strength * 1.0; // Multiply by any necessary render engine here

        ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(imageMapName);
        ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(imageMapName).arg(normal_mapfile);

        ret_str += QString("scene.textures.%1.type = \"normalmap\"\n").arg(normalMapName);
        ret_str += QString("scene.textures.%1.texture = \"%2\"\n").arg(normalMapName).arg(imageMapName);
        ret_str += QString("scene.textures.%1.scale = \"%2\"\n").arg(normalMapName).arg(scale);
    }

    // TranslucencyMap Block
    if (translucency_exists)
        ret_str += GenerateCoreTextureBlock3(matLabel + "_kt", translucency_mapfile,
            translucency_color.redF(), translucency_color.greenF(), translucency_color.blueF());


    // Opacity Block
    opacity_value = (1 - refraction_weight > opacity_value) ? opacity_value : 1 - refraction_weight;
    if (opacity_exists && opacity_mapfile != "")
        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);


    ///////////////////////////////////////////
    //
    // Material definition
    //
    ///////////////////////////////////////////

    // Planning:
    // 1. Metallic Layer
    // 2. Specular Layer
    // 3. Dual Specular Layer
    // 4. Top Coat Layer? Component?
    // 5. Translucency?? Volume?? Component
    // 6. Glossy Component

//    if (!opacity_exists && refraction_weight == 0)
    if (false)
    {
        // if type == metal, use glossy roughness
        // else use glossiness
        // if metallicity == 0, glossy2 only
        // if metallicity == 1, metal2 only
        // else glossy2 + metal2

        // Step 1a. Make glossy2
        // Step 1b. Make metal2
        // Step 1c. Make Glossy component
        // Step 1d. Make Translucency? Volume?
        // Step 2. Mix glossy2 + metal2
        // Step 3. Mix in Dual Spec
        // Step 4. Mix in Top Coat?
        // Step 5. Mix with opacity / null

        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
        if (spec_exists) 
            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        else
            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(matLabel);
        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
//        if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(matLabel).arg(matLabel + "_n");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
        if (metallic_weight != 0 || metallic_mapfile != "") ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(metallic_weight);
    }
    else
    {
//        // setup mix material
//        QString realmatLabel = matLabel + "_0";
//
//        if (refraction_weight != 0)
//        {
//            ret_str += QString("scene.materials.%1.type = \"glass\"\n").arg(realmatLabel);
//            ret_str += QString("scene.materials.%1.interiorior = %2\n").arg(realmatLabel).arg(index_refraction);
//        }
////        else if (translucency_weight != 0)
//        else if (false)
//        {
//            ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(realmatLabel);
////            ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_kt");
//        }
//        else
//        {
//            ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(realmatLabel);
//        }
//
//        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_d");
//        if (spec_exists)
//            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_s");
//        else
//            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(realmatLabel);
//
//        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_b");
////        if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_n");
//        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(realmatLabel).arg(uroughness);
//        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(realmatLabel).arg(vroughness);
//        if (metallic_weight != 0 || metallic_mapfile != "") ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(realmatLabel).arg(metallic_weight);

        if (metallic_weight == 0)
        {
            ////////////////////
            // Step 1a. Make glossy2
            ////////////////////
            QString glossy2Label = matLabel;
            ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(glossy2Label);
            ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_d");
            if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_b");
            //if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_n");
            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(uroughness);
            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(vroughness);
            // ****** use specular reflectivity and skip this *************
    //        ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(glossy2Label).arg("*****TODO: REFLECTIVE*******");

        ////////////////////
        // Step 1c. Make glossy/reflective component
        ////////////////////

        ////////////////////
        // Step 1d. Make translucent/volume component
        ////////////////////

        //////////////////////
        //// Step 2. Mix glossy2 + metal2
        //////////////////////
        //QString glossy_metal_MixLabel = matLabel + "glossy_metal_mix";
        //ret_str += QString("scene.materials.%1.type = \"mix\"\n").arg(glossy_metal_MixLabel);
        //ret_str += QString("scene.materials.%1.material1 = \"%2\"\n").arg(glossy_metal_MixLabel).arg(glossy2Label);
        //ret_str += QString("scene.materials.%1.material2 = \"%2\"\n").arg(glossy_metal_MixLabel).arg(metal2Label);
        //ret_str += QString("scene.materials.%1.amount = %2 %2 %2\n").arg(glossy_metal_MixLabel).arg(metallic_weight);

        ////////////////////
        // Step 3. Mix Dual Spec
        ////////////////////

        ////////////////////
        // Step 4. Mix Top Coat
        ////////////////////

        ////////////////////
        // Step 5. Mix Opacity
        ////////////////////
        //ret_str += QString("scene.materials.%1.type = \"mix\"\n").arg(matLabel);
        //ret_str += QString("scene.materials.%1.material1 = \"%2\"\n").arg(matLabel).arg(glossy2Label);
        //ret_str += QString("scene.materials.%1.material2 = \"%2\"\n").arg(matLabel).arg(glossy2Label);
        //ret_str += QString("scene.materials.%1.amount = 1 1 1\n").arg(matLabel);
        //if (opacity_mapfile != "")
        //    ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
        //else
        //    ret_str += QString("scene.materials.%1.transparency = %2 %2 %2\n").arg(matLabel).arg(opacity_value);

        if (opacity_mapfile != "")
            ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
        else
            ret_str += QString("scene.materials.%1.transparency = %2 %2 %2\n").arg(matLabel).arg(opacity_value);

        }
        else
        {
            ////////////////////
            // Step 1b. Make metal2
            ////////////////////
//            QString metal2Label = matLabel + "_metal2";
            QString metal2Label = matLabel;

            // applp scale to diffuse
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(matLabel + "_d" + "_s");
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(matLabel + "_d");
            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(matLabel + "_d" + "_s").arg(10);

            QString fresnel_texture = metal2Label + "_fresnel";
            ret_str += QString("scene.textures.%1.type = \"fresnelcolor\"\n").arg(fresnel_texture);
            ret_str += QString("scene.textures.%1.kr = \"%2\"\n").arg(fresnel_texture).arg(matLabel + "_d" + "_s");

            ret_str += QString("scene.materials.%1.type = \"metal2\"\n").arg(metal2Label);
            ret_str += QString("scene.materials.%1.fresnel = \"%2\"\n").arg(metal2Label).arg(fresnel_texture);
            if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(metal2Label).arg(matLabel + "_b");
            //if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(metal2Label).arg(matLabel + "_n");
            /////////////// use glossy_roughness map if available
            if (glossy_roughness_mapfile != "")
            {
                QString glossyRoughnessTexLabel = metal2Label + "_glossyRoughnessTex";
                ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(glossyRoughnessTexLabel);
                ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(glossyRoughnessTexLabel).arg(glossy_roughness_mapfile);

                ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(metal2Label).arg(glossyRoughnessTexLabel);
                ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(metal2Label).arg(glossyRoughnessTexLabel);
            }
            else
            {
                ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(metal2Label).arg(uroughness);
                ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(metal2Label).arg(vroughness);
            }
            // **** glossy reflectivity *******
    //        ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(metal2Label).arg("*****TODO: REFLECTIVE*******");
            //// *********** scale metal2 by metallicity *************

            if (opacity_mapfile != "")
                ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
            else
                ret_str += QString("scene.materials.%1.transparency = %2 %2 %2\n").arg(matLabel).arg(opacity_value);

        }



    }


    return ret_str;
}



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
            realtextureName = textureName + "_0";
            scaletextureName = textureName + "_1";
            ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(scaletextureName);
            ret_str += QString("scene.textures.%1.value = %2 %3 %4\n").arg(scaletextureName).arg(textureValue1).arg(textureValue2).arg(textureValue3);
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
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(textureName);
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(textureName).arg(realtextureName);
            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(textureName).arg(scaletextureName);
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

        if (bMixTextures)
        {
            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(textureName);
            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(textureName).arg(realtextureName);
            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(textureName).arg(textureValue);
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

#include "moc_utility_classes.cpp"
