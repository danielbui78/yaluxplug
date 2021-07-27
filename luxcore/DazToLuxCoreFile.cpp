#include "DazToLuxCoreFile.h"

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

#include "DazDefaultToLuxCoreMaterial.h"
#include "IrayUberToLuxCoreMaterial.h"
#include "OmUberSurfaceToLuxCoreMaterial.h"
#include "PBRSkinToLuxCoreMaterial.h"

QString LuxCoreProcessObject(DzObject* daz_obj, QString& mesg)
{
    QString nodeAssetId;
    QString nodeLabel;
    DzFacetMesh* mesh;
    DzShape* shape;
    QString outstr = "";
    QString attributeblock = "";

    if (daz_obj->getName().contains("Tonemapper Options"))
        return "";
    if (daz_obj->getName().contains("Environment Options"))
        return "";

    // Object -> Shape
    shape = daz_obj->getCurrentShape();
    if (shape)
    {
        DzNode* node = shape->getNode();
        daz_obj->update(*node, true);
        daz_obj->finalize(*node, true, true);
    }
    QString shapeLabel = SanitizeCoreLabel(shape->getLabel());

    nodeLabel = SanitizeCoreLabel(shape->getNode()->getLabel());
    nodeAssetId = shape->getNode()->getAssetId();

    // Shape -> Geometry
    mesh = qobject_cast<DzFacetMesh*>(daz_obj->getCachedGeom());
    QString geoLabel = SanitizeCoreLabel(QString("numvertices = %1").arg(mesh->getNumVertices()));

    // DEBUG
    if (YaLuxGlobal.debugLevel > 2)
        mesg += QString("\tobject(node label) = [%1], shape = [%2], %3\n").arg(nodeLabel).arg(shapeLabel).arg(geoLabel);

    // Shape -> MaterialList
    QString matLabel;

    QObjectList texList;
//    int numMaterials = shape->getNumMaterials();
    int numMaterials = shape->getNumAssemblyMaterials();
    // TEXTURES 
    for (int i = 0; i < numMaterials; i++)
    {
        //attributeblock = "AttributeBegin\n";
        DzMaterial* material = shape->getAssemblyMaterial(i);
        if (material == NULL) continue;

        // process related vertex group for this material
        matLabel = SanitizeCoreLabel(material->getLabel());
        QString objMatName = QString("%1_%2").arg(nodeLabel).arg(matLabel);

        QString plyFileName;
        DazToPLY dzPLYexport(mesh, objMatName, material, i);
        plyFileName = dzPLYexport.LuxMakeBinPLY();
        if (plyFileName == "") continue;

        // DEBUG
//        if (YaLuxGlobal.debugLevel > 1)
//            mesg += QString("\t\tmaterial[%1] = getLabel[%2], getName[%3], getMaterialName[%4]").arg(i).arg(matLabel).arg(materialList[i]->getName()).arg(materialList[i]->getMaterialName()) ;

//////////////////////////////////
        // Read the Daz material properties and generate the luxrender material block
        // ****** ADD LOOKUP TABLE TO REMOVE DUPLICATES **********
        ///////////////////////////////////

        // 1. de-Identify material data to be independent of matLabel / material name
        // 2. generate hash
        // 3. do lookup table, skip if present
        // 4. else add to lookup table
        QString token = nodeLabel + matLabel;
        if (YaLuxGlobal.matLookupTable.contains(token))
        {
            continue;
        }
        else
        {
            YaLuxGlobal.matLookupTable.append(token);
        }

        if (material->getMaterialName().toLower().contains("daz studio default"))
        {
//            attributeblock += LuxCoreProcessDazDefaultMaterial(material, mesg, nodeLabel + matLabel);
            attributeblock += DazDefaultToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
        }
        else if (material->getMaterialName().toLower().contains("omubersurface"))
        {
//            attributeblock += LuxCoreProcessOmUberSurfaceMaterial(material, mesg, nodeLabel + matLabel);
            attributeblock += OmUberSurfaceToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
        }
        else if (material->getMaterialName().toLower().contains("iray uber"))
        {
//            attributeblock += LuxCoreProcessIrayUberMaterial(material, mesg, nodeLabel + matLabel);
            attributeblock += IrayUberToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
        }
        else if (material->getMaterialName().toLower().contains("pbrskin"))
        {
//            attributeblock += LuxCoreProcessIrayUberMaterial(material, mesg, nodeLabel + matLabel);
            attributeblock += PBRSkinToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
        }
        else
        {
//            attributeblock += LuxCoreProcessDazDefaultMaterial(material, mesg, nodeLabel + matLabel);
            attributeblock += DazDefaultToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
        }

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

        // sanity check
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
        attributeblock = "";
    }

    return outstr;

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
        lightIntensity = dynamic_cast<DzDistantLight*>(currentLight)->getIntensity() * scale * 5.0; // conversion factor == ~5
    }

    // Check to see if luxrender settings are present
    //LuxProcessProperties((DzElement*)currentLight, mesg);
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
            outstr += QString("scene.lights.%1.type = \"sky2\"\n").arg(lightLabel + "_sky");
            outstr += QString("scene.lights.%1.turbidity = %2\n").arg(lightLabel + "_sky").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_turbidity", mesg));
            outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel + "_sky").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);

            LuxGetFloatProperty(currentLight, "LuxRender_light_sky2_gain", floatval, mesg);
            outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel + "_sky").arg(floatval * 1000);
            //outstr += QString("scene.lights.%1.gain = %2 %2 %2\n").arg(lightLabel).arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_gain", mesg));

            outstr += QString("scene.lights.%1.samples = %2\n").arg(lightLabel + "_sky").arg(LuxGetStringProperty(currentLight, "LuxRender_light_sky2_nsamples", mesg));
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
        QString mapname = propertyNumericImagetoString((DzNumericProperty*)currentLight->findProperty("Color"));
        if (mapname != "")
        {
            outstr += QString("scene.lights.%1.type = \"infinite\"\n").arg(lightLabel);
            outstr += QString("scene.lights.%1.file = \"%2\"\n").arg(lightLabel).arg(mapname);
            outstr += QString("scene.lights.%1.gain = %2 %3 %4\n").arg(lightLabel).arg(lightColor.redF() * lightIntensity).arg(lightColor.greenF() * lightIntensity).arg(lightColor.blueF() * lightIntensity);
            YaLuxGlobal.bDefaultLightsOn = false;

            return outstr;
        }
        else
        {
            lightLabel = lightLabel.toLower().replace("infinite", "distant");
        }

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
        outstr += QString("scene.lights.%1.type = \"sky2\"\n").arg(lightLabel + "_sky");
        outstr += QString("scene.lights.%1.dir = %2 %3 %4\n").arg(lightLabel + "_sky").arg(-lightVector.m_x).arg(lightVector.m_z).arg(-lightVector.m_y);
        outstr += QString("scene.lights.%1.gain = %2 %3 %4\n").arg(lightLabel + "_sky").arg(lightColor.redF() * lightIntensity * 0.5).arg(lightColor.greenF() * lightIntensity * 0.5).arg(lightColor.blueF() * lightIntensity * 0.5);
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

