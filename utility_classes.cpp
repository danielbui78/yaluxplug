//
//  utility_classes.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 5/5/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#include <stdio.h>

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
#include "dzmatrix4.h"

#include "utility_classes.h"
#include "dazToPLY.h"
#include "plugin.h"

struct G YaLuxGlobal;

void WorkerPrepareImage::doPrepareImage()
{
    // 1. do a lookup to see if there is a cached texture
    // 2. if true, then return the img and filename of the cached texture
    // 3. if not, then continue
    // 4. request a new img texture
    // 5. convert filename into the new texture
    // 6. cache the new texture and put it in the lookup table
    //      6.1 generate filename
    //      6.2 place in lookup table
    // 7. emit signal and return
    
    QImage qimg, qImgScaled;
    
    // DEBUG
    if (qimg.load(filename) == false)
    {
        // DEBUG
        // issue error and cancel
        dzApp->log("yaluxplug: Worker prepareImage Error - couldn't save scaled image");
        emit prepareImageComplete(this, img, filename);
    }
    
    // resize
    int ResizeWidth = 2000;
    if (qimg.width() > ResizeWidth) {
        qImgScaled = qimg.scaledToWidth(ResizeWidth);
    }
    
    // make cached name
    QString newName = DzFileIO::getBaseFileName(filename);
    newName += ".png";
    
    if ( qImgScaled.save( YaLuxGlobal.cachePath+newName ) )
    {
        dzApp->log("yaluxplug: Worker prepareImage( " + filename + " ) changed to PNG" );
        //        DzTexture *tex = imgMgr->getImage(filename);
        //        img->setTempFilename(YaLuxGlobal.cachePath + newName);
        emit prepareImageComplete(this, img, YaLuxGlobal.cachePath + newName);
    }
    else 
    {
        dzApp->log("yaluxplug: Worker prepareImage Error - couldn't save scaled image");
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
        if ( obj->inherits(classNames[i]) )
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


QString propertyNumericImagetoString(DzNumericProperty *prop)
{
    DzTexture *propTex;
    QString ret_str, newFileName;
    QSize size;
    QImage qimg, scaledImg;
    
    if ( ((DzNumericProperty*)prop)->isMappable() )
    {
        // get image name
        propTex = prop->getMapValue();
        if (propTex != NULL)
        {
            ret_str = propTex->getTempFilename();
            if (ret_str.contains(YaLuxGlobal.cachePath)==false)
            {
                // image may not be prepped. Create one now if needed
                ret_str = propTex->getFilename();
                size = propTex->getOriginalImageSize();
                if (size.width() > 2000)
                {
                    // yes, we do need to prepare image
                    qimg.load(ret_str);
                    scaledImg = qimg.scaledToWidth(2000);
                    newFileName = YaLuxGlobal.cachePath + DzFileIO::getBaseFileName(ret_str) + ".png";
                    scaledImg.save(newFileName);
                    propTex->setTempFilename(newFileName);
                    return newFileName;
                }
            }
        }
        else
            ret_str= "";
    }    
    
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
        ret_str += QString("\t\"float uscale\" [%1]\n").arg(uscale);
        ret_str += QString("\t\"float vscale\" [%1]\n").arg(vscale);    
        ret_str += QString("\t\"float udelta\" [%1]\n").arg(uoffset);
        ret_str += QString("\t\"float vdelta\" [%1]\n").arg(voffset);
        ret_str += QString("\t\"float gamma\" [%1]\n").arg(gamma);
        ret_str += QString("\t\"string wrap\" [\"%1\"]\n").arg(wrap);
        ret_str += QString("\t\"string filtertype\" [\"%1\"]\n").arg(filtertype);
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


QString LuxProcessLight(DzLight *currentLight, QString &mesg)
{
    QString ret_str;

    ret_str = LuxProcessProperties( (DzElement*) currentLight, mesg);
    dzApp->log(ret_str);
    ret_str = "";

    return ret_str;
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
    DzTexture *currentTex;
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
        attributeblock += LuxProcessMaterial(materialList[i], mesg, matLabel);
        
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
    QString ret_str;
    
    DzNodeListIterator nodeList = parentNode->nodeChildrenIterator();
    
    while (nodeList.hasNext() )
    {
        currentNode = nodeList.next();
        label = currentNode->getAssetId();
        
        //Process Node
        //YaLuxGlobal.currentNode = currentNode;
        //YaLuxGlobal.settings = new DzRenderSettings(this, &opt);
        dzApp->log( QString("yaluxplug: Looking at %1->%2: ").arg(parentLabel).arg(label));
        
        // Node -> Object
        DzObject *currentObject = currentNode->getObject();
        if (currentObject != NULL)
        {
            //outLXS.write("\n# " + label +"\n");
            ret_str += "\n# " + parentLabel + "." + label +"\n";
            QString objectLabel = currentObject->getLabel();
            
            // Object -> Shape
            DzShape *currentShape = currentObject->getCurrentShape();
            QString shapeLabel = currentShape->getName();
            
            // Shape -> Geometry
            DzGeometry *currentGeometry = currentShape->getGeometry();
            QString geoLabel = QString("numvertices = %1").arg(currentGeometry->getNumVertices() );
            
            dzApp->log( QString("\tobject = [%1], shape = [%2], %3").arg(parentLabel+"."+label+"."+objectLabel).arg(shapeLabel).arg(geoLabel) ) ;
            
            //LuxMakeTextureList(currentShape);
            mesg = LuxMakeMaterialList(currentShape);
            //outLXS.write(mesg);
            ret_str += mesg;
            
        } else {
            dzApp->log("\tno object found.");
        }
        
        mesg = "Properties for node = " + parentLabel + "." + label + "\n";
        LuxProcessProperties(currentNode, mesg);
        dzApp->log(mesg);
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
        
        //dzApp->log("yaluxplug: Calling Node->render() # " + parentLabel + "." + label);
        //currentNode->render(*YaLuxGlobal.settings);
        //outLXS.flush();    
        
    }
    
    return ret_str;
}


QString LuxProcessMaterial(DzMaterial *material, QString &mesg, QString matLabel)
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
    float diffuse_gamma =1;
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
    float spec_gamma =1;
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
        spec_uscale = 1/diffuse_uscale;
        bump_uscale = 1/diffuse_uscale;
    }
    currentProperty = material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
        spec_vscale = 1/diffuse_vscale;
        bump_vscale = 1/diffuse_vscale;
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
        if ( (bump_value != 1) || (bump_mapfile != "") )
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
    
    // Diffuse Texture Block
    if ( diffuse_exists )
        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile, 
                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()), 
                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset, 
                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);
    
    // Specular Block
    if (spec_exists)
        ret_str += GenerateTextureBlock(matLabel + ".specular_color", "color", spec_mapfile, 
                                        QString("%1 %2 %3").arg(spec_value.redF()).arg(spec_value.greenF()).arg(spec_value.blueF()), 
                                        spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma, 
                                        spec_wrap, spec_filtertype, spec_channel);
    
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
        ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_color");    
        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
    }
    
    if ( opacity_exists )
    {
        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
        ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
        ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_color");    
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

QString LuxProcessObject(DzObject *daz_obj)
{
    QString objectLabel;
    DzGeometry *geo;
    DzShape *shape;
    
    // Object -> Shape
    shape = daz_obj->getCurrentShape();
    QString shapeLabel = shape->getLabel();
    
    objectLabel = shape->getNode()->getLabel();
    
    // Shape -> Geometry
    geo = daz_obj->getCachedGeom();
    QString geoLabel = QString("numvertices = %1").arg(geo->getNumVertices() );
    
    dzApp->log( QString("\tobject = [%1], shape = [%2], %3").arg(objectLabel).arg(shapeLabel).arg(geoLabel) ) ;
    
    // Shape -> MaterialList
    DzMaterialPtrList materialList;
    QString matLabel;
    shape->getAllRenderPrioritizedMaterials(materialList);
    int i = 0;
    QString mesg = "";
    QObjectList texList;
    DzTexture *currentTex;
    QString outstr = "";
    QString attributeblock = "";
    // TEXTURES 
    while (i < materialList.count() )
    {
        attributeblock = "AttributeBegin\n";
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
        matLabel = materialList[i]->getLabel();
        attributeblock += LuxProcessMaterial(materialList[i], mesg, matLabel);
        
        // process related vertex group for this material
        QString objMatName = QString("%1.%2").arg(objectLabel).arg(matLabel);
        
        QString plyFileName;
        if (geo->inherits("DzFacetMesh"))
        {
            DazToPLY dzPLYexport((DzFacetMesh *)geo, objMatName, materialList[i]);
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
        i++;
    }
    //    dzApp->log(mesg);
    
    return outstr;
    
    
}

