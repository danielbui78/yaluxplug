#include "DazToLuxRender.h"

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
        ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName + "map").arg(textureType).arg("imagemap");
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
        ret_str += QString("\t\"texture tex1\" [\"%1\"]\n").arg(textureName + "map");
        ret_str += QString("\t\"%1 tex2\" [%2]\n").arg(textureType).arg(textureValue);
    }
    else {
        ret_str += QString("Texture \"%1\" \"%2\" \"%3\"\n").arg(textureName).arg(textureType).arg("constant");
        ret_str += QString("\t\"%1 value\" [%2]\n").arg(textureType).arg(textureValue);
    }

    return ret_str;

}

QString LuxProcessLight(DzLight* currentLight, QString& mesg)
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
    LuxProcessProperties((DzElement*)currentLight, mesg);
    int lux_light_type;
    if (LuxGetIntProperty((DzElement*)currentLight, "LuxRender_light_type", lux_light_type, mesg) == true)
    {
        outstr = "\nAttributeBegin\n";
        switch (lux_light_type)
        {
        case 6: // sky2
            outstr += "LightSource \"sky2\"\n";
            outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));
            outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
            outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
            break;
        case 8: // Sun
            outstr += "LightSource \"sun\"\n";
            outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));
            outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
            outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
            outstr += QString("\t\"float relsize\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
            break;
        case 9: // Sun & sky2
            outstr += "LightSource \"sun\"\n";
            outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_gain", mesg));
            outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_nsamples", mesg));
            outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_turbidity", mesg));
            outstr += QString("\t\"float relsize\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sun_relsize", mesg));
            outstr += "LightSource \"sky2\"\n";
            outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));
            outstr += QString("\t\"integer nsamples\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
            outstr += QString("\t\"float turbidity\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
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
            outstr += QString("\t\"float importance\"\t[%1]\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_importance", mesg));
            outstr += QString("%1\n").arg(LuxGetStringProperty(currentLight, "LuxRender_light_extrasettings", mesg));
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
    if (currentLight->getAssetId().contains("Image Based Light"))
    {
        QColor lightColor = currentLight->getDiffuseColor();
        outstr += "LightSource \"infinite\"\n";
        outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        QString mapname = propertyNumericImagetoString((DzNumericProperty*)currentLight->findProperty("Color"));
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
        outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity());

        YaLuxGlobal.bDefaultLightsOn = false;

    }
    if (lightLabel.contains("Sky"))
    {
        outstr += "LightSource \"sky2\"\n";
        outstr += QString("\t\"float importance\"\t[%1]\n").arg(1);
        outstr += QString("\t\"vector sundir\"\t[%1 %2 %3]\n").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("\t\"float gain\"\t[%1]\n").arg(0.0005 * ((DzDistantLight*)currentLight)->getIntensity());
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
        outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzDistantLight*)currentLight)->getIntensity());
        YaLuxGlobal.bDefaultLightsOn = false;
    }
    if (!(lightLabel.contains("Sun")) && !(lightLabel.contains("Sky")) && !(lightLabel.contains("Infinite")))
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
        outstr += QString("%1 %2 %3 %4 ").arg(mat4[3][0] / 100).arg(-mat4[3][2] / 100).arg(mat4[3][1] / 100).arg(mat4[3][3]);
        outstr += "]\n";

        outstr += "AreaLightSource \"area\"\n";
        QColor lightColor = currentLight->getDiffuseColor();
        outstr += QString("\t\"color L\"\t[%1 %2 %3]\n").arg(lightColor.redF()).arg(lightColor.greenF()).arg(lightColor.blueF());
        outstr += QString("\t\"float power\"\t[%1]\n").arg(100);
        outstr += QString("\t\"float efficacy\"\t[%1]\n").arg(17);

        if (currentLight->inherits("DzSpotLight"))
        {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzSpotLight*)currentLight)->getIntensity());
            outstr += spotLightPlane.join("");

        }
        else if (currentLight->inherits("DzPointLight")) {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzPointLight*)currentLight)->getIntensity());
            outstr += "Shape \"sphere\" \"float radius\" [0.5]\n";

        }
        else if (currentLight->inherits("DzDistantLight")) {
            outstr += QString("\t\"float gain\"\t[%1]\n").arg(((DzDistantLight*)currentLight)->getIntensity());
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

QString LuxMakeSHAPE(DzFacetMesh* mesh, QString meshName)
{
    int numFaces;
    int numVerts;
    int numNormals;
    int numUVpts;
    DzPnt3* vertexList;
    DzPnt3* normalsList;
    DzFacet* facesList;
    DzPnt2* uvList;

    QString ret_str = "";
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
    float* ptrF;
    ret_str += "\t\"point P\" [";
    while (i < numVerts)
    {
        ptrF = (float*)&vertexList[i];
        vert_str += QString("%1 %2 %3 ").arg(ptrF[0] / 100).arg(-ptrF[2] / 100).arg(ptrF[1] / 100);
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    ret_str += "\t\"normal N\" [";
    vert_str = "";
    i = 0;
    while (i < numNormals)
    {
        ptrF = (float*)&normalsList[i];
        vert_str += QString("%1 %2 %3 ").arg(ptrF[0]).arg(-ptrF[2]).arg(ptrF[1]);
        i++;
    }
    ret_str += vert_str;
    ret_str += "]\n";
    ret_str += "\t\"float uv\" [";
    vert_str = "";
    i = 0;
    while (i < numUVpts)
    {
        ptrF = (float*)&uvList[i];
        vert_str += QString("%1 %2 ").arg(ptrF[0]).arg(ptrF[1]);
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


QString LuxMakeTextureList(DzShape* currentShape)
{
    // Shape -> MaterialList
    DzMaterialPtrList materialList;
    QString matLabel;
    currentShape->getAllRenderPrioritizedMaterials(materialList);
    int i = 0;
    QString mesg;
    DzTexture* currentTex;
    DzDefaultMaterial* currentMat;
    QString outstr = "";
    QString materialDef = "";
    // TEXTURES 
    while (i < materialList.count())
    {
        materialDef += " # MATERIAL\n";
        outstr += " # TEXTURES\n";
        matLabel = materialList[i]->getLabel();
        mesg += "  # MATERIAL -" + matLabel + "\n";

        materialDef += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
        materialDef += "\t\"string type\" [\"glossy\"]\n";
        // named maps
        // diffuse
        currentTex = materialList[i]->getColorMap();
        if (currentTex != NULL)
        {
            outstr += LuxMakeSingleTexture(currentTex, matLabel + ".diffuse_map", "color");
            // add respective line to Material Definition
            materialDef += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_map");
        }
        if (materialList[i]->inherits("DzDefaultMaterial"))
        {
            DzMaterial* matptr = materialList[i];
            currentMat = (DzDefaultMaterial*)matptr;
            currentTex = currentMat->getSpecularColorMap();
            if (currentTex != NULL)
            {
                outstr += LuxMakeSingleTexture(currentTex, matLabel + ".specular_map", "color");
                // add respective line to Material Definition
                materialDef += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_map");
            }
            currentTex = currentMat->getBumpMap();
            if (currentTex != NULL)
            {
                outstr += LuxMakeSingleTexture(currentTex, matLabel + ".bump_map", "float");
                // add respective line to Material Definition
                materialDef += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_map");
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

//QString LuxMakeMaterialList(DzShape *currentShape)
//{
//    // Shape -> MaterialList
//    DzMaterialPtrList materialList;
//    QString matLabel;
//    currentShape->getAllRenderPrioritizedMaterials(materialList);
//    int i = 0;
//    QString mesg = "";
//    QObjectList texList;
//
//    QString outstr = "";
//    QString attributeblock = "";
//    // TEXTURES 
//    while (i < materialList.count() )
//    {
//        attributeblock = "AttributeBegin\n";
//        
//        DzMatrix3 mat3 = (currentShape->getNode())->getWSTransform();
//        DzMatrix4 mat4 = mat3.matrix4();
//        attributeblock += "Transform [";
//        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(-mat4[0][2]).arg(mat4[0][1]).arg(mat4[0][3]);
//        // row[1] and row[2] are switched
//        attributeblock += QString("%1 %2 %3 %4 ").arg(-mat4[2][0]).arg(mat4[2][2]).arg(-mat4[2][1]).arg(mat4[2][3]);
//        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[1][0]).arg(-mat4[1][2]).arg(mat4[1][1]).arg(mat4[1][3]);        
//        // row[3] needs to be scaled
//        attributeblock += QString("%1 %2 %3 %4 ").arg(mat4[3][0]/100).arg(-mat4[3][2]/100).arg(mat4[3][1]/100).arg(mat4[3][3]);
//        attributeblock += "]\n";
//        
//        //        matLabel = materialList[i]->getMaterialName();
//        QString shapeLabel = currentShape->getNode()->getAssetId();
//        matLabel = materialList[i]->getLabel();
//        attributeblock += LuxProcessGlossyMaterial(materialList[i], mesg, matLabel);
//        
//        // process related shapes for this material
//        DzGeometry *mesh = currentShape->getGeometry();
//        QString meshName = QString("%1.%2").arg(shapeLabel).arg( matLabel );
//        
//        QString plyFileName;
//        if (mesh->inherits("DzFacetMesh"))
//        {
//            DazToPLY dzPLYexport((DzFacetMesh *)mesh, meshName, materialList[i]);
//            plyFileName = dzPLYexport.LuxMakeAsciiPLY();
//            //            plyFileName = dzPLYexport.LuxMakeBinPLY();
//            //            plyFileName = LuxMakePLY((DzFacetMesh*)mesh, meshName, materialList[i]);
//            //            outstr += LuxMakeSHAPE( (DzFacetMesh*)mesh, meshName);
//        }
//        if (plyFileName != "")
//        {   
//            // add in reference to plyFileName
//            attributeblock += "Shape \"plymesh\"\n";
//            attributeblock += QString("\t\"string filename\" [\"%1\"]\n").arg(plyFileName);
//            
//            attributeblock += "AttributeEnd\n\n";
//        } else {
//            // no shapes to render for this material, just scrap this attribute block and move on
//            attributeblock = "";
//        }
//        outstr += attributeblock;
//        i++;
//    }
//    //    dzApp->log(mesg);
//    
//    return outstr;
//    
//}

//QString LuxProcessMatteMaterial(DzMaterial *material, QString &mesg, QString matLabel)
//{
//    QString ret_str = "# MATERIAL " + matLabel + "\n";
//
//    // process all material information and output a full material LXS description block
//
//
//    // Lux Texture format
//    // Texture "NAME" color|float|spectrum "TYPE" <values>
//    // TYPE:
//    //  imagemap
//    //  mix
//    //  scale (mulitply with color)
//    // PARAMS
//    //  mapping = [uv|spherical|cylindrical|planar]
//    //  uscale/vscale = [float] scale texcoords
//    //  udelta/vdelta = [float] offfset tex mappings
//    //  v1/v2 = [vector] : define the plane for planar mapping
//    //  constant::'value' = [color|float]
//    //  scale::'tex1'/'tex2' = [color|float] (multiply 2 textures)
//    //  imagemap::'filename' = [string] (with path)
//    //  imagemap::'wrap' = [string] (repeat|black|clamp)
//    //  imagemap::'filtertype' = [string] (bilinear|mipmap_trilinear|mipmap_ewa|nearest
//    //  imagemap::'channel' = [string] (mean|red|green|blue|alpha|colored_mean)
//
//    // Lux Material format
//    //  bumpmap = [float texture]
//    //  matte::Kd = [color texture]
//    //  matte::sigma = [float texture] (sigma of Oren-Nayer shader in degrees)
//    // --- translucent matte ----
//    //  matte::Kr = [color texture] (reflectivity)
//    //  matte::Kt = [color texture] (transmissivity)
//
//    //  glossy::Kd = [color texture] (diffuse)
//    //  glossy::Ks = [color texture] (specular)
//    //  glossy::Ka = [color texture] (absorption)
//    //  glossy::uroughness = [float texture] 0 to 1 (roughness in u direction)
//    //  glossy::vroughness = [float texture] 0 to 1 (... in v direction)
//    //  glossy::d = [float texture] (depth of absorption effects. 0=disable)
//    //  glossy::index = [float texture] (IOR of coating)
//
//    // diffuse image and color
//    float diffuse_vscale =-1;
//    float diffuse_uscale =1;
//    float diffuse_gamma = 2.2;
//    float diffuse_voffset =0; // vdelta
//    float diffuse_uoffset =0; // udelta
//    QString diffuse_wrap ="repeat"; // repeat|black|clamp
//    QString diffuse_filtertype ="bilinear";
//    QString diffuse_channel ="";
//    QString diffuse_mapfile =""; // Diffuse Color
//    QColor diffuse_value;
//    bool diffuse_exists =false;
//
//    // specular image and color
//    float spec_vscale =-1;
//    float spec_uscale =1;
//    float spec_gamma = 2.2;
//    float spec_voffset =0;
//    float spec_uoffset =0;
//    QString spec_wrap = "repeat"; // repeat|black|clamp
//    QString spec_filtertype = "bilinear";
//    QString spec_channel ="";
//    QString spec_mapfile =""; // Specular Color
//    QColor spec_value;
//    bool spec_exists =false;
//
//    // bump image and values
//    float bump_vscale =-1;
//    float bump_uscale =1;
//    float bump_gamma =1;
//    float bump_voffset =0;
//    float bump_uoffset =0;
//    QString bump_channel ="";
//    QString bump_wrap ="repeat";
//    QString bump_filtertype ="bilinear";
//    QString bump_mapfile = ""; // "Bump Strength"
//    float bump_value;
//    bool bump_exists =false;
//
//    // transmission map
//    QString opacity_mapfile ="";
//    float opacity_value =1;
//    bool opacity_exists =false;
//
//    // material definition
//    float uroughness=0.8;
//    float vroughness=0.8;
//    float index_refraction; // IOR
//
//
//    // 1. search for specific properties and populate data
//    // 2. generate full material block
//
//    QString propertyLabel;
//    DzProperty *currentProperty;
//    currentProperty = material->findProperty("Diffuse Color");
//    if (currentProperty != NULL)
//    {
//        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( (diffuse_value != 1) || (diffuse_mapfile != "") )
//            diffuse_exists = true;
//    }
//    currentProperty = material->findProperty("Horizontal Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_uscale = 1/((DzFloatProperty*)currentProperty)->getValue();
//        spec_uscale = diffuse_uscale;
//        bump_uscale = diffuse_uscale;
//    }
//    currentProperty = material->findProperty("Vertical Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
//        spec_vscale = diffuse_vscale;
//        bump_vscale = diffuse_vscale;
//    }
//    currentProperty = material->findProperty("Horizontal Offset");
//    if (currentProperty != NULL)
//    {
//        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
//        spec_uoffset = diffuse_uoffset;
//        bump_uoffset = diffuse_uoffset;
//    }
//    currentProperty = material->findProperty("Vertical Offset");
//    if (currentProperty != NULL)
//    {
//        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
//        spec_voffset = diffuse_voffset;
//        bump_voffset = diffuse_voffset;
//    }
//
//    currentProperty = material->findProperty("Bump Strength");
//    if (currentProperty != NULL)
//    {
//        bump_value = ((DzFloatProperty*)currentProperty)->getValue();
//        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( bump_mapfile != "" )
//            bump_exists = true;
//    }
//    currentProperty = material->findProperty("eta"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
//        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( (opacity_value != 1) || (opacity_mapfile != "") )
//            opacity_exists = true;
//    }
//    currentProperty = material->findProperty("Glossiness");
//    if (currentProperty != NULL)
//    {
//        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
//        if (uroughness > 0.8) uroughness = 0.8;
//        vroughness = uroughness;
//    }
//
//    // Diffuse Texture Block
//    if ( diffuse_exists )
//        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile,
//                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()),
//                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);
//
//
//    // Bumpmap Block
//    if (bump_exists)
//        ret_str += GenerateTextureBlock(matLabel + ".bump_texture", "float", bump_mapfile, QString("%1").arg(bump_value),
//                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                                        bump_wrap, bump_filtertype, bump_channel);
//
//    // Opacity Block
//    if ( opacity_exists )
//        ret_str += GenerateTextureBlock(matLabel + ".opacity_texture", "float", opacity_mapfile, QString("%1").arg(opacity_value),
//                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                                        bump_wrap, bump_filtertype, bump_channel);
//
//    // Material definition
//    // decide what type of material...
//    if ( !opacity_exists )
//    {
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
//        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
//        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
//    }
//
//    if ( opacity_exists )
//    {
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
//        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
//        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
//
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
//        ret_str += QString("\t\"string type\" [\"null\"]\n");
//
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
//        ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
//        ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel+".NULL");
//        ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel+".base");
//    }
//    
//    ret_str += QString("NamedMaterial \"%1\"\n").arg(matLabel);
//    
//    return ret_str;
//}

//QString LuxProcessGenMaterial(DzMaterial *material, QString &mesg, QString matLabel)
//{
//    QString ret_str = "# MATERIAL " + matLabel + "\n";
//    
//    // process all material information and output a full material LXS description block
//    
//    
//    // Lux Texture format
//    // Texture "NAME" color|float|spectrum "TYPE" <values>
//    // TYPE:
//    //  imagemap
//    //  mix
//    //  scale (mulitply with color)
//    // PARAMS
//    //  mapping = [uv|spherical|cylindrical|planar]
//    //  uscale/vscale = [float] scale texcoords
//    //  udelta/vdelta = [float] offfset tex mappings
//    //  v1/v2 = [vector] : define the plane for planar mapping
//    //  constant::'value' = [color|float]
//    //  scale::'tex1'/'tex2' = [color|float] (multiply 2 textures)
//    //  imagemap::'filename' = [string] (with path)
//    //  imagemap::'wrap' = [string] (repeat|black|clamp)
//    //  imagemap::'filtertype' = [string] (bilinear|mipmap_trilinear|mipmap_ewa|nearest
//    //  imagemap::'channel' = [string] (mean|red|green|blue|alpha|colored_mean)
//    
//    // Lux Material format
//    //  bumpmap = [float texture]
//    //  matte::Kd = [color texture]
//    //  matte::sigma = [float texture] (sigma of Oren-Nayer shader in degrees)
//    // --- translucent matte ----
//    //  matte::Kr = [color texture] (reflectivity)
//    //  matte::Kt = [color texture] (transmissivity)
//    
//    //  glossy::Kd = [color texture] (diffuse)
//    //  glossy::Ks = [color texture] (specular)
//    //  glossy::Ka = [color texture] (absorption)
//    //  glossy::uroughness = [float texture] 0 to 1 (roughness in u direction)
//    //  glossy::vroughness = [float texture] 0 to 1 (... in v direction)
//    //  glossy::d = [float texture] (depth of absorption effects. 0=disable)
//    //  glossy::index = [float texture] (IOR of coating)
//    
//    // diffuse image and color
//    float diffuse_vscale =-1;
//    float diffuse_uscale =1;
//    float diffuse_gamma = 2.2;
//    float diffuse_voffset =0; // vdelta
//    float diffuse_uoffset =0; // udelta
//    QString diffuse_wrap ="repeat"; // repeat|black|clamp
//    QString diffuse_filtertype ="bilinear";
//    QString diffuse_channel ="";
//    QString diffuse_mapfile =""; // Diffuse Color
//    QColor diffuse_value;
//    bool diffuse_exists =false;
//    
//    // specular image and color
//    float spec_vscale =-1;
//    float spec_uscale =1;
//    float spec_gamma = 2.2;
//    float spec_voffset =0;
//    float spec_uoffset =0;
//    QString spec_wrap = "repeat"; // repeat|black|clamp
//    QString spec_filtertype = "bilinear";
//    QString spec_channel ="";
//    QString spec_mapfile =""; // Specular Color
//    QColor spec_value;
//    bool spec_exists =false;
//    
//    // bump image and values
//    float bump_vscale =-1;
//    float bump_uscale =1;
//    float bump_gamma =1;
//    float bump_voffset =0;
//    float bump_uoffset =0;
//    QString bump_channel ="";
//    QString bump_wrap ="repeat";
//    QString bump_filtertype ="bilinear";
//    QString bump_mapfile = ""; // "Bump Strength"
//    float bump_value;
//    bool bump_exists =false;
//    
//    // transmission map
//    QString opacity_mapfile ="";
//    float opacity_value =1;
//    bool opacity_exists =false;
//    
//    // material definition
//    float uroughness=0.8;
//    float vroughness=0.8;
//    float index_refraction; // IOR
//    
//    
//    // 1. search for specific properties and populate data
//    // 2. generate full material block
//    
//    QString propertyLabel;
//    DzProperty *currentProperty;
//    currentProperty = material->findProperty("Diffuse Color");
//    if (currentProperty != NULL)
//    {
//        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( (diffuse_value != 1) || (diffuse_mapfile != "") )
//            diffuse_exists = true;
//    }
//    currentProperty = material->findProperty("Horizontal Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_uscale = 1/((DzFloatProperty*)currentProperty)->getValue();
//        spec_uscale = diffuse_uscale;
//        bump_uscale = diffuse_uscale;
//    }
//    currentProperty = material->findProperty("Vertical Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_vscale = -1/((DzFloatProperty*)currentProperty)->getValue();
//        spec_vscale = diffuse_vscale;
//        bump_vscale = diffuse_vscale;
//    }
//    currentProperty = material->findProperty("Horizontal Offset");
//    if (currentProperty != NULL)
//    {
//        diffuse_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
//        spec_uoffset = diffuse_uoffset;
//        bump_uoffset = diffuse_uoffset;
//    }
//    currentProperty = material->findProperty("Vertical Offset");
//    if (currentProperty != NULL)
//    {
//        diffuse_voffset = ((DzFloatProperty*)currentProperty)->getValue();
//        spec_voffset = diffuse_voffset;
//        bump_voffset = diffuse_voffset;
//    }
//    currentProperty = material->findProperty("Specular Color");
//    if (currentProperty != NULL)
//    {
//        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( (spec_value != 1) || (spec_mapfile != "") )
//            spec_exists = true;
//    }
//    currentProperty = material->findProperty("Bump Strength");
//    if (currentProperty != NULL)
//    {
//        bump_value = ((DzFloatProperty*)currentProperty)->getValue();
//        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( bump_mapfile != "" )
//            bump_exists = true;
//    }
//    currentProperty = material->findProperty("eta"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Opacity Strength"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
//        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ( (opacity_value != 1) || (opacity_mapfile != "") )
//            opacity_exists = true;
//    }
//    currentProperty = material->findProperty("Glossiness");
//    if (currentProperty != NULL)
//    {
//        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
//        if (uroughness > 0.8) uroughness = 0.8;
//        vroughness = uroughness;
//    }
//
//    // Diffuse Texture Block
//    if ( diffuse_exists )
//        ret_str += GenerateTextureBlock(matLabel + ".diffuse_color", "color", diffuse_mapfile, 
//                                        QString("%1 %2 %3").arg(diffuse_value.redF()).arg(diffuse_value.greenF()).arg(diffuse_value.blueF()),
//                                        diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset, 
//                                        diffuse_gamma, diffuse_wrap, diffuse_filtertype, diffuse_channel);
//    
//    // Specular Block
//    if (spec_exists)
//    {
//        ret_str += GenerateTextureBlock(matLabel + ".specular_color", "color", spec_mapfile, 
//                                        QString("%1 %2 %3").arg(spec_value.redF()).arg(spec_value.greenF()).arg(spec_value.blueF()),
//                                        spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
//                                        spec_wrap, spec_filtertype, spec_channel);
//
//        switch (YaLuxGlobal.specularMode)
//        {
//        case 0: // 90% Diffuse + 10% Specular
//            ret_str += MixTextures(matLabel+".specular_mix", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.1]");
//            break;
//        case 1: // Specular * Glossiness
//            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", QString("%1 %1 %1").arg(1-uroughness), "color");
//            break;
//        case 2: // (75% Diffuse + 25% Specular) * Glossiness
//            ret_str += MixTextures(matLabel+".specular_mix1", matLabel+".diffuse_color", matLabel+".specular_color", "color", "\"float amount\" [0.25]");
//            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_mix1", QString("%1 %1 %1").arg(1-uroughness), "color");
//            break;
//        case 3: // 10% Specular
//            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "0.1 0.1 0.1", "color");
//            break;
//        case 4: // Full Specular
//            ret_str += ScaleTexture(matLabel+".specular_mix", matLabel+".specular_color", "1 1 1", "color");
//            break;
//        case 5: // Specular off
//            // .specular_mix == 0
//            ret_str += GenerateTextureBlock(matLabel+".specular_mix", "color", "", "0 0 0", 1, -1, 0, 0, 2.2, "", "", "");
//            break;
//        }
//
//
//    }
//
//    // Bumpmap Block
//    if (bump_exists)
//        ret_str += GenerateTextureBlock(matLabel + ".bump_texture", "float", bump_mapfile, QString("%1").arg(bump_value),
//                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                                        bump_wrap, bump_filtertype, bump_channel);
//    
//    // Opacity Block
//    if ( opacity_exists )
//        ret_str += GenerateTextureBlock(matLabel + ".opacity_texture", "float", opacity_mapfile, QString("%1").arg(opacity_value),
//                                        bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                                        bump_wrap, bump_filtertype, bump_channel);
//    
//    // Material definition
//    // decide what type of material...
//    if ( !opacity_exists )
//    {
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
//        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
//        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
//        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
//        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
//        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
//        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
//    }
//    
//    if ( opacity_exists )
//    {
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".base");
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("glossy");
//        if (bump_exists) ret_str += QString("\t\"texture bumpmap\" [\"%1\"]\n").arg(matLabel + ".bump_texture");
//        if (diffuse_exists) ret_str += QString("\t\"texture Kd\" [\"%1\"]\n").arg(matLabel + ".diffuse_color");
//        if (spec_exists) ret_str += QString("\t\"texture Ks\" [\"%1\"]\n").arg(matLabel + ".specular_mix");
//        ret_str += QString("\t\"float uroughness\" [%1]\n").arg(uroughness);
//        ret_str += QString("\t\"float vroughness\" [%1]\n").arg(vroughness);
//        ret_str += QString("\t\"float index\" [%1]\n").arg(index_refraction);
//        
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel + ".NULL");
//        ret_str += QString("\t\"string type\" [\"null\"]\n");
//        
//        ret_str += QString("MakeNamedMaterial \"%1\"\n").arg(matLabel);
//        ret_str += QString("\t\"string type\" [\"%1\"]\n").arg("mix");
//        ret_str += QString("\t\"texture amount\" [\"%1\"]\n").arg(matLabel + ".opacity_texture");
//        ret_str += QString("\t\"string namedmaterial1\" [\"%1\"]\n").arg(matLabel+".NULL");
//        ret_str += QString("\t\"string namedmaterial2\" [\"%1\"]\n").arg(matLabel+".base");
//    }
//    
//    ret_str += QString("NamedMaterial \"%1\"\n").arg(matLabel);
//    
//    return ret_str;
//}

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
        bump_value = ((DzFloatProperty*)currentProperty)->getValue()/1000;
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
//
//

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
//
//

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

        //// DEBUG
        //if ( YaLuxGlobal.debugLevel >= 2 ) // debug data
        //{
        //    mesg = "Properties for node = " + nodeAssetId + "\n";
        //    LuxProcessProperties(currentNode, mesg);
        //    dzApp->log(mesg);
        //    mesg = "";

        //    // Process child nodes
        //    QString output;
        //    dzApp->log("**Processing child nodes for " + nodeAssetId + QString(" (%1 children total)").arg(currentNode->getNumNodeChildren()) + "***" );
        //    output = processChildNodes(currentNode, mesg, nodeAssetId);
        //    dzApp->log(mesg);

        //    if (output != "")
        //        outLXS.write(output.toAscii());
        //}

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

bool DazToLuxRender::WriteRenderFiles()
{
    return true;
}

