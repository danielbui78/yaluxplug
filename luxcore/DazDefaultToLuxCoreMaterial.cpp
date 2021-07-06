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

    //currentProperty = m_Material->findProperty("eta"); // index of refreaction
    //if (currentProperty != NULL)
    //{
    //    index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
    //}

    currentProperty = m_Material->findProperty("Opacity Strength"); // index of refreaction
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
            m_Roughness += 0.5;
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
        m_OpacityTex.data = GenerateCoreTextureBlock1(m_OpacityTex.name, m_OpacityMap, m_OpacityValue);

    // Diffuse Texture Block
    m_DiffuseTex.name = m_LuxMaterialName + "_d";
    if (m_DiffuseExists)
        m_DiffuseTex.data = GenerateCoreTextureBlock3(m_DiffuseTex.name, m_DiffuseMap,
            m_DiffuseColor.redF(), m_DiffuseColor.greenF(), m_DiffuseColor.blueF(),
            m_uscale, m_vscale, m_uoffset, m_voffset,
            m_DiffuseGamma, "", "");

    // Specular Block
    if (m_SpecularExists)
    {
        // check specular strength!!!
        m_SpecularTex.name = m_LuxMaterialName + "_s";
        QString realSpecularLabel = m_SpecularTex.name;
        bool bDoMixtureTexture = false;
        float spec_strength = 1.0;
        QString mesg;
        if (LuxGetFloatProperty(m_Material, "Specular Strength", spec_strength, mesg) && spec_strength < 1.0)
        {
            bDoMixtureTexture = true;
            realSpecularLabel = m_SpecularTex.name + "_0";
        }
        m_SpecularTex.data = GenerateCoreTextureBlock3(realSpecularLabel, m_SpecularMap,
            m_SpecularColor.redF(), m_SpecularColor.greenF(), m_SpecularColor.blueF(),
            m_uscale, m_vscale, m_uoffset, m_voffset, 
            m_DiffuseGamma, "", "");

        if (bDoMixtureTexture)
        {
            QString specularScaleLabel = m_SpecularTex.name;
            m_SpecularTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(specularScaleLabel);
            m_SpecularTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specularScaleLabel).arg(realSpecularLabel);
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = %2\n").arg(specularScaleLabel).arg(spec_strength);
        }
    }

    // Bumpmap Block
    m_BumpTex.name = m_LuxMaterialName + "_b";
    float bump_gamma = 1.0;
    if (m_BumpExists)
        m_BumpTex.data = GenerateCoreTextureBlock1_Grey(m_BumpTex.name, m_BumpMap, m_BumpStrength,
            m_uscale, m_vscale, m_uoffset, m_voffset,
            bump_gamma, "", "");

    return true;
}

bool DazDefaultToLuxCoreMaterial::CreateMaterials()
{
    QString ret_str;
    QString matLabel = m_LuxMaterialName;

    ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
    if (m_DiffuseExists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(m_DiffuseTex.name);
    if (m_SpecularExists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(m_SpecularTex.name);

    if (YaLuxGlobal.bDoBumpMaps)
    {
        if (m_BumpExists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(m_BumpTex.name);
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
    
    // add texture blocks
    ret_str += m_DiffuseTex.data;
    ret_str += m_SpecularTex.data;
    ret_str += m_BumpTex.data;
    ret_str += m_OpacityTex.data;

    // add material block
    ret_str += m_PrimaryMaterialBlock.data;

    return ret_str;
}

#include "moc_DazDefaultToLuxCoreMaterial.cpp"