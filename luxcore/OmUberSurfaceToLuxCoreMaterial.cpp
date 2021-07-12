#include "OmUberSurfaceToLuxCoreMaterial.h"

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

OmUberSurfaceToLuxCoreMaterial::OmUberSurfaceToLuxCoreMaterial(DzMaterial* m, QString luxMatName) :
    DzMaterialToLuxCoreMaterial(m, luxMatName)
{
    ImportValues();
    CreateTextures();
    CreateMaterials();
}

bool OmUberSurfaceToLuxCoreMaterial::ImportValues()
{

    //// diffuse image and color
    //float diffuse_vscale = -1;
    //float diffuse_uscale = 1;
    //float diffuse_gamma = 2.2;
    //float diffuse_voffset = 0; // vdelta
    //float diffuse_uoffset = 0; // udelta
    //QString diffuse_wrap = "repeat"; // repeat|black|clamp
    //QString diffuse_filtertype = "bilinear";
    //QString diffuse_channel = "";
    //QString diffuse_mapfile = ""; // Diffuse Color
    //QColor diffuse_value;
    //bool diffuse_exists = false;

    //// specular image and color
    //float spec_vscale = -1;
    //float spec_uscale = 1;
    //float spec_gamma = 2.2;
    //float spec_voffset = 0;
    //float spec_uoffset = 0;
    //QString spec_wrap = "repeat"; // repeat|black|clamp
    //QString spec_filtertype = "bilinear";
    //QString spec_channel = "";
    //QString spec_mapfile = ""; // Specular Color
    //QColor spec_value;
    //bool spec_exists = false;

    //// bump image and values
    //float bump_vscale = -1;
    //float bump_uscale = 1;
    //float bump_gamma = 1;
    //float bump_voffset = 0;
    //float bump_uoffset = 0;
    //QString bump_channel = "";
    //QString bump_wrap = "repeat";
    //QString bump_filtertype = "bilinear";
    //QString bump_mapfile = ""; // "Bump Strength"
    //float bump_value;
    //bool bump_exists = false;

    //// transmission map
    //QString opacity_mapfile = "";
    //float opacity_value = 1;
    //bool opacity_exists = false;

    //// material definition
    //float uroughness = 0.8;
    //float vroughness = 0.8;
    //float index_refraction = 0.0; // IOR


    enum { glossy, matte, plastic, metal } material_type = glossy;

    QString propertyLabel;
    DzProperty* currentProperty;

    // Matte vs Glossy
    currentProperty = m_Material->findProperty("Lighting Model");
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

    currentProperty = m_Material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        m_DiffuseColor = ((DzColorProperty*)currentProperty)->getColorValue();
        m_DiffuseMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_DiffuseColor != QColor(255,255,255)) || (m_DiffuseMap!= ""))
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
        m_SpecularColor= ((DzColorProperty*)currentProperty)->getColorValue();
        m_SpecularMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_SpecularColor != QColor(255,255,255)) || (m_SpecularMap != ""))
            m_SpecularExists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = m_Material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
        m_BumpStrength= ((DzFloatProperty*)currentProperty)->getValue() / 100;
        m_BumpMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (m_BumpMap!= "")
            m_BumpExists = true;
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

        if (material_type == plastic)
        {
            m_Roughness += 0.5;
            m_Roughness = (m_Roughness > 1.0) ? 1.0 : m_Roughness;
        }

    }

    return true;
}

bool OmUberSurfaceToLuxCoreMaterial::CreateTextures()
{

    m_DiffuseTex.name = m_LuxMaterialName + "_d";
    // Diffuse Texture Block
    if (m_DiffuseExists)
        m_DiffuseTex.data += GenerateCoreTextureBlock3(m_DiffuseTex.name, m_DiffuseMap,
            GetRed(m_DiffuseColor), GetGreen(m_DiffuseColor), GetBlue(m_DiffuseColor),
            m_uscale, m_vscale, m_uoffset, m_voffset);

    // Specular Block
    if (m_SpecularExists)
    {
        // check specular strength!!!
        QString realSpecularLabel = m_LuxMaterialName + "_s";
        m_SpecularTex.name = realSpecularLabel;
        bool bDoMixtureTexture = false;
        float spec_strength = 1.0;

        // Always mix down by 25%
        QString mesg;
        LuxGetFloatProperty(m_Material, "Specular Strength", spec_strength, mesg);
        realSpecularLabel = m_LuxMaterialName + "_s" + "_0";
//        spec_strength *= 0.25;

        m_SpecularTex.data += GenerateCoreTextureBlock3(realSpecularLabel, m_SpecularMap,
            GetRed(m_SpecularColor), GetGreen(m_SpecularColor), GetBlue(m_SpecularColor),
            m_uscale, m_vscale, m_uoffset, m_voffset);

        m_SpecularTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(m_SpecularTex.name);
        m_SpecularTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(m_SpecularTex.name).arg(realSpecularLabel);
        m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(m_SpecularTex.name).arg(spec_strength);

    }

    // Bumpmap Block
    m_BumpTex.name = m_LuxMaterialName + "_b";
    if (m_BumpExists)
        m_BumpTex.data += GenerateCoreTextureBlock1_Grey(m_BumpTex.name, m_BumpMap, m_BumpStrength,
            m_uscale, m_vscale, m_uoffset, m_voffset);


    // Opacity Block
    m_OpacityTex.name = m_LuxMaterialName + "_o";
    if (m_OpacityExists && m_OpacityMap != "")
    {
        m_OpacityTex.data = GenerateCoreTextureBlock1(m_OpacityTex.name, m_OpacityMap, m_OpacityValue,
            m_uscale, m_vscale, m_uoffset, m_voffset);
    }

    return true;
}

bool OmUberSurfaceToLuxCoreMaterial::CreateMaterials()
{
    QString ret_str = "";

    // Material definition
    // decide what type of material...
    ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(m_LuxMaterialName);
    if (m_DiffuseExists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(m_LuxMaterialName).arg(m_DiffuseTex.name);
    if (m_SpecularExists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(m_LuxMaterialName).arg(m_SpecularTex.name);

    if (YaLuxGlobal.bDoBumpMaps)
    {
        if (m_BumpExists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(m_LuxMaterialName).arg(m_BumpTex.name);
    }

    ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(m_LuxMaterialName).arg(m_Roughness);
    ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(m_LuxMaterialName).arg(m_Roughness);

    if (m_OpacityExists && m_OpacityMap != "")
        ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(m_LuxMaterialName).arg(m_OpacityTex.name);
    else if (m_OpacityExists)
        ret_str += QString("scene.materials.%1.transparency = %2\n").arg(m_LuxMaterialName).arg(m_OpacityValue);


    m_PrimaryMaterialBlock.name = m_LuxMaterialName;
    m_PrimaryMaterialBlock.data = ret_str;

    return false;
}


QString OmUberSurfaceToLuxCoreMaterial::toString()
{
    QString ret_str = "# (OmUberSurface) MATERIAL " + m_LuxMaterialName + "\n";

    // add texture blocks
    ret_str += m_DiffuseTex.data;
    ret_str += m_SpecularTex.data;
    ret_str += m_BumpTex.data;
    ret_str += m_OpacityTex.data;

    // add material block
    ret_str += m_PrimaryMaterialBlock.data;

    return ret_str;
}

#include "moc_OmUberSurfaceToLuxCoreMaterial.cpp"