bool LuxMakeSCNFile(QString filenameSCN, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt)
{
    // Initialize material Lookup Table
    YaLuxGlobal.matLookupTable.clear();
    // Initialize volume list
    while (YaLuxGlobal.VolumeList.isEmpty() == false)
    {
        VolumeData* item = YaLuxGlobal.VolumeList.last();
        YaLuxGlobal.VolumeList.removeLast();
        delete(item);
    }

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
    if (environment && !environment->isHidden())
    {
        // check environment mode: "Environment Mode"
        enum {dome_and_scene, dome_only, sun_sky_only, scene_only } environment_mode_enum = dome_and_scene;
        QString environment_mode_string = "";
        DzProperty *environment_mode_property = environment->findProperty("Environment Mode");
        if (environment_mode_property != NULL)
        {
            if (environment_mode_property->inherits("DzEnumProperty"))
            {
                environment_mode_string = ((DzEnumProperty*)environment_mode_property)->getStringValue().toLower();
            }
        }
        if (environment_mode_string.contains("dome and scene")) environment_mode_enum = dome_and_scene;
        if (environment_mode_string.contains("dome only")) environment_mode_enum = dome_only;
        if (environment_mode_string.contains("sun-sky only")) environment_mode_enum = sun_sky_only;
        if (environment_mode_string.contains("scene only")) environment_mode_enum = scene_only;

        // Dome Mode (enum: 6)
        // Draw Dome (bool)
        // Environment Intensity (float)
        // Environment Map (float/image)
        // Environment Tint (color)
        // Environment Lighting Resolution (int)
        // Environment Lighting Blur (bool)
        // Dome Scale Multiplier (float)
        // Dome Radius (float)
        // Dome Origin X (float)
        // Dome Orientation X (float)
        // Dome Rotation (float)

        if (environment_mode_enum == dome_and_scene || environment_mode_enum == dome_only)
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
                //mat4.rotateZ( (floatval + 165) * 0.0174533); // convert degree to radian
                mat4[0][0] *= -1;
                mat4.rotateZ((-floatval) * 0.0174533); // convert degree to radian
                outstr += QString("scene.lights.environment_options.transformation = ");
                outstr += QString("%1 %2 %3 %4 ").arg(mat4[0][0]).arg(mat4[0][1]).arg(mat4[0][2]).arg(mat4[0][3]);
                outstr += QString("%5 %6 %7 %8 ").arg(mat4[1][0]).arg(mat4[1][1]).arg(mat4[1][2]).arg(mat4[1][3]);
                outstr += QString("%9 %10 %11 %12 ").arg(mat4[2][0]).arg(mat4[2][1]).arg(mat4[2][2]).arg(mat4[2][3]);
                outstr += QString("%13 %14 %15 %16\n").arg(mat4[3][0]).arg(mat4[3][1]).arg(mat4[3][2]).arg(mat4[3][3]);
            }
            outSCN.write(outstr.toAscii());
        }

        ///////
        // Iray Ground
        ///////

        // Ground Texture Scale (float)
        // Ground Position Mode (enum: auto, manual)
        // Ground Origin X (float)
        // Draw Ground (bool)
        // Ground Reflectivity (color)
        // Ground Glossiness (float)
        // Ground Shadow Intensity (float)
        // Ground Visible from Below (bool)

        // 1. If DrawGround,
        // 2. get ground origin x, y, z
        // set material color to ?
        // 3. set material reflectivity
        // 4. set roughness to 1-glossiness
        // shadow intensity??
        // 5. Draw ground

        ///////
        // Matte Fog
        //////
        // Matte Fog (bool)
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


    // SHARED TEXTURES AND MATERIALS
    QString sharedMaterials;
    QString noise_image = "4096-noise.png";
    if (YaLuxGlobal.maxTextureSize <= 1024)
        noise_image = "1024-noise.png";
    else if (YaLuxGlobal.maxTextureSize <= 2048)
        noise_image = "2048-noise.png";
    QString path = DzFileIO::getFilePath(YaLuxGlobal.LuxExecPath);

    QString noise_mask = "shared_material_noise_mask";
    sharedMaterials += QString("scene.textures.%1.type = \"imagemap\"\n").arg(noise_mask);
    sharedMaterials += QString("scene.textures.%1.file = \"%2\"\n").arg(noise_mask).arg(path + "/" + noise_image);

    outSCN.write(sharedMaterials.toAscii());



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

