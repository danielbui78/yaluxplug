#include "DazDefaultToLuxCoreMaterial.h"

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

DazDefaultToLuxCoreMaterial::DazDefaultToLuxCoreMaterial(DzMaterial* material, QString luxMatName)
	: DzMaterialToLuxCoreMaterial(material, luxMatName)
{
    ImportValues();
    CreateTextures();
    CreateMaterials();
}

bool DazDefaultToLuxCoreMaterial::ImportValues()
{

    // diffuse image and color
    //float m_vscale = -1;
    //float m_uscale = 1;
    //float diffuse_gamma = 2.2;
    //float m_Material_voffset = 0; // vdelta
    //float m_Material_uoffset = 0; // udelta
    //QString diffuse_wrap = "repeat"; // repeat|black|clamp
    //QString diffuse_filtertype = "bilinear";
    //QString diffuse_channel = "";
    //QString m_DiffuseMap = ""; // Diffuse Color
    //QColor m_DiffuseColor = QColor(255, 255, 255);
    //bool diffuse_exists = false;

    // specular image and color
    //float spec_gamma = 2.2;
    //QString spec_wrap = "repeat"; // repeat|black|clamp
    //QString spec_filtertype = "bilinear";
    //QString spec_channel = "";
    //QString spec_mapfile = ""; // Specular Color
    //QColor spec_value;
    //bool spec_exists = false;

    // bump image and values
    //float bump_gamma = 1;
    //QString bump_channel = "";
    //QString bump_wrap = "repeat";
    //QString bump_filtertype = "bilinear";
    //QString bump_mapfile = ""; // "Bump Strength"
    //float bump_value;
    //bool bump_exists = false;

    // transmission map
    //QString opacity_mapfile = "";
    //float opacity_value = 1;
    //bool opacity_exists = false;

    // m_Material definition
    //float uroughness = 0.8;
    //float vroughness = 0.8;
    //float index_refraction = 0.0; // IOR

    QString propertyLabel;
    DzProperty* currentProperty;

    // m_Material types
    enum { glossy, matte, plastic, metal } m_Material_type = glossy;
    currentProperty = m_Material->findProperty("Lighting Model");
    if (currentProperty != NULL)
    {
        QString lighting_model = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
        if (lighting_model.contains("glossy"))
        {
            m_Material_type = glossy;
        }
        else if (lighting_model.contains("matte"))
        {
            m_Material_type = matte;
        }
        else if (lighting_model.contains("plastic"))
        {
            m_Material_type = plastic;
        }
        else if (lighting_model.contains("metal"))
        {
            m_Material_type = metal;
        }
    }

    currentProperty = m_Material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        // Sanity Check (FIX for Granite Iray Shader)
        if (currentProperty->inherits("DzColorProperty"))
            m_DiffuseColor = ((DzColorProperty*)currentProperty)->getColorValue();
        if (currentProperty->inherits("DzNumericProperty"))
            m_DiffuseMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        else if (currentProperty->inherits("DzImageProperty"))
            m_DiffuseMap = ((DzImageProperty*)currentProperty)->getValue()->getTempFilename();
        if ((m_DiffuseColor != QColor(255, 255, 255)) || (m_DiffuseMap != ""))
            m_DiffuseExists = true;
    }
    currentProperty = m_Material->findProperty("Horizontal Tiles");
    if (currentProperty != NULL)
    {
        m_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Vertical Tiles");
    if (currentProperty != NULL)
    {
        m_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Horizontal Offset");
    if (currentProperty != NULL)
    {
        m_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Vertical Offset");
    if (currentProperty != NULL)
    {
        m_voffset = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Specular Color");
    if (currentProperty != NULL)
    {
        m_SpecularColor = ((DzColorProperty*)currentProperty)->getColorValue();
        m_SpecularMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_SpecularColor != 1) || (m_SpecularMap != ""))
            m_SpecularExists = true;
    }
    currentProperty = m_Material->findProperty("Specular Strength");
    if (currentProperty != NULL)
    {
        m_SpecularStrength = ((DzFloatProperty*)currentProperty)->getValue();
        m_SpecularStrengthMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = m_Material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        m_BumpStrength = ((DzFloatProperty*)currentProperty)->getValue() / 100;
        m_BumpMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (m_BumpMap != "")
            m_BumpExists = true;
    }
    currentProperty = m_Material->findProperty("Normal Map");
    if (currentProperty != NULL)
    {
        m_NormalMap = propertyValuetoString(currentProperty);
        if (m_NormalMap != "")
            m_NormalExists = true;
    }

    //currentProperty = m_Material->findProperty("eta"); // index of refreaction
    //if (currentProperty != NULL)
    //{
    //    index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    //}

    currentProperty = m_Material->findProperty("Opacity Strength");
    if (currentProperty != NULL)
    {
        m_OpacityValue = ((DzFloatProperty*)currentProperty)->getValue();
        m_OpacityMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_OpacityValue != 1) || (m_OpacityMap != ""))
            m_OpacityExists = true;
    }
    currentProperty = m_Material->findProperty("Glossiness");
    if (currentProperty != NULL)
    {
        m_Roughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
        if (m_Roughness > 0.8) m_Roughness = 0.8;

        if (m_Material_type == plastic)
        {
            m_Roughness += 0.33;
            m_Roughness = (m_Roughness > 1.0) ? 1.0 : m_Roughness;
        }

    }

    return true;
}

