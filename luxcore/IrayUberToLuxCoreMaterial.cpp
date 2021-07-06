#include "IrayUberToLuxCoreMaterial.h"

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

IrayUberToLuxCoreMaterial::IrayUberToLuxCoreMaterial(DzMaterial* m, QString luxMatName) :
	DzMaterialToLuxCoreMaterial(m, luxMatName)
{
	ImportValues();
	CreateTextures();
	CreateMaterials();
}

bool IrayUberToLuxCoreMaterial::ImportValues()
{
 
    //QString ret_str = "# MATERIAL " + m_LuxMaterialName + "\n";

    // diffuse image and color
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
    //QString specweight_mapfile = ""; // Specular [Dual Lobe Specular]
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

    //float metallic_weight = 0.0;
    //QString metallic_mapfile = ""; // "Metallic Weight"

    //float spec_weight = 0;
    //float refraction_weight = 0;
    //float translucency_weight = 0;
    //float glossy_layered_weight = 0;

    //// translucency (volume rendering)
    //QColor translucency_color; // "Translucency Color"
    //QString translucency_mapfile = ""; // "Translucency Color"
    //QColor sss_tint = QColor(255, 255, 255); // "SSS Reflectance Tint"
    //bool translucency_exists = false;
    //bool volume_exists = false;
    ///////////////////// Volume
    //float transmission_distance = 0; // "Transmitted Measurement Distance"
    //QColor transmission_color; // "Transmitted Color"
    //float scattering_distance = 0; // "Scattering Measurement Distance"
    //QColor scattering_color; // "SSS Color"
    //float scattering_direction; // "SSS Direction" == assymetry in luxcore

    //// normal map
    //float normal_strength = 0; // "Normal Map"
    //QString normal_mapfile = "";

    //// glossy layer
    //float glossy_roughness = 0;
    //QString glossy_roughness_mapfile = "";
    //float glossy_reflectivity = 0;
    //QColor glossy_color;
    //QString glossy_mapfile = "";

    // cheats
    float glossy_anisotropy = 0;


    enum { metal_roughness, specular_glossy } material_type = specular_glossy;

    QString propertyLabel;
    DzProperty* currentProperty;

    // Matte vs Glossy
    currentProperty = m_Material->findProperty("Base Mixing");
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

    currentProperty = m_Material->findProperty("Diffuse Color");
    if (currentProperty != NULL)
    {
        m_DiffuseColor = ((DzColorProperty*)currentProperty)->getColorValue();
        m_DiffuseMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
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
    currentProperty = m_Material->findProperty("Dual Lobe Specular Weight");
    if (currentProperty != NULL)
    {
        m_SpecularWeight = ((DzFloatProperty*)currentProperty)->getValue();
        m_SpecularMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_SpecularWeight != 0) || (m_SpecularMap != ""))
            m_SpecularExists = true;
    }
    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
    currentProperty = m_Material->findProperty("Bump Strength");
    if (currentProperty != NULL)
    {
//        m_BumpStrength = ((DzFloatProperty*)currentProperty)->getValue() / 500; // equivalent to (* 0.002)
        m_BumpStrength = ((DzFloatProperty*)currentProperty)->getValue() * 0.0012;
        m_BumpMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if (m_BumpMap != "")
            m_BumpExists = true;
    }
    currentProperty = m_Material->findProperty("Normal Map");
    if (currentProperty != NULL)
    {
        m_NormalStrength = ((DzFloatProperty*)currentProperty)->getValue();
        m_NormalMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    currentProperty = m_Material->findProperty("Refraction Weight");
    if (currentProperty != NULL)
    {
        m_RefractionWeight = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Refraction Index"); // index of refreaction
    if (currentProperty != NULL)
    {
        m_RefractionIndex = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Cutout Opacity"); // cutout opacity
    if (currentProperty != NULL)
    {
        m_OpacityValue = ((DzFloatProperty*)currentProperty)->getValue();
        m_OpacityMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        if ((m_OpacityValue != 1) || (m_OpacityMap != ""))
            m_OpacityExists = true;
    }
    ////////  Glossy Layer //////////////
    currentProperty = m_Material->findProperty("Glossy Layered Weight"); // glossy layered weight
    if (currentProperty != NULL)
    {
        m_GlossyLayeredWeight = ((DzFloatProperty*)currentProperty)->getValue();
    }
    currentProperty = m_Material->findProperty("Glossy Color");
    if (currentProperty != NULL)
    {
        m_GlossyColor = ((DzColorProperty*)currentProperty)->getColorValue();
        m_GlossyMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    currentProperty = m_Material->findProperty("Glossy Reflectivity"); // glossy reflectivity
    if (currentProperty != NULL)
    {
        m_GlossyReflectivity = ((DzFloatProperty*)currentProperty)->getValue() * m_GlossyLayeredWeight;
    }
    currentProperty = m_Material->findProperty("Glossy Roughness"); // glossy roughness
    if (currentProperty != NULL)
    {
        m_GlossyRoughness = ((DzFloatProperty*)currentProperty)->getValue() * m_GlossyLayeredWeight;
        m_GlossyRoughnessMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
    }
    if (material_type == metal_roughness)
    {
        currentProperty = m_Material->findProperty("Metallic Weight"); // metallicity
        if (currentProperty != NULL)
        {
            m_MetallicWeight = ((DzFloatProperty*)currentProperty)->getValue();
            m_MetallicMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
        }
    }
    if (material_type == metal_roughness)
    {
        currentProperty = m_Material->findProperty("Glossy Roughness");
        if (currentProperty != NULL)
        {
            // Linear Interpolation: roughness == 1.0 when glossy_layered_weight == 0%
            m_Roughness = 1.0 * (1 - m_GlossyLayeredWeight) + ((DzFloatProperty*)currentProperty)->getValue() * m_GlossyLayeredWeight;
            if (m_Roughness > 0.8) m_Roughness = 0.8;

        }
    }
    else
    {
        currentProperty = m_Material->findProperty("Glossiness");
        if (currentProperty != NULL)
        {
            // simple linear interpolation: glossiness == 0 when glossy_layered_weight == 0%
            m_Roughness = ((DzFloatProperty*)currentProperty)->getValue() * m_GlossyLayeredWeight;
            m_Roughness = (1 - m_Roughness > 0.8) ? 0.8 : (1 - m_Roughness);
        }
    }
    currentProperty = m_Material->findProperty("Glossy Anisotropy"); // index of refreaction
    if (currentProperty != NULL)
    {
        glossy_anisotropy = ((DzFloatProperty*)currentProperty)->getValue() * m_GlossyLayeredWeight;
        if (glossy_anisotropy > 0)
        {
            if (material_type == metal_roughness)
            {
                m_Roughness = (0.8 * glossy_anisotropy) + (m_Roughness * (1 - glossy_anisotropy));
            }
            else if (material_type == specular_glossy)
            {
                m_Roughness = (m_Roughness * (1 - glossy_anisotropy));
            }
        }
    }
    currentProperty = m_Material->findProperty("Translucency Weight");
    if (currentProperty != NULL)
    {
        m_TranslucencyWeight = ((DzFloatProperty*)currentProperty)->getValue();
        if (m_TranslucencyWeight != 0)
        {
            m_TranslucencyExists = true;
            currentProperty = m_Material->findProperty("Translucency Color");
            if (currentProperty != NULL)
            {
                m_TranslucencyColor = ((DzColorProperty*)currentProperty)->getColorValue();
                m_TranslucencyMap = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
            }
            currentProperty = m_Material->findProperty("SSS Reflectance Tint");
            if (currentProperty != NULL)
            {
                m_SSS_tint = ((DzColorProperty*)currentProperty)->getColorValue();
            }
            /////////////////// volume rendering
            currentProperty = m_Material->findProperty("Transmitted Color");
            if (currentProperty != NULL)
            {
                m_TransmissionColor = ((DzColorProperty*)currentProperty)->getColorValue();
                if (m_TransmissionColor != QColor(0, 0, 0))
                {
                    m_VolumeExists = true;
                }
            }
            currentProperty = m_Material->findProperty("SSS Color");
            if (currentProperty != NULL)
            {
                m_ScatteringColor = ((DzColorProperty*)currentProperty)->getColorValue();
            }
            currentProperty = m_Material->findProperty("Transmitted Measurement Distance");
            if (currentProperty != NULL)
            {
                // I think Daz values are in cm, so divide by 100 to get meters for lux?
                m_TransmissionDistance = ((DzFloatProperty*)currentProperty)->getValue() / 100;
            }
            currentProperty = m_Material->findProperty("Scattering Measurement Distance");
            if (currentProperty != NULL)
            {
                m_ScatteringDistance = ((DzFloatProperty*)currentProperty)->getValue() / 1;
            }
            currentProperty = m_Material->findProperty("SSS Direction");
            if (currentProperty != NULL)
            {
                m_ScatteringDirection = ((DzFloatProperty*)currentProperty)->getValue();
            }

        }
    }

	return true;
}

bool IrayUberToLuxCoreMaterial::CreateTextures()
{

    // TODO: check for metallicity = 0 vs 1.... see metallicity below
    // Diffuse Texture Block
    QString mainDiffTex = m_LuxMaterialName + "_d";
    m_DiffuseTex.name = mainDiffTex;

    if (m_DiffuseExists)
        m_DiffuseTex.data += GenerateCoreTextureBlock3(mainDiffTex, m_DiffuseMap,
            m_DiffuseColor.redF(), m_DiffuseColor.greenF(), m_DiffuseColor.blueF(),
            m_uscale, m_vscale, m_uoffset, m_voffset,
            2.2, "", "");


    // Specular Block (DUAL LOBE)
    float spec1_float = 0;
    float spec2_float = 0;
    float spec_ratio = 0;
    float spec_reflectivity = 0;
    QString spec1_mapfile = "";
    QString spec2_mapfile = "";
    QString specratio_mapfile = "";
    QString specref_mapfile = "";

    QString mainSpec = m_LuxMaterialName + "_s";
    m_SpecularTex.name = mainSpec;

    QString specref_label = mainSpec + "_spec_reflect";
    QString rawDualRoughness = mainSpec + "_raw_rough";
    m_SpecRoughness_1 = mainSpec + "_scaled_rough1";
    m_SpecRoughness_2 = mainSpec + "_scaled_rough2";
    QString spec1_label = rawDualRoughness + "_spec1";
    QString spec2_label = rawDualRoughness + "_spec2";
    QString specratio_label = rawDualRoughness + "_specratio";

    if (m_SpecularWeight > 0 && YaLuxGlobal.bDoSpecular)
    {
        QString mesg;
        LuxGetFloatProperty(m_Material, "Specular Lobe 1 Roughness", spec1_float, mesg);
        spec1_mapfile = LuxGetImageMapProperty(m_Material, "Specular Lobe 1 Roughness", mesg);
        LuxGetFloatProperty(m_Material, "Specular Lobe 2 Roughness", spec2_float, mesg);
        spec2_mapfile = LuxGetImageMapProperty(m_Material, "Specular Lobe 2 Roughness", mesg);
        LuxGetFloatProperty(m_Material, "Dual Lobe Specular Ratio", spec_ratio, mesg);
        specratio_mapfile = LuxGetImageMapProperty(m_Material, "Dual Lobe Specular Ratio", mesg);
        LuxGetFloatProperty(m_Material, "Dual Lobe Specular Reflectivity", spec_reflectivity, mesg);
        specref_mapfile = LuxGetImageMapProperty(m_Material, "Dual Lobe Specular Reflectivity", mesg);

        // generate texture 1, 2, ratio, reflectivity
        if (m_SpecularMap != "")
            m_SpecularTex.data += GenerateCoreTextureBlock1(mainSpec + "_weight", m_SpecularMap, m_SpecularWeight,
                m_uscale, m_vscale, m_uoffset, m_voffset);
        if (spec1_mapfile != "")
            m_SpecularTex.data += GenerateCoreTextureBlock1(spec1_label, spec1_mapfile, spec1_float,
                m_uscale, m_vscale, m_uoffset, m_voffset);
        if (spec2_mapfile != "")
            m_SpecularTex.data += GenerateCoreTextureBlock1(spec2_label, spec1_mapfile, spec2_float,
                m_uscale, m_vscale, m_uoffset, m_voffset);
        if (specratio_mapfile != "")
            m_SpecularTex.data += GenerateCoreTextureBlock1(specratio_label, specratio_mapfile, spec_ratio,
                m_uscale, m_vscale, m_uoffset, m_voffset);
        if (specref_mapfile != "")
            //            ret_str += GenerateCoreTextureBlock1(specref_label, specref_mapfile, spec_reflectivity);
            m_SpecularTex.data += GenerateCoreTextureBlock1(specref_label, specref_mapfile, 1.0,
                m_uscale, m_vscale, m_uoffset, m_voffset);

        // mix spec1 + spec2
        m_SpecularTex.data += QString("scene.textures.%1.type = \"mix\"\n").arg(rawDualRoughness);
        if (spec1_mapfile != "")
            m_SpecularTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(rawDualRoughness).arg(spec1_label);
        else
            m_SpecularTex.data += QString("scene.textures.%1.texture1 = %2\n").arg(rawDualRoughness).arg(spec1_float);
        if (spec2_mapfile != "")
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(rawDualRoughness).arg(spec2_label);
        else
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = %2\n").arg(rawDualRoughness).arg(spec2_float);
        if (specratio_mapfile != "")
            m_SpecularTex.data += QString("scene.textures.%1.amount = \"%2\"\n").arg(rawDualRoughness).arg(specratio_label);
        else
            m_SpecularTex.data += QString("scene.textures.%1.amount = %2\n").arg(rawDualRoughness).arg(spec_ratio);

        // scale roughness from grey (0.5)
        m_SpecularTex.data += QString("scene.textures.%1.type = \"mix\"\n").arg(m_SpecRoughness_2);
        m_SpecularTex.data += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(m_SpecRoughness_2);
        m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(m_SpecRoughness_2).arg(rawDualRoughness);
        if (m_SpecularMap != "")
            m_SpecularTex.data += QString("scene.textures.%1.amount = \"%2\"\n").arg(m_SpecRoughness_2).arg(mainSpec + "_weight");
        else
            m_SpecularTex.data += QString("scene.textures.%1.amount = %2\n").arg(m_SpecRoughness_2).arg(m_SpecularWeight);

        // scale roughness from grey (0.5)
        m_SpecularTex.data += QString("scene.textures.%1.type = \"mix\"\n").arg(m_SpecRoughness_1);
        m_SpecularTex.data += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(m_SpecRoughness_1);
        if (spec1_mapfile != "")
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(m_SpecRoughness_1).arg(spec1_label);
        else
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(m_SpecRoughness_1).arg(spec1_float);
        if (m_SpecularMap != "")
            m_SpecularTex.data += QString("scene.textures.%1.amount = \"%2\"\n").arg(m_SpecRoughness_1).arg(mainSpec + "_weight");
        else
            m_SpecularTex.data += QString("scene.textures.%1.amount = %2\n").arg(m_SpecRoughness_1).arg(m_SpecularWeight);


        // mix specweight
        //if (specref_mapfile != "")
        //{
        //    //// no scaling of specular reflectivity
        //    //mainSpec = QString("%1 %1 %1").arg(spec_reflectivity);
        //    //if (specref_mapfile != "") mainSpec = specref_label;
        //    mainSpec = specref_label;
        //}
        QString finalMix = QString("%1 %1 %1").arg(spec_reflectivity);
        if (specref_mapfile != "") finalMix = specref_label;
        m_SpecularTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(mainSpec);
        m_SpecularTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(mainSpec).arg(finalMix);
        if (m_SpecularMap != "")
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(mainSpec).arg(mainSpec + "_weight");
        else
            m_SpecularTex.data += QString("scene.textures.%1.texture2 = %2\n").arg(mainSpec).arg(m_SpecularWeight);

    }
    else
    {
        mainSpec = "0 0 0";
        m_SpecularTex.name = mainSpec;
    }

    // Glossy Layer
    QString glossyRoughnessTexLabel = m_LuxMaterialName + "_glossyRoughnessTex";
    if (m_GlossyLayeredWeight > 0)
    {
        if (YaLuxGlobal.bDoSpecular)
        {
            QString glossyTex = m_LuxMaterialName + "_glossy";
            if (m_GlossyMap != "")
                m_GlossyTex.data += GenerateCoreTextureBlock3(glossyTex, m_GlossyMap,
                    m_GlossyColor.redF(), m_GlossyColor.greenF(), m_GlossyColor.blueF(),
                    m_uscale, m_vscale, m_uoffset, m_voffset);
            else
                glossyTex = QString("%1 %2 %3").arg(m_GlossyColor.redF()).arg(m_GlossyColor.greenF()).arg(m_GlossyColor.blueF());

            // mix glossy with mainSpec (dual spec)
            QString glossyMixTex = m_LuxMaterialName + "_glossy" + "_mix";
            m_GlossyTex.name = glossyMixTex;

            m_GlossyTex.data += QString("scene.textures.%1.type = \"mix\"\n").arg(glossyMixTex);
            m_GlossyTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(glossyMixTex).arg(mainSpec);
            m_GlossyTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(glossyMixTex).arg(glossyTex);
            m_GlossyTex.data += QString("scene.textures.%1.amount = \"%2\"\n").arg(glossyMixTex).arg(m_GlossyLayeredWeight * 0.05);
            mainSpec = glossyMixTex;
            m_SpecularTex.name = mainSpec;
        }

        if (m_GlossyRoughnessMap != "")
        {
            // note: roughness is already multiplied by glossy_layered_weight when imported above
            m_GlossyRoughnessTex.name = glossyRoughnessTexLabel;
            m_GlossyRoughnessTex.data += GenerateCoreTextureBlock1(glossyRoughnessTexLabel, m_GlossyMap, m_GlossyRoughness,
                m_uscale, m_vscale, m_uoffset, m_voffset);
        }

    }

    // Bumpmap Block
    QString bumpMapName = m_LuxMaterialName + "_b";
    m_BumpTex.name = bumpMapName;
    if (m_LuxMaterialName.toLower().contains("face") || m_LuxMaterialName.toLower().contains("lip") || m_LuxMaterialName.toLower().contains("ear"))
        m_BumpStrength *= 0.515;
    if (m_BumpExists && YaLuxGlobal.bDoBumpMaps)
        m_BumpTex.data += GenerateCoreTextureBlock1_Grey(bumpMapName, m_BumpMap, m_BumpStrength,
            m_uscale, m_vscale, m_uoffset, m_voffset);

    // Normalmap Block
    QString normalMapName = m_LuxMaterialName + "_n";
    m_NormalTex.name = normalMapName;
    QString imageMapName = normalMapName + "_t";
    if (m_NormalMap != "" && YaLuxGlobal.bDoNormalMaps)
    {
        float scale = m_NormalStrength * 1.0; // Multiply by any necessary render engine correction here
        m_NormalTex.data += GenerateCoreTextureBlock1(normalMapName, m_NormalMap, 1.0,
            m_uscale, m_vscale, m_uoffset, m_voffset);
        if (scale != 1)
        {
            m_NormalTex.data += QString("scene.textures.%1.type = \"normalmap\"\n").arg(normalMapName);
            m_NormalTex.data += QString("scene.textures.%1.texture = \"%2\"\n").arg(normalMapName).arg(imageMapName);
            m_NormalTex.data += QString("scene.textures.%1.scale = \"%2\"\n").arg(normalMapName).arg(scale);
        }

    }

    // TranslucencyMap Block
    QString translucencyTexture = m_LuxMaterialName + "_translucency";
    QString transmissionTexture = m_LuxMaterialName + "_transmission";
    QString absorptionTexture = m_LuxMaterialName + "_absorption";
    QString scatteringTexture = m_LuxMaterialName + "_scattering";

    QString scaled_transmissionTexture = m_LuxMaterialName + "_transmission_scaled";
    QString scaled_absorptionTexture = m_LuxMaterialName + "_absorption_scaled";
    QString scaled_scatteringTexture = m_LuxMaterialName + "_scattering_scaled";

    m_TranslucencyTex.name = translucencyTexture;

    QString volumeLabel = m_LuxMaterialName + "_volume";
    if (m_TranslucencyExists && YaLuxGlobal.bDoTranslucency)
    {
        m_TranslucencyTex.data += GenerateCoreTextureBlock3(translucencyTexture, m_TranslucencyMap,
            m_TranslucencyColor.redF(), m_TranslucencyColor.greenF(), m_TranslucencyColor.blueF(),
            m_uscale, m_vscale, m_uoffset, m_voffset);
    }
    if (m_VolumeExists && YaLuxGlobal.bDoSSSVolume && m_RefractionWeight == 0)
    {
        // create volumedata and check against volumelist
        VolumeData* v = new VolumeData();
        v->name = volumeLabel;
        v->type = "homogeneous";
        v->transmission_color = m_TransmissionColor.value();
        v->transmission_distance = m_TransmissionDistance;
        v->scattering_color = m_ScatteringColor.value();
        v->scattering_distance = m_ScatteringDistance;
        v->asymmetry_val = m_ScatteringDirection;
        v->multiscattering = true;

        // search volumelist
        bool match_found = false;
        for (QList<VolumeData*>::iterator el_iter = YaLuxGlobal.VolumeList.begin(); el_iter != YaLuxGlobal.VolumeList.end(); el_iter++)
        {
            VolumeData* el = *el_iter;
            if (*v == *el)
            {
                match_found = true;
                // change volumeLabel to matched label
                volumeLabel = el->name;
                delete(v);
                break;
            }
        }

        if (match_found == false)
        {
            // add to volumelist
            YaLuxGlobal.VolumeList.append(v);

            // create new volume block.....
            QString transmission_mapfile = "";
            QString scattering_mapfile = "";

            if (m_SSS_tint != QColor(255, 255, 255))
            {
                m_TransmissionColor.setRedF(m_TransmissionColor.redF() * m_SSS_tint.redF());
                m_TransmissionColor.setGreenF(m_TransmissionColor.greenF() * m_SSS_tint.greenF());
                m_TransmissionColor.setBlueF(m_TransmissionColor.blueF() * m_SSS_tint.blueF());
            }

            if (transmission_mapfile != "")
                m_TranslucencyTex.data += GenerateCoreTextureBlock3(transmissionTexture, transmission_mapfile,
                    m_TransmissionColor.redF(), m_TransmissionColor.greenF(), m_TransmissionColor.blueF());
            else
                transmissionTexture = QString("%1 %2 %3").arg(m_TransmissionColor.redF()).arg(m_TransmissionColor.greenF()).arg(m_TransmissionColor.blueF());

            scatteringTexture = QString("%1 %2 %3").arg(1 - m_ScatteringColor.redF()).arg(1 - m_ScatteringColor.greenF()).arg(1 - m_ScatteringColor.blueF());
            //            scatteringTexture = QString("%1 %2 %3").arg(scattering_color.redF()).arg(scattering_color.greenF()).arg(scattering_color.blueF());
                        //scatteringTexture = "0 0 0";

            // Assume absorption = 1 - transmission
            m_TranslucencyTex.data += QString("scene.textures.%1.type = \"subtract\"\n").arg(absorptionTexture);
            m_TranslucencyTex.data += QString("scene.textures.%1.texture1 = 1 1 1\n").arg(absorptionTexture);
            m_TranslucencyTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(absorptionTexture).arg(transmissionTexture);

            //////////////////////////////////////////
            // scale conversion for Daz Volume parameters to Lux Volume parameters
            //////////////////////////////////////////
            float transmissionDelta = (m_TransmissionDistance - 0.008) / 0.0068;
            float scatteringDelta = (m_ScatteringDistance - 0.15) / 0.135;

            //// transmissionDelta == 0, no recalibration, transmissionDelta == 0.1 --> 
            //float adjustment_a = 1.0;
            //float adjustment_b = 3.125;
            //float inverseTransmissionDelta = 1-transmissionDelta;
            //if (inverseTransmissionDelta < 0) inverseTransmissionDelta = 0;
            //float adjustment = (adjustment_a)*(inverseTransmissionDelta) + (adjustment_b)*(transmissionDelta);
            //transmission_distance*= adjustment;

            // numbers made to multiply by 15/scattering
            //adjustment_a = 1.0;
            //adjustment_b = 5.0;
            //float inverseScatteringDelta = 1-scatteringDelta;
            //if (inverseScatteringDelta < 0) inverseScatteringDelta = 0;
            //adjustment = (adjustment_a)*(inverseScatteringDelta) + (adjustment_b)*(scatteringDelta);
            //scattering_distance *= adjustment;

            // transmissionDelta == 0, no recalibration, transmissionDelta == 0.1 --> 
            float adjustment_a = 0.008;
            float adjustment_b = 0.00425;
            float adjustment = (adjustment_a)+(adjustment_b) * (transmissionDelta);
            m_TransmissionDistance = adjustment;

            adjustment_a = 0.01;
            adjustment_b = 0.005;
            adjustment = (adjustment_a)+(adjustment_b) * (scatteringDelta);
            m_ScatteringDistance = adjustment;


            // scale conversion
            //QColor scaled_absorption_color = QColor(0,0,0);
            //float scale = 1;
            //float v; 
            //float scaled_component;
            //v = transmission_color.redF();
            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
            //scaled_absorption_color.setRedF(scaled_component);
            //v = transmission_color.greenF();
            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
            //scaled_absorption_color.setGreenF(scaled_component);
            //v = transmission_color.blueF();
            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
            //scaled_absorption_color.setBlueF(scaled_component);

            // clamp values
            //float transmission_density = (transmission_density > 1000) ? 1000 : transmission_density;
            //float scattering_density = (scattering_density > 500) ? 500 : scattering_density;

            // Multiply translucency into transmission and scattering
            //QString transmissionTexture_2 = transmissionTexture + "_2";
            //ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(transmissionTexture_2);
            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(transmissionTexture_2).arg(transmissionTexture);
            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(transmissionTexture_2).arg(translucencyTexture);
            //QString scatteringTexture_2 = scatteringTexture + "_2";
            //ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(scatteringTexture_2);
            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(scatteringTexture_2).arg(scatteringTexture);
            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(scatteringTexture_2).arg(translucencyTexture);

            // now scale everything up(down) for volumetric rendering data
            if (YaLuxGlobal.bDoSSSAbsorption)
            {
                m_TranslucencyTex.data += QString("scene.textures.%1.type = \"colordepth\"\n").arg(scaled_transmissionTexture);
                m_TranslucencyTex.data += QString("scene.textures.%1.kt = \"%2\"\n").arg(scaled_transmissionTexture).arg(transmissionTexture);
                m_TranslucencyTex.data += QString("scene.textures.%1.depth = \"%2\"\n").arg(scaled_transmissionTexture).arg(m_TransmissionDistance);
            }
            else
            {
                scaled_transmissionTexture = QString("%1 %1 %1").arg(1 / m_TransmissionDistance);
            }

            if (YaLuxGlobal.bDoSSSScattering)
            {
                m_TranslucencyTex.data += QString("scene.textures.%1.type = \"colordepth\"\n").arg(scaled_scatteringTexture);
                m_TranslucencyTex.data += QString("scene.textures.%1.kt = \"%2\"\n").arg(scaled_scatteringTexture).arg(scatteringTexture);
                m_TranslucencyTex.data += QString("scene.textures.%1.depth = \"%2\"\n").arg(scaled_scatteringTexture).arg(m_ScatteringDistance);
            }
            else
            {
                scaled_scatteringTexture = QString("%1").arg(1 / m_ScatteringDistance);
            }

            // create volume block
            m_TranslucencyTex.data += QString("scene.volumes.%1.type = \"homogeneous\"\n").arg(volumeLabel);
            m_TranslucencyTex.data += QString("scene.volumes.%1.absorption = \"%2\"\n").arg(volumeLabel).arg(scaled_transmissionTexture);
            m_TranslucencyTex.data += QString("scene.volumes.%1.scattering = \"%2\"\n").arg(volumeLabel).arg(scaled_scatteringTexture);
            m_TranslucencyTex.data += QString("scene.volumes.%1.assymetry = \"%2\"\n").arg(volumeLabel).arg(m_ScatteringDirection);
            m_TranslucencyTex.data += QString("scene.volumes.%1.multiscattering = %2\n").arg(volumeLabel).arg(1);
        }

    }
    m_VolumeName = volumeLabel;


    // Opacity Block
    // modify for refraction
    QString OpacityTex = m_LuxMaterialName + "_o";
    m_OpacityTex.name = OpacityTex;
    QString SSSMaskTex0 = m_LuxMaterialName + "_SSS_MASK" + "_0";
    QString SSSMaskTex1 = m_LuxMaterialName + "_SSS_MASK" + "_1";
    double override_opacity;
    // over 0.75, use glass instead of glossytranslucent for refraction
    if (m_RefractionWeight > 0 && m_RefractionWeight < 0.5)
    {
        override_opacity = 1 - (m_RefractionWeight * 0.99);
        if (m_RefractionWeight > 0 && m_OpacityValue == 0) m_OpacityValue = override_opacity;
        m_OpacityValue = (m_OpacityValue < override_opacity) ? m_OpacityValue : override_opacity;
        if (m_TranslucencyExists == false)
        {
            translucencyTexture = "";
            m_TranslucencyExists = true;
        }
    }
    if (m_OpacityExists && m_OpacityMap != "")
        m_OpacityTex.data += GenerateCoreTextureBlock1(OpacityTex, m_OpacityMap, m_OpacityValue,
            m_uscale, m_vscale, m_uoffset, m_voffset);
    else
        OpacityTex = QString("%1 %1 %1").arg(m_OpacityValue);
    if (YaLuxGlobal.bDoSSSVolume && m_VolumeExists && m_RefractionWeight == 0)
    {
        // create SSS mask
        // 1. start with diffuse texture
        // 2. apply/filter-out SSS filter-color (translucency_color)
        // 3. scale down by translucency_weight
        m_OpacityTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(SSSMaskTex0);
        m_OpacityTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(SSSMaskTex0).arg(translucencyTexture);
        m_OpacityTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(SSSMaskTex0).arg(m_TranslucencyWeight);

        m_OpacityTex.data += QString("scene.textures.%1.type = \"subtract\"\n").arg(SSSMaskTex1);
        m_OpacityTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(SSSMaskTex1).arg(OpacityTex);
        m_OpacityTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(SSSMaskTex1).arg(SSSMaskTex0);

        OpacityTex = SSSMaskTex1;
        m_OpacityExists = true;
    }
    //    if (!volume_exists) translucency_exists = false;
    m_OpacityTex.name = OpacityTex;


        // Metallicity
    QString metallicityTex = m_LuxMaterialName + "_metallicity";
    m_MetallicTex.name = metallicityTex;
    if (m_MetallicWeight > 0 && YaLuxGlobal.bDoMetallic)
    {
        //if (metallic_weight == 0) metallic_weight = 0.01;
        float metallicity_scale = m_MetallicWeight;
        //if (glossy_layered_weight > 0) metallicity_scale *= glossy_layered_weight;

        QString filterMetallicityTex = metallicityTex + "_raw_filter";
        QString inverseFitlerMetallicityTex = metallicityTex + "_raw_filter_inverse";
        if (m_MetallicMap != "")
        {
            m_MetallicTex.data += GenerateCoreTextureBlock1(metallicityTex, m_MetallicMap, metallicity_scale,
                m_uscale, m_vscale, m_uoffset, m_voffset);

            // create metal-filter and inverse-metal-fitler
            m_MetallicTex.data += GenerateCoreTextureBlock1(filterMetallicityTex, m_MetallicMap, 1.0,
                m_uscale, m_vscale, m_uoffset, m_voffset);
            m_MetallicTex.data += QString("scene.textures.%1.type = \"subtract\"\n").arg(inverseFitlerMetallicityTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = 1\n").arg(inverseFitlerMetallicityTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(inverseFitlerMetallicityTex).arg(filterMetallicityTex);
        }
        else
        {
            m_MetallicTex.data += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(metallicityTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(metallicityTex).arg(metallicity_scale);
            m_MetallicTex.data += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(filterMetallicityTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(filterMetallicityTex).arg(m_MetallicWeight);
            m_MetallicTex.data += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(inverseFitlerMetallicityTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(inverseFitlerMetallicityTex).arg(1 - m_MetallicWeight);
        }
        /////////////
        // Mix into specular
        ////////////////
        QString specA_metallic_override = metallicityTex + "_override_spec_A";
        QString specB_metallic_override = metallicityTex + "_override_spec_B";
        QString specC_metallic_override = metallicityTex + "_override_spec_C";
        QString specD_metallic_override = metallicityTex + "_override_spec_D";

        // create filtered-metal specular grey: 0.5? 0.1?
        m_MetallicTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(specA_metallic_override);
        m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specA_metallic_override).arg("0.01 0.01 0.01");
        m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specA_metallic_override).arg(filterMetallicityTex);
        // merge with metal-filtered, subtracted specular from above
        //if (spec_weight > 0 || glossy_layered_weight > 0)
        if (false)
        {
            // create black out or scale down metal-filtered specular
            m_MetallicTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(specB_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specB_metallic_override).arg(mainSpec);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specB_metallic_override).arg(inverseFitlerMetallicityTex);

            // add A + B
            m_MetallicTex.data += QString("scene.textures.%1.type = \"add\"\n").arg(specC_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specC_metallic_override).arg(specA_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specC_metallic_override).arg(specB_metallic_override);

            // add metallized+colored specular
            // 1. create metallic-filtered diffuse texture
            // use the true scaled metallicity instead of the raw filtered metallicity
            QString diffFilter = metallicityTex + "_diffuse_filtered";
            m_MetallicTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(diffFilter);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffFilter).arg(mainDiffTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffFilter).arg(metallicityTex);
            // 2. add to subtracted specular
            m_MetallicTex.data += QString("scene.textures.%1.type = \"add\"\n").arg(specD_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specD_metallic_override).arg(specC_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specD_metallic_override).arg(diffFilter);
            // clamp
            QString clamped_metallic_override = specD_metallic_override + "_clamped";
            m_MetallicTex.data += QString("scene.textures.%1.type = \"clamp\"\n").arg(clamped_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(clamped_metallic_override).arg(specD_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.min = 0\n").arg(clamped_metallic_override);
            m_MetallicTex.data += QString("scene.textures.%1.max = 1\n").arg(clamped_metallic_override);

            mainSpec = clamped_metallic_override;
        }
        else
        {
            QString diffFilter = metallicityTex + "_diffuse_filtered";
            m_MetallicTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(diffFilter);
            m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffFilter).arg(mainDiffTex);
            m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffFilter).arg(metallicityTex);

            //// add A + B
            //ret_str += QString("scene.textures.%1.type = \"add\"\n").arg(specC_metallic_override);
            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specC_metallic_override).arg(specA_metallic_override);
            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specC_metallic_override).arg(diffFilter);

            //// clamp
            //QString clamped_metallic_override = specC_metallic_override + "_clamped";
            //ret_str += QString("scene.textures.%1.type = \"clamp\"\n").arg(clamped_metallic_override);
            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(clamped_metallic_override).arg(specC_metallic_override);
            //ret_str += QString("scene.textures.%1.min = 0\n").arg(clamped_metallic_override);
            //ret_str += QString("scene.textures.%1.max = 1\n").arg(clamped_metallic_override);

            mainSpec = diffFilter;
        }
        m_SpecularTex.name = mainSpec;
        m_SpecularExists = true;

        /////////////
        // Subtract from Diffuse
        /////////////
        QString diffuseA_metallic_override = metallicityTex + "_override_diff_A";
        QString diffuseB_metallic_override = metallicityTex + "_override_diff_B"; // not needed?
        QString inverseMetallicityTex = metallicityTex + "_inverse";

        // create proper inverse of scaled metallicity
        m_MetallicTex.data += QString("scene.textures.%1.type = \"subtract\"\n").arg(inverseMetallicityTex);
        m_MetallicTex.data += QString("scene.textures.%1.texture1 = 1\n").arg(inverseMetallicityTex);
        m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(inverseMetallicityTex).arg(metallicityTex);

        // black out or scale down metal-filtered diffuse
        m_MetallicTex.data += QString("scene.textures.%1.type = \"scale\"\n").arg(diffuseA_metallic_override);
        m_MetallicTex.data += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffuseA_metallic_override).arg(mainDiffTex);
        m_MetallicTex.data += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffuseA_metallic_override).arg(inverseMetallicityTex);

        //// rename maindiff
        mainDiffTex = diffuseA_metallic_override;
        m_DiffuseTex.name = mainDiffTex;
    }


	return true;
}