//        if (currentObject == NULL || currentNode->isHidden() || !currentNode->isVisible())
        if (currentObject == NULL || !currentNode->isVisible())
        {
            if (currentObject == NULL)
                dzApp->log("\tno object found.");
            else if (!currentNode->isVisible())
                dzApp->log("\tobject is not visible.");
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
        if (currentNode->inherits("DzFigure"))
        {
            DzFigure* figure = dynamic_cast<DzFigure*>(currentNode);
            if (figure->isGraftFollowing())
            {
                DzSkeleton* target = figure->getFollowTarget();
                if (target && target->isVisible() && target->isVisibileInRender())
                {
                    /// DB (2021-07-27)
                    /// Fixed broken programmer logic and re-enabled.
                    dzApp->log("yaluxplug: DEBUG: skipping geograft node: " + currentNode->getName());
                    continue;
                }

            }

        }

        outSCN.write(QString("\n# AssetId=[" + nodeAssetId + "],nodeLabel=[" + currentNode->getLabel() + "]\n").toAscii());
        QString objectLabel = currentObject->getLabel();

        QString output;
        output = LuxCoreProcessObject(currentObject, mesg);
        outSCN.write(output.toAscii());


        //// DEBUG
        //if (YaLuxGlobal.debugLevel >= 2) // debug data
        //{
        //    mesg = "Properties for node = " + nodeAssetId + "\n";
        //    LuxProcessProperties(currentNode, mesg);
        //    dzApp->log(mesg);
        //    mesg = "";

        //    // Process child nodes
        //    QString output;
        //    dzApp->log("**Processing child nodes for " + nodeAssetId + QString(" (%1 children total)").arg(currentNode->getNumNodeChildren()) + "***");
        //    //            output = processChildNodes(currentNode, mesg, nodeAssetId);
        //    dzApp->log(mesg);

        //    //            if (output != "")
        //    //                outSCN.write(output.toAscii());
        //}

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

bool LuxMakeCFGFile(QString filenameCFG, DzRenderer* r, DzCamera* camera, const DzRenderOptions& opt)
{
    int numLogicalCores = QThread::idealThreadCount();
    if (numLogicalCores == -1) numLogicalCores = 1;
    int concurrentThreads = 1;
    switch (numLogicalCores)
    {
    case 1:
        concurrentThreads = 1;
        break;
    case 2:
    case 3:
    case 4:
        concurrentThreads = numLogicalCores - 1;
        break;
    case 5:
    case 6:
    case 7:
        concurrentThreads = numLogicalCores - 2;
        break;
    case 8:
    default:
        concurrentThreads = numLogicalCores * 0.75;
    }

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
            mesg += QString("native.threads.count = %1\n").arg(concurrentThreads);
//            mesg += "path.hybridbackforward.enable = 0\n";
            break;
        case 1: // Hybrid (Native CPU + OpenCL GPU)
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 0\n";
            mesg += "opencl.gpu.use = 1\n";
            mesg += QString("opencl.native.threads.count = %1\n").arg(concurrentThreads);
//            mesg += "path.hybridbackforward.enable = 0\n";
//            mesg += "path.hybridbackforward.partition = 0.0\n";
            break;
        case 2: // OpenCL GPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 0\n";
            mesg += "opencl.gpu.use = 1\n";
            mesg += "opencl.native.threads.count = 0\n";
            break;
        case 3: // OpenCL CPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 32\n";
            mesg += "opencl.gpu.use = 0\n";
            mesg += "opencl.native.threads.count = 0\n";
            break;
        case 4: // OpenCL GPU + OpenCL CPU
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 0\n";
            mesg += "opencl.gpu.use = 1\n";
            mesg += "opencl.native.threads.count = 0\n";
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
    if (tonemapper && !tonemapper->isHidden())
    {
        // If "Tone Mapping Enable" property does not exist, then default to true
        bool tonemapping_enabled = true;
        LuxGetBoolProperty(tonemapper, "Tone Mapping Enable", tonemapping_enabled, mesg);
        if (tonemapping_enabled)
        {
            float floatval = 0;
            YaLuxGlobal.LuxToneMapper = "linear";
            if (LuxGetFloatProperty(tonemapper, "Film ISO", floatval, mesg))
                YaLuxGlobal.tonemapISO = floatval;
            if (LuxGetFloatProperty(tonemapper, "Gamma", floatval, mesg))
                YaLuxGlobal.tonemapGamma = floatval;
            if (LuxGetFloatProperty(tonemapper, "Shutter Speed", floatval, mesg))
                YaLuxGlobal.tonemapExposureTime = 1 / (floatval > 0 ? floatval : 0.00000001);
            if (LuxGetFloatProperty(tonemapper, "Aperture", floatval, mesg))
                YaLuxGlobal.tonemapFstop = floatval;
            if (LuxGetFloatProperty(tonemapper, "Film ISO", floatval, mesg))
                YaLuxGlobal.tonemapISO = floatval;
        }
    }

    // sampler settings
    outCFG.write("\nsampler.type = \"SOBOL\"");
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
    mesg += "periodicsave.film.outputs.period = 5\n";
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

    return true;
}

bool DazToLuxCoreFile::WriteRenderFiles()
{
    bool result;
    result = LuxMakeCFGFile(fullPathRenderFilenameBase + ".cfg", renderer, camera, options);
    if (result)
        result = LuxMakeSCNFile(fullPathRenderFilenameBase+ ".scn", renderer, camera, options);

    return result;
}

#include "moc_DazToLuxCoreFile.cpp"