bool DazDefaultToLuxCoreMaterial::CreateTextures()
{

    // Opacity Block
    m_OpacityTex.name = m_LuxMaterialName + "_o";
    if (m_OpacityExists && m_OpacityMap != "")
    {
        double cutoff_threshold = 0.5;
        double feather_amount = 0.8;
        m_OpacityTex.data = GenerateCoreTextureBlock1(m_OpacityTex.name, m_OpacityMap, m_OpacityValue,
            m_uscale, m_vscale, m_uoffset, m_voffset );

        m_OpacityTex.data += CreateFeatheredCutOffTexture(m_OpacityTex.name, "greaterthan", cutoff_threshold, feather_amount);
        m_OpacityTex.name += "_cutoff_feathered";
    }

    // Diffuse Texture Block
    m_DiffuseTex.name = m_LuxMaterialName + "_d";
    if (m_DiffuseExists)
        m_DiffuseTex.data = GenerateCoreTextureBlock3(m_DiffuseTex.name, m_DiffuseMap,
            GetRed(m_DiffuseColor), GetGreen(m_DiffuseColor), GetBlue(m_DiffuseColor),
            m_uscale, m_vscale, m_uoffset, m_voffset,
            m_DiffuseGamma, "", "");

    // Specular Block
    if (m_SpecularExists)
    {
        m_SpecularTex.name = m_LuxMaterialName + "_s";
        QString specTex = m_SpecularTex.name;
        QString specStrengthTex = m_SpecularTex.name + "_strength";

        bool bDoMixtureTexture = false;
        if (m_SpecularStrength < 1.0 || m_SpecularStrengthMap != "")
        {
            bDoMixtureTexture = true;
            specTex = m_SpecularTex.name + "_0";
            if (m_SpecularStrengthMap != "")
                m_SpecularTex.data += GenerateCoreTextureBlock1(specStrengthTex, m_SpecularStrengthMap, m_SpecularStrength,
                    m_uscale, m_vscale, m_uoffset, m_voffset);
            else
                specStrengthTex = QString("%1").arg(m_SpecularStrength);
        }

        m_SpecularTex.data += GenerateCoreTextureBlock3(specTex, m_SpecularMap,
            GetRed(m_SpecularColor), GetGreen(m_SpecularColor), GetBlue(m_SpecularColor),
            m_uscale, m_vscale, m_uoffset, m_voffset, 
            m_DiffuseGamma, "", "");

        if (bDoMixtureTexture)
        {
            QString specularScaleLabel = m_SpecularTex.name;
            m_SpecularTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(specularScaleLabel);
            m_SpecularTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specularScaleLabel).arg(specTex);
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specularScaleLabel).arg(specStrengthTex);
        }
    }

    // Bumpmap Block
    m_BumpTex.name = m_LuxMaterialName + "_b";
    if (m_BumpExists)
        m_BumpTex.data = GenerateCoreTextureBlock1_Grey(m_BumpTex.name, m_BumpMap, m_BumpStrength,
            m_uscale, m_vscale, m_uoffset, m_voffset);

    // Normalmap Block
    QString normalMapName = m_LuxMaterialName + "_n";
    QString imageMapName = normalMapName + "_t";
    if (m_NormalMap != "" && YaLuxGlobal.bDoNormalMaps)
    {
        if (YaLuxGlobal.bDoNormalAsBump)
        {
            double scale = 0.02;
            m_NormalTex.data += GenerateCoreTextureBlock1(imageMapName, m_NormalMap, 1.0,
                m_uscale, m_vscale, m_uoffset, m_voffset,
                1.0, "", "colored_mean");
            //m_NormalTex.data += QString("scene.textures.%1.type = \"mix\"\n").arg(normalMapName);
            //m_NormalTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(normalMapName).arg(0.5);
            //m_NormalTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(normalMapName).arg(imageMapName);
            //m_NormalTex.data += QString("scene.textures.%1.amount = \"%2\"\n").arg(normalMapName).arg(scale);
            m_NormalTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(normalMapName);
            m_NormalTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(normalMapName).arg(imageMapName);
            m_NormalTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(normalMapName).arg(-0.2);
            m_NormalTex.name = normalMapName;
        }
        else
        {
            m_NormalTex.data += GenerateCoreTextureBlock3(imageMapName, m_NormalMap,
                1.0, 1.0, 1.0,
                m_uscale, m_vscale, m_uoffset, m_voffset,
                1.0, "", "rgb");
            m_NormalTex.name = imageMapName;
        }

    }


    return true;
}