bool IrayUberToLuxCoreMaterial::CreateMaterials()
{
    QString ret_str = "";
    QString matLabel = m_LuxMaterialName;

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
        if (m_SpecularExists)
            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
        else
            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(matLabel);
        if (m_BumpExists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
        //        if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(matLabel).arg(matLabel + "_n");
        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(m_Roughness);
        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(m_Roughness);
        //if (metallic_weight != 0 || metallic_mapfile != "") ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(metallic_weight);
    }
    else
    {
        QString glossy2Label = matLabel;

        if (m_TranslucencyExists || m_RefractionWeight > 0)
        {
            if (YaLuxGlobal.bDoDebugSSS && m_VolumeExists)
                ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(glossy2Label);
            else if (m_RefractionWeight >= 0.5)
                ret_str += QString("scene.materials.%1.type = \"glass\"\n").arg(glossy2Label);
            else if (m_TranslucencyExists && YaLuxGlobal.bDoTranslucency)
                ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(glossy2Label);
            else
                ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(glossy2Label);
            if (YaLuxGlobal.bDoSSSVolume && m_VolumeExists && m_RefractionWeight == 0)
                ret_str += QString("scene.materials.%1.volume.interior = \"%2\"\n").arg(glossy2Label).arg(m_VolumeName);
            if (YaLuxGlobal.bDoTranslucency && m_TranslucencyExists)
            {
                ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(glossy2Label).arg(m_TranslucencyTex.name);
            }
        }
        else
        {
            ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(glossy2Label);
        }
        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(glossy2Label).arg(m_DiffuseTex.name);

        if ((m_SpecularExists || m_GlossyLayeredWeight > 0 || m_MetallicWeight > 0) && YaLuxGlobal.bDoSpecular)
        {
            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(glossy2Label).arg(m_SpecularTex.name);
            if (YaLuxGlobal.bDoTranslucency && m_TranslucencyExists)
                ret_str += QString("scene.materials.%1.ks_bf = \"%2\"\n").arg(glossy2Label).arg(m_SpecularTex.name);
        }
        else
        {
            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(glossy2Label);
            if (YaLuxGlobal.bDoTranslucency && m_TranslucencyExists)
                ret_str += QString("scene.materials.%1.ks_bf = 0 0 0\n").arg(glossy2Label);
        }

        if (m_BumpExists && YaLuxGlobal.bDoBumpMaps)
        {
            ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_b");
            //ret_str += QString("scene.materials.%1.bumpsamplingdistance = \"%2\"\n").arg(glossy2Label).arg(1 / 1000000);
        }
        if (m_NormalMap != "" && YaLuxGlobal.bDoNormalMaps)
            ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_n");


        if (m_SpecularWeight > 0 && YaLuxGlobal.bDoSpecular)
        {
            // use dual lobe specular for front and backface

            ret_str += QString("scene.materials.%1.uroughness = \"%2\"\n").arg(glossy2Label).arg(m_SpecRoughness_1);
            ret_str += QString("scene.materials.%1.vroughness = \"%2\"\n").arg(glossy2Label).arg(m_SpecRoughness_1);
            if (YaLuxGlobal.bDoSpecular && m_TranslucencyExists)
            {
                ret_str += QString("scene.materials.%1.uroughness_bf = \"%2\"\n").arg(glossy2Label).arg(m_SpecRoughness_2);
                ret_str += QString("scene.materials.%1.vroughness_bf = \"%2\"\n").arg(glossy2Label).arg(m_SpecRoughness_2);
            }
        }
        else if (m_GlossyRoughnessMap != "" && m_GlossyLayeredWeight > 0)
        {
            //QString glossyRoughnessTexLabel = glossy2Label + "_glossyRoughnessTex";
            //ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(glossyRoughnessTexLabel);
            //ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(glossyRoughnessTexLabel).arg(glossy_roughness_mapfile);
            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(m_GlossyRoughnessTex.name);
            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(m_GlossyRoughnessTex.name);
        }
        else
        {
            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(m_Roughness);
            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(m_Roughness);
        }

        if (YaLuxGlobal.bDoDebugSSS && m_VolumeExists)
            ret_str += QString("scene.materials.%1.transparency = 1\n").arg(glossy2Label);
        else if (m_OpacityExists)
            ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(glossy2Label).arg(m_OpacityTex.name);
        else if (m_OpacityValue != 1)
            ret_str += QString("scene.materials.%1.transparency = %2\n").arg(glossy2Label).arg(m_OpacityValue);

    }

    m_PrimaryMaterialBlock.name = matLabel;
    m_PrimaryMaterialBlock.data = ret_str;

	return true;
}

QString IrayUberToLuxCoreMaterial::toString()
{
    QString ret_str;

    // add texture blocks
    ret_str += m_DiffuseTex.data;
    ret_str += m_SpecularTex.data;
    ret_str += m_BumpTex.data;

    ret_str += m_GlossyTex.data;
    ret_str += m_GlossyRoughnessTex.data;
    ret_str += m_NormalTex.data;
    ret_str += m_TranslucencyTex.data;

    ret_str += m_OpacityTex.data;

    ret_str += m_MetallicTex.data;
    // TODO: more blocks????

    // add material block
    ret_str += m_PrimaryMaterialBlock.data;

    return ret_str;
}

#include "moc_IrayUberToLuxCoreMaterial.cpp"