bool DazDefaultToLuxCoreMaterial::CreateMaterials()
{
    QString ret_str = "";
    QString matLabel = m_LuxMaterialName;

    ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
    if (m_DiffuseExists) 
        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(m_DiffuseTex.name);
    else
        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg("1.0 1.0 1.0");
    if (m_SpecularExists) 
        ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(m_SpecularTex.name);
    else
        ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg("1.0 1.0 1.0");


    if (YaLuxGlobal.bDoBumpMaps)
    {
        if (m_BumpExists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(m_BumpTex.name);
    }
    if (YaLuxGlobal.bDoNormalMaps && m_NormalExists)
    {
        if (YaLuxGlobal.bDoNormalAsBump)
            ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(m_LuxMaterialName).arg(m_NormalTex.name);
        else
            ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(m_LuxMaterialName).arg(m_NormalTex.name);
    }

    ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(m_Roughness);
    ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(m_Roughness);

    if (m_OpacityExists)
    {
        if (m_OpacityMap != "")
            ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(matLabel).arg(m_OpacityTex.name);
        else
            ret_str += QString("scene.materials.%1.transparency = %2\n").arg(matLabel).arg(m_OpacityValue);
    }

    m_PrimaryMaterialBlock.name = matLabel;
    m_PrimaryMaterialBlock.data = ret_str;

    return true;
}

QString DazDefaultToLuxCoreMaterial::toString()
{
    QString ret_str;

    ret_str = "# (DazDefault) MATERIAL " + m_LuxMaterialName + "\n";

    // add texture blocks
    ret_str += m_DiffuseTex.data;
    ret_str += m_SpecularTex.data;
    ret_str += m_BumpTex.data;
    ret_str += m_NormalTex.data;
    ret_str += m_OpacityTex.data;

    // add material block
    ret_str += m_PrimaryMaterialBlock.data;

    return ret_str;
}

#include "moc_DazDefaultToLuxCoreMaterial.cpp"