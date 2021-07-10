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

//QString LuxCoreProcessIrayUberMaterial(DzMaterial* material, QString& mesg, QString matLabel)
//{
//    QString ret_str = "# MATERIAL " + matLabel + "\n";
//
//    // diffuse image and color
//    float diffuse_vscale = -1;
//    float diffuse_uscale = 1;
//    float diffuse_gamma = 2.2;
//    float diffuse_voffset = 0; // vdelta
//    float diffuse_uoffset = 0; // udelta
//    QString diffuse_wrap = "repeat"; // repeat|black|clamp
//    QString diffuse_filtertype = "bilinear";
//    QString diffuse_channel = "";
//    QString diffuse_mapfile = ""; // Diffuse Color
//    QColor diffuse_value;
//    bool diffuse_exists = false;
//
//    // specular image and color
//    float spec_vscale = -1;
//    float spec_uscale = 1;
//    float spec_gamma = 2.2;
//    float spec_voffset = 0;
//    float spec_uoffset = 0;
//    QString spec_wrap = "repeat"; // repeat|black|clamp
//    QString spec_filtertype = "bilinear";
//    QString spec_channel = "";
//    QString specweight_mapfile = ""; // Specular [Dual Lobe Specular]
//    bool spec_exists = false;
//
//    // bump image and values
//    float bump_vscale = -1;
//    float bump_uscale = 1;
//    float bump_gamma = 1;
//    float bump_voffset = 0;
//    float bump_uoffset = 0;
//    QString bump_channel = "";
//    QString bump_wrap = "repeat";
//    QString bump_filtertype = "bilinear";
//    QString bump_mapfile = ""; // "Bump Strength"
//    float bump_value;
//    bool bump_exists = false;
//
//    // transmission map
//    QString opacity_mapfile = "";
//    float opacity_value = 1;
//    bool opacity_exists = false;
//
//    // material definition
//    float uroughness = 0.8;
//    float vroughness = 0.8;
//    float index_refraction = 0.0; // IOR
//
//    float metallic_weight = 0.0;
//    QString metallic_mapfile = ""; // "Metallic Weight"
//
//    float spec_weight = 0;
//    float refraction_weight = 0;
//    float translucency_weight = 0;
//    float glossy_layered_weight = 0;
//
//    // translucency (volume rendering)
//    QColor translucency_color; // "Translucency Color"
//    QString translucency_mapfile = ""; // "Translucency Color"
//    QColor sss_tint = QColor(255, 255, 255); // "SSS Reflectance Tint"
//    bool translucency_exists = false;
//    bool volume_exists = false;
//    /////////////////// Volume
//    float transmission_distance = 0; // "Transmitted Measurement Distance"
//    QColor transmission_color; // "Transmitted Color"
//    float scattering_distance = 0; // "Scattering Measurement Distance"
//    QColor scattering_color; // "SSS Color"
//    float scattering_direction; // "SSS Direction" == assymetry in luxcore
//
//    // normal map
//    float normal_strength = 0; // "Normal Map"
//    QString normal_mapfile = "";
//
//    // glossy layer
//    float glossy_roughness = 0;
//    QString glossy_roughness_mapfile = "";
//    float glossy_reflectivity = 0;
//    QColor glossy_color;
//    QString glossy_mapfile = "";
//
//    // cheats
//    float glossy_anisotropy = 0;
//
//
//    enum { metal_roughness, specular_glossy } material_type = specular_glossy;
//
//    QString propertyLabel;
//    DzProperty* currentProperty;
//
//    // Matte vs Glossy
//    currentProperty = material->findProperty("Base Mixing");
//    if (currentProperty != NULL)
//    {
//        QString base_mixing = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
//        if (base_mixing.contains("metal"))
//        {
//            material_type = metal_roughness;
//        }
//        else if (base_mixing.contains("specular"))
//        {
//            material_type = specular_glossy;
//        }
//    }
//
//    currentProperty = material->findProperty("Diffuse Color");
//    if (currentProperty != NULL)
//    {
//        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        diffuse_exists = true;
//    }
//    currentProperty = material->findProperty("Horizontal Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
//        spec_uscale = diffuse_uscale;
//        bump_uscale = diffuse_uscale;
//    }
//    currentProperty = material->findProperty("Vertical Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
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
//    currentProperty = material->findProperty("Dual Lobe Specular Weight");
//    if (currentProperty != NULL)
//    {
//        spec_weight = ((DzFloatProperty*)currentProperty)->getValue();
//        specweight_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ((spec_weight != 0) || (specweight_mapfile != ""))
//            spec_exists = true;
//    }
//    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
//    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
//    currentProperty = material->findProperty("Bump Strength");
//    if (currentProperty != NULL)
//    {
//        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 500;
//        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if (bump_mapfile != "")
//            bump_exists = true;
//    }
//    currentProperty = material->findProperty("Normal Map");
//    if (currentProperty != NULL)
//    {
//        normal_strength = ((DzFloatProperty*)currentProperty)->getValue();
//        normal_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//    }
//    currentProperty = material->findProperty("Refraction Weight");
//    if (currentProperty != NULL)
//    {
//        refraction_weight = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Refraction Index"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        index_refraction = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Cutout Opacity"); // cutout opacity
//    if (currentProperty != NULL)
//    {
//        opacity_value = ((DzFloatProperty*)currentProperty)->getValue();
//        opacity_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ((opacity_value != 1) || (opacity_mapfile != ""))
//            opacity_exists = true;
//    }
//    ////////  Glossy Layer //////////////
//    currentProperty = material->findProperty("Glossy Layered Weight"); // glossy layered weight
//    if (currentProperty != NULL)
//    {
//        glossy_layered_weight = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Glossy Color");
//    if (currentProperty != NULL)
//    {
//        glossy_color = ((DzColorProperty*)currentProperty)->getColorValue();
//        glossy_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//    }
//    currentProperty = material->findProperty("Glossy Reflectivity"); // glossy reflectivity
//    if (currentProperty != NULL)
//    {
//        glossy_reflectivity = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
//    }
//    currentProperty = material->findProperty("Glossy Roughness"); // glossy roughness
//    if (currentProperty != NULL)
//    {
//        glossy_roughness = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
//        glossy_roughness_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//    }
//    if (material_type == metal_roughness)
//    {
//        currentProperty = material->findProperty("Metallic Weight"); // metallicity
//        if (currentProperty != NULL)
//        {
//            metallic_weight = ((DzFloatProperty*)currentProperty)->getValue();
//            metallic_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        }
//    }
//    if (material_type == metal_roughness)
//    {
//        currentProperty = material->findProperty("Glossy Roughness");
//        if (currentProperty != NULL)
//        {
//            // Linear Interpolation: roughness == 1.0 when glossy_layered_weight == 0%
//            uroughness = 1.0 * (1 - glossy_layered_weight) + ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
//            if (uroughness > 0.8) uroughness = 0.8;
//            vroughness = uroughness;
//
//            //if (material_type == specular_glossy)
//            //{
//            //    uroughness = (1 - uroughness > 0.8) ? 0.8 : (1 - uroughness);
//            //    vroughness = uroughness;
//            //}
//        }
//    }
//    else
//    {
//        currentProperty = material->findProperty("Glossiness");
//        if (currentProperty != NULL)
//        {
//            // simple linear interpolation: glossiness == 0 when glossy_layered_weight == 0%
//            uroughness = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
//            uroughness = (1 - uroughness > 0.8) ? 0.8 : (1 - uroughness);
//            vroughness = uroughness;
//        }
//    }
//    currentProperty = material->findProperty("Glossy Anisotropy"); // index of refreaction
//    if (currentProperty != NULL)
//    {
//        glossy_anisotropy = ((DzFloatProperty*)currentProperty)->getValue() * glossy_layered_weight;
//        if (glossy_anisotropy > 0)
//        {
//            if (material_type == metal_roughness)
//            {
//                uroughness = (0.8 * glossy_anisotropy) + (uroughness * (1 - glossy_anisotropy));
//                vroughness = uroughness;
//            }
//            else if (material_type == specular_glossy)
//            {
//                uroughness = (uroughness * (1 - glossy_anisotropy));
//                vroughness = uroughness;
//            }
//        }
//    }
//    currentProperty = material->findProperty("Translucency Weight");
//    if (currentProperty != NULL)
//    {
//        translucency_weight = ((DzFloatProperty*)currentProperty)->getValue();
//        if (translucency_weight != 0)
//        {
//            translucency_exists = true;
//            currentProperty = material->findProperty("Translucency Color");
//            if (currentProperty != NULL)
//            {
//                translucency_color = ((DzColorProperty*)currentProperty)->getColorValue();
//                translucency_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//            }
//            currentProperty = material->findProperty("SSS Reflectance Tint");
//            if (currentProperty != NULL)
//            {
//                sss_tint = ((DzColorProperty*)currentProperty)->getColorValue();
//            }
//            /////////////////// volume rendering
//            currentProperty = material->findProperty("Transmitted Color");
//            if (currentProperty != NULL)
//            {
//                transmission_color = ((DzColorProperty*)currentProperty)->getColorValue();
//                if (transmission_color != QColor(0, 0, 0))
//                {
//                    volume_exists = true;
//                }
//            }
//            currentProperty = material->findProperty("SSS Color");
//            if (currentProperty != NULL)
//            {
//                scattering_color = ((DzColorProperty*)currentProperty)->getColorValue();
//            }
//            currentProperty = material->findProperty("Transmitted Measurement Distance");
//            if (currentProperty != NULL)
//            {
//                // I think Daz values are in cm, so divide by 100 to get meters for lux?
//                transmission_distance = ((DzFloatProperty*)currentProperty)->getValue() / 100;
//            }
//            currentProperty = material->findProperty("Scattering Measurement Distance");
//            if (currentProperty != NULL)
//            {
//                scattering_distance = ((DzFloatProperty*)currentProperty)->getValue() / 1;
//            }
//            currentProperty = material->findProperty("SSS Direction");
//            if (currentProperty != NULL)
//            {
//                scattering_direction = ((DzFloatProperty*)currentProperty)->getValue();
//            }
//
//        }
//    }
//
//    // TODO: check for metallicity = 0 vs 1.... see metallicity below
//    // Diffuse Texture Block
//    QString mainDiffTex = matLabel + "_d";
//    if (diffuse_exists)
//        ret_str += GenerateCoreTextureBlock3(mainDiffTex, diffuse_mapfile,
//            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
//            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//            diffuse_gamma, diffuse_wrap, diffuse_channel);
//
//
//
//    // Specular Block (DUAL LOBE)
//    float spec1_float = 0;
//    float spec2_float = 0;
//    float spec_ratio = 0;
//    float spec_reflectivity = 0;
//    QString spec1_mapfile = "";
//    QString spec2_mapfile = "";
//    QString specratio_mapfile = "";
//    QString specref_mapfile = "";
//
//    QString mainSpec = matLabel + "_s";
//    QString specref_label = mainSpec + "_spec_reflect";
//    QString rawDualRoughness = mainSpec + "_raw_rough";
//    QString scaledSpec1Roughness = mainSpec + "_scaled_rough1";
//    QString scaledDualRoughness = mainSpec + "_scaled_rough2";
//    QString spec1_label = rawDualRoughness + "_spec1";
//    QString spec2_label = rawDualRoughness + "_spec2";
//    QString specratio_label = rawDualRoughness + "_specratio";
//
//    if (spec_weight > 0 && YaLuxGlobal.bDoSpecular)
//    {
//
//        LuxGetFloatProperty(material, "Specular Lobe 1 Roughness", spec1_float, mesg);
//        spec1_mapfile = LuxGetImageMapProperty(material, "Specular Lobe 1 Roughness", mesg);
//        LuxGetFloatProperty(material, "Specular Lobe 2 Roughness", spec2_float, mesg);
//        spec2_mapfile = LuxGetImageMapProperty(material, "Specular Lobe 2 Roughness", mesg);
//        LuxGetFloatProperty(material, "Dual Lobe Specular Ratio", spec_ratio, mesg);
//        specratio_mapfile = LuxGetImageMapProperty(material, "Dual Lobe Specular Ratio", mesg);
//        LuxGetFloatProperty(material, "Dual Lobe Specular Reflectivity", spec_reflectivity, mesg);
//        specref_mapfile = LuxGetImageMapProperty(material, "Dual Lobe Specular Reflectivity", mesg);
//
//        // generate texture 1, 2, ratio, reflectivity
//        if (specweight_mapfile != "")
//            ret_str += GenerateCoreTextureBlock1(mainSpec + "_weight", specweight_mapfile, spec_weight);
//        if (spec1_mapfile != "")
//            ret_str += GenerateCoreTextureBlock1(spec1_label, spec1_mapfile, spec1_float);
//        if (spec2_mapfile != "")
//            ret_str += GenerateCoreTextureBlock1(spec2_label, spec1_mapfile, spec2_float);
//        if (specratio_mapfile != "")
//            ret_str += GenerateCoreTextureBlock1(specratio_label, specratio_mapfile, spec_ratio);
//        if (specref_mapfile != "")
//            //            ret_str += GenerateCoreTextureBlock1(specref_label, specref_mapfile, spec_reflectivity);
//            ret_str += GenerateCoreTextureBlock1(specref_label, specref_mapfile, 1.0);
//
//        // mix spec1 + spec2
//        ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(rawDualRoughness);
//        if (spec1_mapfile != "")
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(rawDualRoughness).arg(spec1_label);
//        else
//            ret_str += QString("scene.textures.%1.texture1 = %2\n").arg(rawDualRoughness).arg(spec1_float);
//        if (spec2_mapfile != "")
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(rawDualRoughness).arg(spec2_label);
//        else
//            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(rawDualRoughness).arg(spec2_float);
//        if (specratio_mapfile != "")
//            ret_str += QString("scene.textures.%1.amount = \"%2\"\n").arg(rawDualRoughness).arg(specratio_label);
//        else
//            ret_str += QString("scene.textures.%1.amount = %2\n").arg(rawDualRoughness).arg(spec_ratio);
//
//        // scale roughness from grey (0.5)
//        ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(scaledDualRoughness);
//        ret_str += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(scaledDualRoughness);
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(scaledDualRoughness).arg(rawDualRoughness);
//        if (specweight_mapfile != "")
//            ret_str += QString("scene.textures.%1.amount = \"%2\"\n").arg(scaledDualRoughness).arg(mainSpec + "_weight");
//        else
//            ret_str += QString("scene.textures.%1.amount = %2\n").arg(scaledDualRoughness).arg(spec_weight);
//
//        // scale roughness from grey (0.5)
//        ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(scaledSpec1Roughness);
//        ret_str += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(scaledSpec1Roughness);
//        if (spec1_mapfile != "")
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(scaledSpec1Roughness).arg(spec1_label);
//        else
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(scaledSpec1Roughness).arg(spec1_float);
//        if (specweight_mapfile != "")
//            ret_str += QString("scene.textures.%1.amount = \"%2\"\n").arg(scaledSpec1Roughness).arg(mainSpec + "_weight");
//        else
//            ret_str += QString("scene.textures.%1.amount = %2\n").arg(scaledSpec1Roughness).arg(spec_weight);
//
//
//        // mix specweight
//        //if (specref_mapfile != "")
//        //{
//        //    //// no scaling of specular reflectivity
//        //    //mainSpec = QString("%1 %1 %1").arg(spec_reflectivity);
//        //    //if (specref_mapfile != "") mainSpec = specref_label;
//        //    mainSpec = specref_label;
//        //}
//        QString finalMix = QString("%1 %1 %1").arg(spec_reflectivity);
//        if (specref_mapfile != "") finalMix = specref_label;
//        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(mainSpec);
//        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(mainSpec).arg(finalMix);
//        if (specweight_mapfile != "")
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(mainSpec).arg(mainSpec + "_weight");
//        else
//            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(mainSpec).arg(spec_weight);
//
//    }
//    else
//    {
//        mainSpec = "0 0 0";
//    }
//
//    // Glossy Layer
//    QString glossyRoughnessTexLabel = matLabel + "_glossyRoughnessTex";
//    if (glossy_layered_weight > 0)
//    {
//        if (YaLuxGlobal.bDoSpecular)
//        {
//            QString glossyTex = matLabel + "_glossy";
//            if (glossy_mapfile != "")
//                ret_str += GenerateCoreTextureBlock3(glossyTex, glossy_mapfile,
//                    glossy_color.redF(), glossy_color.greenF(), glossy_color.blueF(),
//                    diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//                    diffuse_gamma, diffuse_wrap, diffuse_channel);
//            else
//                glossyTex = QString("%1 %2 %3").arg(glossy_color.redF()).arg(glossy_color.greenF()).arg(glossy_color.blueF());
//
//            // mix glossy with mainSpec (dual spec)
//            QString glossyMixTex = matLabel + "_glossy" + "_mix";
//
//            ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(glossyMixTex);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(glossyMixTex).arg(mainSpec);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(glossyMixTex).arg(glossyTex);
//            ret_str += QString("scene.textures.%1.amount = \"%2\"\n").arg(glossyMixTex).arg(glossy_layered_weight * 0.05);
//            mainSpec = glossyMixTex;
//        }
//
//        if (glossy_roughness_mapfile != "")
//        {
//            // note: roughness is already multiplied by glossy_layered_weight when imported above
//            ret_str += GenerateCoreTextureBlock1(glossyRoughnessTexLabel, glossy_roughness_mapfile, glossy_roughness);
//        }
//
//    }
//
//    // Bumpmap Block
//    QString bumpMapName = matLabel + "_b";
//    if (matLabel.toLower().contains("face") || matLabel.toLower().contains("lip") || matLabel.toLower().contains("ear"))
//        bump_value /= 4;
//    if (bump_exists && YaLuxGlobal.bDoBumpMaps)
//        ret_str += GenerateCoreTextureBlock1_Grey(bumpMapName, bump_mapfile, bump_value,
//            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//            bump_wrap, bump_channel);
//
//    // Normalmap Block
//    QString normalMapName = matLabel + "_n";
//    QString imageMapName = normalMapName + "_t";
//    if (normal_mapfile != "" && YaLuxGlobal.bDoNormalMaps)
//    {
//        float scale = normal_strength * 1.0; // Multiply by any necessary render engine correction here
//
//        if (scale != 1)
//        {
//            ret_str += GenerateCoreTextureBlock1(imageMapName, normal_mapfile, 1.0,
//                bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                bump_wrap, bump_channel);
//            ret_str += QString("scene.textures.%1.type = \"normalmap\"\n").arg(normalMapName);
//            ret_str += QString("scene.textures.%1.texture = \"%2\"\n").arg(normalMapName).arg(imageMapName);
//            ret_str += QString("scene.textures.%1.scale = \"%2\"\n").arg(normalMapName).arg(scale);
//        }
//        else
//        {
//            ret_str += GenerateCoreTextureBlock1(normalMapName, normal_mapfile, 1.0,
//                bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//                bump_wrap, bump_channel);
//        }
//    }
//
//    // TranslucencyMap Block
//    QString translucencyTexture = matLabel + "_translucency";
//    QString transmissionTexture = matLabel + "_transmission";
//    QString absorptionTexture = matLabel + "_absorption";
//    QString scatteringTexture = matLabel + "_scattering";
//
//    QString scaled_transmissionTexture = matLabel + "_transmission_scaled";
//    QString scaled_absorptionTexture = matLabel + "_absorption_scaled";
//    QString scaled_scatteringTexture = matLabel + "_scattering_scaled";
//
//    QString volumeLabel = matLabel + "_volume";
//    if (translucency_exists && YaLuxGlobal.bDoTranslucency)
//    {
//        ret_str += GenerateCoreTextureBlock3(translucencyTexture, translucency_mapfile,
//            translucency_color.redF(), translucency_color.greenF(), translucency_color.blueF(),
//            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//            diffuse_gamma, diffuse_wrap, diffuse_channel);
//    }
//    if (volume_exists && YaLuxGlobal.bDoSSSVolume && refraction_weight == 0)
//    {
//        // create volumedata and check against volumelist
//        VolumeData* v = new VolumeData();
//        v->name = volumeLabel;
//        v->type = "homogeneous";
//        v->transmission_color = transmission_color.value();
//        v->transmission_distance = transmission_distance;
//        v->scattering_color = scattering_color.value();
//        v->scattering_distance = scattering_distance;
//        v->asymmetry_val = scattering_direction;
//        v->multiscattering = true;
//
//        // search volumelist
//        bool match_found = false;
//        for (QList<VolumeData*>::iterator el_iter = YaLuxGlobal.VolumeList.begin(); el_iter != YaLuxGlobal.VolumeList.end(); el_iter++)
//        {
//            VolumeData* el = *el_iter;
//            if (*v == *el)
//            {
//                match_found = true;
//                // change volumeLabel to matched label
//                volumeLabel = el->name;
//                delete(v);
//                break;
//            }
//        }
//
//        if (match_found == false)
//        {
//            // add to volumelist
//            YaLuxGlobal.VolumeList.append(v);
//
//            // create new volume block.....
//            QString transmission_mapfile = "";
//            QString scattering_mapfile = "";
//
//            if (sss_tint != QColor(255, 255, 255))
//            {
//                transmission_color.setRedF(transmission_color.redF() * sss_tint.redF());
//                transmission_color.setGreenF(transmission_color.greenF() * sss_tint.greenF());
//                transmission_color.setBlueF(transmission_color.blueF() * sss_tint.blueF());
//            }
//
//            if (transmission_mapfile != "")
//                ret_str += GenerateCoreTextureBlock3(transmissionTexture, transmission_mapfile,
//                    transmission_color.redF(), transmission_color.greenF(), transmission_color.blueF());
//            else
//                transmissionTexture = QString("%1 %2 %3").arg(transmission_color.redF()).arg(transmission_color.greenF()).arg(transmission_color.blueF());
//
//            scatteringTexture = QString("%1 %2 %3").arg(1 - scattering_color.redF()).arg(1 - scattering_color.greenF()).arg(1 - scattering_color.blueF());
//            //            scatteringTexture = QString("%1 %2 %3").arg(scattering_color.redF()).arg(scattering_color.greenF()).arg(scattering_color.blueF());
//                        //scatteringTexture = "0 0 0";
//
//                        // Assume absorption = 1 - transmission
//            ret_str += QString("scene.textures.%1.type = \"subtract\"\n").arg(absorptionTexture);
//            ret_str += QString("scene.textures.%1.texture1 = 1 1 1\n").arg(absorptionTexture);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(absorptionTexture).arg(transmissionTexture);
//
//            //////////////////////////////////////////
//            // scale conversion for Daz Volume parameters to Lux Volume parameters
//            //////////////////////////////////////////
//            float transmissionDelta = (transmission_distance - 0.008) / 0.0068;
//            float scatteringDelta = (scattering_distance - 0.15) / 0.135;
//
//            //// transmissionDelta == 0, no recalibration, transmissionDelta == 0.1 --> 
//            //float adjustment_a = 1.0;
//            //float adjustment_b = 3.125;
//            //float inverseTransmissionDelta = 1-transmissionDelta;
//            //if (inverseTransmissionDelta < 0) inverseTransmissionDelta = 0;
//            //float adjustment = (adjustment_a)*(inverseTransmissionDelta) + (adjustment_b)*(transmissionDelta);
//            //transmission_distance*= adjustment;
//
//            // numbers made to multiply by 15/scattering
//            //adjustment_a = 1.0;
//            //adjustment_b = 5.0;
//            //float inverseScatteringDelta = 1-scatteringDelta;
//            //if (inverseScatteringDelta < 0) inverseScatteringDelta = 0;
//            //adjustment = (adjustment_a)*(inverseScatteringDelta) + (adjustment_b)*(scatteringDelta);
//            //scattering_distance *= adjustment;
//
//            // transmissionDelta == 0, no recalibration, transmissionDelta == 0.1 --> 
//            float adjustment_a = 0.008;
//            float adjustment_b = 0.00425;
//            float adjustment = (adjustment_a)+(adjustment_b) * (transmissionDelta);
//            transmission_distance = adjustment;
//
//            adjustment_a = 0.01;
//            adjustment_b = 0.005;
//            adjustment = (adjustment_a)+(adjustment_b) * (scatteringDelta);
//            scattering_distance = adjustment;
//
//
//            // scale conversion
//            //QColor scaled_absorption_color = QColor(0,0,0);
//            //float scale = 1;
//            //float v; 
//            //float scaled_component;
//            //v = transmission_color.redF();
//            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
//            //scaled_absorption_color.setRedF(scaled_component);
//            //v = transmission_color.greenF();
//            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
//            //scaled_absorption_color.setGreenF(scaled_component);
//            //v = transmission_color.blueF();
//            //scaled_component = (-log(max(v, pow(1, -30))) / transmission_distance) * scale * (v == 1.0 && -1 || 1);
//            //scaled_absorption_color.setBlueF(scaled_component);
//
//            // clamp values
//            //float transmission_density = (transmission_density > 1000) ? 1000 : transmission_density;
//            //float scattering_density = (scattering_density > 500) ? 500 : scattering_density;
//
//            // Multiply translucency into transmission and scattering
//            //QString transmissionTexture_2 = transmissionTexture + "_2";
//            //ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(transmissionTexture_2);
//            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(transmissionTexture_2).arg(transmissionTexture);
//            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(transmissionTexture_2).arg(translucencyTexture);
//            //QString scatteringTexture_2 = scatteringTexture + "_2";
//            //ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(scatteringTexture_2);
//            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(scatteringTexture_2).arg(scatteringTexture);
//            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(scatteringTexture_2).arg(translucencyTexture);
//
//            // now scale everything up(down) for volumetric rendering data
//            if (YaLuxGlobal.bDoSSSAbsorption)
//            {
//                ret_str += QString("scene.textures.%1.type = \"colordepth\"\n").arg(scaled_transmissionTexture);
//                ret_str += QString("scene.textures.%1.kt = \"%2\"\n").arg(scaled_transmissionTexture).arg(transmissionTexture);
//                ret_str += QString("scene.textures.%1.depth = \"%2\"\n").arg(scaled_transmissionTexture).arg(transmission_distance);
//            }
//            else
//            {
//                scaled_transmissionTexture = QString("%1 %1 %1").arg(1 / transmission_distance);
//            }
//
//            if (YaLuxGlobal.bDoSSSScattering)
//            {
//                ret_str += QString("scene.textures.%1.type = \"colordepth\"\n").arg(scaled_scatteringTexture);
//                ret_str += QString("scene.textures.%1.kt = \"%2\"\n").arg(scaled_scatteringTexture).arg(scatteringTexture);
//                ret_str += QString("scene.textures.%1.depth = \"%2\"\n").arg(scaled_scatteringTexture).arg(scattering_distance);
//            }
//            else
//            {
//                scaled_scatteringTexture = QString("%1").arg(1 / scattering_distance);
//            }
//
//            // create volume block
//            ret_str += QString("scene.volumes.%1.type = \"homogeneous\"\n").arg(volumeLabel);
//            ret_str += QString("scene.volumes.%1.absorption = \"%2\"\n").arg(volumeLabel).arg(scaled_transmissionTexture);
//            ret_str += QString("scene.volumes.%1.scattering = \"%2\"\n").arg(volumeLabel).arg(scaled_scatteringTexture);
//            ret_str += QString("scene.volumes.%1.assymetry = \"%2\"\n").arg(volumeLabel).arg(scattering_direction);
//            ret_str += QString("scene.volumes.%1.multiscattering = %2\n").arg(volumeLabel).arg(1);
//        }
//
//    }
//
//
//    // Opacity Block
//    // modify for refraction
//    QString OpacityTex = matLabel + "_o";
//    QString SSSMaskTex0 = matLabel + "_SSS_MASK" + "_0";
//    QString SSSMaskTex1 = matLabel + "_SSS_MASK" + "_1";
//    double override_opacity;
//    // over 0.75, use glass instead of glossytranslucent for refraction
//    if (refraction_weight > 0 && refraction_weight < 0.5)
//    {
//        override_opacity = 1 - (refraction_weight * 0.99);
//        if (refraction_weight > 0 && opacity_value == 0) opacity_value = override_opacity;
//        opacity_value = (opacity_value < override_opacity) ? opacity_value : override_opacity;
//        if (translucency_exists == false)
//        {
//            translucencyTexture = "";
//            translucency_exists = true;
//        }
//    }
//    if (opacity_exists && opacity_mapfile != "")
//        ret_str += GenerateCoreTextureBlock1(OpacityTex, opacity_mapfile, opacity_value,
//            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//            bump_wrap, bump_channel);
//    else
//        OpacityTex = QString("%1 %1 %1").arg(opacity_value);
//    if (YaLuxGlobal.bDoSSSVolume && volume_exists && refraction_weight == 0)
//    {
//        // create SSS mask
//        // 1. start with diffuse texture
//        // 2. apply/filter-out SSS filter-color (translucency_color)
//        // 3. scale down by translucency_weight
//        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(SSSMaskTex0);
//        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(SSSMaskTex0).arg(translucencyTexture);
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(SSSMaskTex0).arg(translucency_weight);
//
//        ret_str += QString("scene.textures.%1.type = \"subtract\"\n").arg(SSSMaskTex1);
//        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(SSSMaskTex1).arg(OpacityTex);
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(SSSMaskTex1).arg(SSSMaskTex0);
//
//        OpacityTex = SSSMaskTex1;
//        opacity_exists = true;
//    }
//    //    if (!volume_exists) translucency_exists = false;
//
//
//
//        // Metallicity
//    QString metallicityTex = matLabel + "_metallicity";
//    if (metallic_weight > 0 && YaLuxGlobal.bDoMetallic)
//    {
//        //if (metallic_weight == 0) metallic_weight = 0.01;
//        float metallicity_scale = metallic_weight;
//        //if (glossy_layered_weight > 0) metallicity_scale *= glossy_layered_weight;
//
//        QString filterMetallicityTex = metallicityTex + "_raw_filter";
//        QString inverseFitlerMetallicityTex = metallicityTex + "_raw_filter_inverse";
//        if (metallic_mapfile != "")
//        {
//            ret_str += GenerateCoreTextureBlock1(metallicityTex, metallic_mapfile, metallicity_scale,
//                diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//                diffuse_gamma, diffuse_wrap, diffuse_channel);
//
//            // create metal-filter and inverse-metal-fitler
//            ret_str += GenerateCoreTextureBlock1(filterMetallicityTex, metallic_mapfile, 1.0,
//                diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//                diffuse_gamma, diffuse_wrap, diffuse_channel);
//            ret_str += QString("scene.textures.%1.type = \"subtract\"\n").arg(inverseFitlerMetallicityTex);
//            ret_str += QString("scene.textures.%1.texture1 = 1\n").arg(inverseFitlerMetallicityTex);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(inverseFitlerMetallicityTex).arg(filterMetallicityTex);
//        }
//        else
//        {
//            ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(metallicityTex);
//            ret_str += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(metallicityTex).arg(metallicity_scale);
//            ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(filterMetallicityTex);
//            ret_str += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(filterMetallicityTex).arg(metallic_weight);
//            ret_str += QString("scene.textures.%1.type = \"constfloat3\"\n").arg(inverseFitlerMetallicityTex);
//            ret_str += QString("scene.textures.%1.texture1 = %2 %2 %2\n").arg(inverseFitlerMetallicityTex).arg(1 - metallic_weight);
//        }
//
//        /////////////
//        // Mix into specular
//        ////////////////
//        QString specA_metallic_override = metallicityTex + "_override_spec_A";
//        QString specB_metallic_override = metallicityTex + "_override_spec_B";
//        QString specC_metallic_override = metallicityTex + "_override_spec_C";
//        QString specD_metallic_override = metallicityTex + "_override_spec_D";
//
//        // create filtered-metal specular grey: 0.5? 0.1?
//        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(specA_metallic_override);
//        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specA_metallic_override).arg("0.01 0.01 0.01");
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specA_metallic_override).arg(filterMetallicityTex);
//        // merge with metal-filtered, subtracted specular from above
//        //if (spec_weight > 0 || glossy_layered_weight > 0)
//        if (false)
//        {
//            // create black out or scale down metal-filtered specular
//            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(specB_metallic_override);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specB_metallic_override).arg(mainSpec);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specB_metallic_override).arg(inverseFitlerMetallicityTex);
//
//            // add A + B
//            ret_str += QString("scene.textures.%1.type = \"add\"\n").arg(specC_metallic_override);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specC_metallic_override).arg(specA_metallic_override);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specC_metallic_override).arg(specB_metallic_override);
//
//            // add metallized+colored specular
//            // 1. create metallic-filtered diffuse texture
//            // use the true scaled metallicity instead of the raw filtered metallicity
//            QString diffFilter = metallicityTex + "_diffuse_filtered";
//            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(diffFilter);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffFilter).arg(mainDiffTex);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffFilter).arg(metallicityTex);
//            // 2. add to subtracted specular
//            ret_str += QString("scene.textures.%1.type = \"add\"\n").arg(specD_metallic_override);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specD_metallic_override).arg(specC_metallic_override);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specD_metallic_override).arg(diffFilter);
//            // clamp
//            QString clamped_metallic_override = specD_metallic_override + "_clamped";
//            ret_str += QString("scene.textures.%1.type = \"clamp\"\n").arg(clamped_metallic_override);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(clamped_metallic_override).arg(specD_metallic_override);
//            ret_str += QString("scene.textures.%1.min = 0\n").arg(clamped_metallic_override);
//            ret_str += QString("scene.textures.%1.max = 1\n").arg(clamped_metallic_override);
//
//            mainSpec = clamped_metallic_override;
//        }
//        else
//        {
//            QString diffFilter = metallicityTex + "_diffuse_filtered";
//            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(diffFilter);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffFilter).arg(mainDiffTex);
//            ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffFilter).arg(metallicityTex);
//
//            //// add A + B
//            //ret_str += QString("scene.textures.%1.type = \"add\"\n").arg(specC_metallic_override);
//            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specC_metallic_override).arg(specA_metallic_override);
//            //ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(specC_metallic_override).arg(diffFilter);
//
//            //// clamp
//            //QString clamped_metallic_override = specC_metallic_override + "_clamped";
//            //ret_str += QString("scene.textures.%1.type = \"clamp\"\n").arg(clamped_metallic_override);
//            //ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(clamped_metallic_override).arg(specC_metallic_override);
//            //ret_str += QString("scene.textures.%1.min = 0\n").arg(clamped_metallic_override);
//            //ret_str += QString("scene.textures.%1.max = 1\n").arg(clamped_metallic_override);
//
//            mainSpec = diffFilter;
//        }
//
//        spec_exists = true;
//
//        /////////////
//        // Subtract from Diffuse
//        /////////////
//        QString diffuseA_metallic_override = metallicityTex + "_override_diff_A";
//        QString diffuseB_metallic_override = metallicityTex + "_override_diff_B"; // not needed?
//        QString inverseMetallicityTex = metallicityTex + "_inverse";
//
//        // create proper inverse of scaled metallicity
//        ret_str += QString("scene.textures.%1.type = \"subtract\"\n").arg(inverseMetallicityTex);
//        ret_str += QString("scene.textures.%1.texture1 = 1\n").arg(inverseMetallicityTex);
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(inverseMetallicityTex).arg(metallicityTex);
//
//        // black out or scale down metal-filtered diffuse
//        ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(diffuseA_metallic_override);
//        ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(diffuseA_metallic_override).arg(mainDiffTex);
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(diffuseA_metallic_override).arg(inverseMetallicityTex);
//
//        //// rename maindiff
//        mainDiffTex = diffuseA_metallic_override;
//
//    }
//
//
//
//    ///////////////////////////////////////////
//    //
//    // Material definition
//    //
//    ///////////////////////////////////////////
//
//    // Planning:
//    // 1. Metallic Layer
//    // 2. Specular Layer
//    // 3. Dual Specular Layer
//    // 4. Top Coat Layer? Component?
//    // 5. Translucency?? Volume?? Component
//    // 6. Glossy Component
//
////    if (!opacity_exists && refraction_weight == 0)
//    if (false)
//    {
//        // if type == metal, use glossy roughness
//        // else use glossiness
//        // if metallicity == 0, glossy2 only
//        // if metallicity == 1, metal2 only
//        // else glossy2 + metal2
//
//        // Step 1a. Make glossy2
//        // Step 1b. Make metal2
//        // Step 1c. Make Glossy component
//        // Step 1d. Make Translucency? Volume?
//        // Step 2. Mix glossy2 + metal2
//        // Step 3. Mix in Dual Spec
//        // Step 4. Mix in Top Coat?
//        // Step 5. Mix with opacity / null
//
//        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
//        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
//        if (spec_exists)
//            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
//        else
//            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(matLabel);
//        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
//        //        if (normal_mapfile != "") ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(matLabel).arg(matLabel + "_n");
//        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
//        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
//        //if (metallic_weight != 0 || metallic_mapfile != "") ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(metallic_weight);
//    }
//    else
//    {
//        QString nullmatLabel = matLabel + "_null";
//        ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(nullmatLabel);
//
//        QString glossy2Label = matLabel;
//
//        if (translucency_exists || refraction_weight > 0)
//        {
//            if (YaLuxGlobal.bDoDebugSSS && volume_exists)
//                ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(glossy2Label);
//            else if (refraction_weight >= 0.5)
//                ret_str += QString("scene.materials.%1.type = \"glass\"\n").arg(glossy2Label);
//            else if (translucency_exists && YaLuxGlobal.bDoTranslucency)
//                ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(glossy2Label);
//            else
//                ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(glossy2Label);
//            if (YaLuxGlobal.bDoSSSVolume && volume_exists && refraction_weight == 0)
//                ret_str += QString("scene.materials.%1.volume.interior = \"%2\"\n").arg(glossy2Label).arg(volumeLabel);
//            if (YaLuxGlobal.bDoTranslucency && translucency_exists)
//            {
//                ret_str += QString("scene.materials.%1.kt = \"%2\"\n").arg(glossy2Label).arg(translucencyTexture);
//            }
//        }
//        else
//        {
//            ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(glossy2Label);
//        }
//        ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(glossy2Label).arg(mainDiffTex);
//
//        if ((spec_exists || glossy_layered_weight > 0 || metallic_weight > 0) && YaLuxGlobal.bDoSpecular)
//        {
//            ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(glossy2Label).arg(mainSpec);
//            if (YaLuxGlobal.bDoTranslucency && translucency_exists)
//                ret_str += QString("scene.materials.%1.ks_bf = \"%2\"\n").arg(glossy2Label).arg(mainSpec);
//        }
//        else
//        {
//            ret_str += QString("scene.materials.%1.ks = 0 0 0\n").arg(glossy2Label);
//            if (YaLuxGlobal.bDoTranslucency && translucency_exists)
//                ret_str += QString("scene.materials.%1.ks_bf = 0 0 0\n").arg(glossy2Label);
//        }
//
//        if (bump_exists && YaLuxGlobal.bDoBumpMaps)
//        {
//            ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_b");
//            //ret_str += QString("scene.materials.%1.bumpsamplingdistance = \"%2\"\n").arg(glossy2Label).arg(1 / 1000000);
//        }
//        if (normal_mapfile != "" && YaLuxGlobal.bDoNormalMaps)
//            ret_str += QString("scene.materials.%1.normaltex = \"%2\"\n").arg(glossy2Label).arg(matLabel + "_n");
//
//
//        if (spec_weight > 0 && YaLuxGlobal.bDoSpecular)
//        {
//            // use dual lobe specular for front and backface
//            //QString spec1_string = QString("%1").arg(spec1_float);
//            //if (spec1_mapfile != "") spec1_string = QString("\"%1\"").arg(spec1_label);
//            QString spec1_string = QString("\"%1\"").arg(scaledSpec1Roughness); // spec2 is modified by spec_ratio
//            QString spec2_string = QString("\"%1\"").arg(scaledDualRoughness); // spec2 is modified by spec_ratio
//
//            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(spec1_string);
//            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(spec1_string);
//            if (YaLuxGlobal.bDoSpecular && translucency_exists)
//            {
//                ret_str += QString("scene.materials.%1.uroughness_bf = %2\n").arg(glossy2Label).arg(spec2_string);
//                ret_str += QString("scene.materials.%1.vroughness_bf = %2\n").arg(glossy2Label).arg(spec2_string);
//            }
//        }
//        else if (glossy_roughness_mapfile != "" && glossy_layered_weight > 0)
//        {
//            //QString glossyRoughnessTexLabel = glossy2Label + "_glossyRoughnessTex";
//            //ret_str += QString("scene.textures.%1.type = \"imagemap\"\n").arg(glossyRoughnessTexLabel);
//            //ret_str += QString("scene.textures.%1.file = \"%2\"\n").arg(glossyRoughnessTexLabel).arg(glossy_roughness_mapfile);
//            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(glossyRoughnessTexLabel);
//            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(glossyRoughnessTexLabel);
//        }
//        else
//        {
//            ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(glossy2Label).arg(uroughness);
//            ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(glossy2Label).arg(vroughness);
//        }
//
//        if (YaLuxGlobal.bDoDebugSSS && volume_exists)
//            ret_str += QString("scene.materials.%1.transparency = 1\n").arg(glossy2Label);
//        else if (opacity_exists)
//            ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(glossy2Label).arg(OpacityTex);
//        else if (opacity_value != 1)
//            ret_str += QString("scene.materials.%1.transparency = %2\n").arg(glossy2Label).arg(opacity_value);
//
//    }
//
//
//    return ret_str;
//}

//QString LuxCoreProcessOmUberSurfaceMaterial(DzMaterial* material, QString& mesg, QString matLabel)
//{
//    QString ret_str = "# (OmUberSurface) MATERIAL " + matLabel + "\n";
//
//    // diffuse image and color
//    float diffuse_vscale = -1;
//    float diffuse_uscale = 1;
//    float diffuse_gamma = 2.2;
//    float diffuse_voffset = 0; // vdelta
//    float diffuse_uoffset = 0; // udelta
//    QString diffuse_wrap = "repeat"; // repeat|black|clamp
//    QString diffuse_filtertype = "bilinear";
//    QString diffuse_channel = "";
//    QString diffuse_mapfile = ""; // Diffuse Color
//    QColor diffuse_value;
//    bool diffuse_exists = false;
//
//    // specular image and color
//    float spec_vscale = -1;
//    float spec_uscale = 1;
//    float spec_gamma = 2.2;
//    float spec_voffset = 0;
//    float spec_uoffset = 0;
//    QString spec_wrap = "repeat"; // repeat|black|clamp
//    QString spec_filtertype = "bilinear";
//    QString spec_channel = "";
//    QString spec_mapfile = ""; // Specular Color
//    QColor spec_value;
//    bool spec_exists = false;
//
//    // bump image and values
//    float bump_vscale = -1;
//    float bump_uscale = 1;
//    float bump_gamma = 1;
//    float bump_voffset = 0;
//    float bump_uoffset = 0;
//    QString bump_channel = "";
//    QString bump_wrap = "repeat";
//    QString bump_filtertype = "bilinear";
//    QString bump_mapfile = ""; // "Bump Strength"
//    float bump_value;
//    bool bump_exists = false;
//
//    // transmission map
//    QString opacity_mapfile = "";
//    float opacity_value = 1;
//    bool opacity_exists = false;
//
//    // material definition
//    float uroughness = 0.8;
//    float vroughness = 0.8;
//    float index_refraction = 0.0; // IOR
//
//
//    enum { glossy, matte, plastic, metal } material_type = glossy;
//
//    QString propertyLabel;
//    DzProperty* currentProperty;
//
//    // Matte vs Glossy
//    currentProperty = material->findProperty("Lighting Model");
//    if (currentProperty != NULL)
//    {
//        QString lighting_model = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
//        if (lighting_model.contains("glossy"))
//        {
//            material_type = glossy;
//        }
//        else if (lighting_model.contains("matte"))
//        {
//            material_type = matte;
//        }
//        else if (lighting_model.contains("plastic"))
//        {
//            material_type = plastic;
//        }
//        else if (lighting_model.contains("metal"))
//        {
//            material_type = metal;
//        }
//    }
//
//    currentProperty = material->findProperty("Diffuse Color");
//    if (currentProperty != NULL)
//    {
//        diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ((diffuse_value != 1) || (diffuse_mapfile != ""))
//            diffuse_exists = true;
//    }
//    currentProperty = material->findProperty("Horizontal Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
//        spec_uscale = diffuse_uscale;
//        bump_uscale = diffuse_uscale;
//    }
//    currentProperty = material->findProperty("Vertical Tiles");
//    if (currentProperty != NULL)
//    {
//        diffuse_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
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
//        if ((spec_value != 1) || (spec_mapfile != ""))
//            spec_exists = true;
//    }
//    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
//    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
//    currentProperty = material->findProperty("Bump Strength");
//    if (currentProperty != NULL)
//    {
//        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 100;
//        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if (bump_mapfile != "")
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
//        if ((opacity_value != 1) || (opacity_mapfile != ""))
//            opacity_exists = true;
//    }
//    currentProperty = material->findProperty("Glossiness");
//    if (currentProperty != NULL)
//    {
//        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
//        if (uroughness > 0.8) uroughness = 0.8;
//        vroughness = uroughness;
//
//        if (material_type == plastic)
//        {
//            uroughness += 0.5;
//            uroughness = (uroughness > 1.0) ? 1.0 : uroughness;
//            vroughness = uroughness;
//        }
//
//    }
//
//    // Diffuse Texture Block
//    if (diffuse_exists)
//        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
//            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
//            diffuse_uscale, diffuse_vscale, diffuse_uoffset, diffuse_voffset,
//            diffuse_gamma, diffuse_wrap, diffuse_channel);
//
//    // Specular Block
//    if (spec_exists)
//    {
//        // check specular strength!!!
//        QString realSpecularLabel = matLabel + "_s";
//        bool bDoMixtureTexture = false;
//        float spec_strength = 1.0;
//
//        // Always mix down by 25%
//        LuxGetFloatProperty(material, "Specular Strength", spec_strength, mesg);
//        realSpecularLabel = matLabel + "_s" + "_0";
//        spec_strength *= 0.25;
//
//        ret_str += GenerateCoreTextureBlock3(realSpecularLabel, spec_mapfile,
//            spec_value.redF(), spec_value.greenF(), spec_value.blueF(),
//            spec_uscale, spec_vscale, spec_uoffset, spec_voffset, spec_gamma,
//            spec_wrap, spec_channel);
//
//        ret_str += QString("scene.textures.%1.type = \"mix\"\n").arg(matLabel + "_s");
//        ret_str += QString("scene.textures.%1.texture1 = 0 0 0\n").arg(matLabel + "_s");
//        ret_str += QString("scene.textures.%1.texture2 = \"%2\"\n").arg(matLabel + "_s").arg(realSpecularLabel);
//        ret_str += QString("scene.textures.%1.amount = %2\n").arg(matLabel + "_s").arg(spec_strength);
//
//    }
//
//    // Bumpmap Block
//    if (bump_exists)
//        ret_str += GenerateCoreTextureBlock1_Grey(matLabel + "_b", bump_mapfile, bump_value,
//            bump_uscale, bump_vscale, bump_uoffset, bump_voffset, bump_gamma,
//            bump_wrap, bump_channel);
//
//
//    // Opacity Block
//    if (opacity_exists && opacity_mapfile != "")
//        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);
//
//    // Material definition
//    // decide what type of material...
//    if (!opacity_exists)
//    {
//        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
//        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
//        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
//
//        if (YaLuxGlobal.bDoBumpMaps)
//        {
//            if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
//            //ret_str += QString("scene.materials.%1.bumpsamplingdistance = \"%2\"\n").arg(matLabel).arg(1 / 1000000);
//        }
//
//        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
//        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
//        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(matLabel).arg(index_refraction);
//    }
//
//    if (opacity_exists)
//    {
//        // setup mix material
//        QString realmatLabel = matLabel + "_0";
//        QString nullmatLabel = matLabel + "_1";
//        ret_str += QString("scene.materials.%1.type = \"null\"\n").arg(nullmatLabel);
//
//        //ret_str += QString("scene.materials.%1.type = \"glossytranslucent\"\n").arg(realmatLabel);
//        ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(realmatLabel);
//
//        if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_d");
//        if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_s");
//
//        if (YaLuxGlobal.bDoBumpMaps)
//        {
//            if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(realmatLabel).arg(matLabel + "_b");
//            //ret_str += QString("scene.materials.%1.bumpsamplingdistance = \"%2\"\n").arg(realmatLabel).arg(1 / 1000000);
//        }
//
//        ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(realmatLabel).arg(uroughness);
//        ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(realmatLabel).arg(vroughness);
//        if (!spec_exists) ret_str += QString("scene.materials.%1.index = %2 %2 %2\n").arg(realmatLabel).arg(index_refraction);
//
//        ret_str += QString("scene.materials.%1.type = \"mix\"\n").arg(matLabel);
//        ret_str += QString("scene.materials.%1.material2 = \"%2\"\n").arg(matLabel).arg(realmatLabel);
//        ret_str += QString("scene.materials.%1.material1 = \"%2\"\n").arg(matLabel).arg(nullmatLabel);
//
//        if (opacity_mapfile != "")
//            ret_str += QString("scene.materials.%1.amount = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
//        else
//            ret_str += QString("scene.materials.%1.amount = %2\n").arg(matLabel).arg(opacity_value);
//
//    }
//
//
//    return ret_str;
//}

//QString LuxCoreProcessDazDefaultMaterial(DzMaterial* material, QString& mesg, QString matLabel)
//{
//    QString ret_str = "# MATERIAL " + matLabel + "\n";
//
//    // diffuse image and color
//    float material_vscale = -1;
//    float material_uscale = 1;
//    float diffuse_gamma = 2.2;
//    float material_voffset = 0; // vdelta
//    float material_uoffset = 0; // udelta
//    QString diffuse_wrap = "repeat"; // repeat|black|clamp
//    QString diffuse_filtertype = "bilinear";
//    QString diffuse_channel = "";
//    QString diffuse_mapfile = ""; // Diffuse Color
//    QColor diffuse_value = QColor(255, 255, 255);
//    bool diffuse_exists = false;
//
//    // specular image and color
//    float spec_gamma = 2.2;
//    QString spec_wrap = "repeat"; // repeat|black|clamp
//    QString spec_filtertype = "bilinear";
//    QString spec_channel = "";
//    QString spec_mapfile = ""; // Specular Color
//    QColor spec_value;
//    bool spec_exists = false;
//
//    // bump image and values
//    float bump_gamma = 1;
//    QString bump_channel = "";
//    QString bump_wrap = "repeat";
//    QString bump_filtertype = "bilinear";
//    QString bump_mapfile = ""; // "Bump Strength"
//    float bump_value;
//    bool bump_exists = false;
//
//    // transmission map
//    QString opacity_mapfile = "";
//    float opacity_value = 1;
//    bool opacity_exists = false;
//
//    // material definition
//    float uroughness = 0.8;
//    float vroughness = 0.8;
//    float index_refraction = 0.0; // IOR
//
//    QString propertyLabel;
//    DzProperty* currentProperty;
//
//    // material types
//    enum { glossy, matte, plastic, metal } material_type = glossy;
//    currentProperty = material->findProperty("Lighting Model");
//    if (currentProperty != NULL)
//    {
//        QString lighting_model = ((DzEnumProperty*)currentProperty)->getStringValue().toLower();
//        if (lighting_model.contains("glossy"))
//        {
//            material_type = glossy;
//        }
//        else if (lighting_model.contains("matte"))
//        {
//            material_type = matte;
//        }
//        else if (lighting_model.contains("plastic"))
//        {
//            material_type = plastic;
//        }
//        else if (lighting_model.contains("metal"))
//        {
//            material_type = metal;
//        }
//    }
//
//    currentProperty = material->findProperty("Diffuse Color");
//    if (currentProperty != NULL)
//    {
//        // Sanity Check (FIX for Granite Iray Shader)
//        if (currentProperty->inherits("DzColorProperty"))
//            diffuse_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        if (currentProperty->inherits("DzNumericProperty"))
//            diffuse_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        else if (currentProperty->inherits("DzImageProperty"))
//            diffuse_mapfile = ((DzImageProperty*)currentProperty)->getValue()->getTempFilename();
//        if ((diffuse_value != QColor(255, 255, 255)) || (diffuse_mapfile != ""))
//            diffuse_exists = true;
//    }
//    currentProperty = material->findProperty("Horizontal Tiles");
//    if (currentProperty != NULL)
//    {
//        material_uscale = 1 / ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Vertical Tiles");
//    if (currentProperty != NULL)
//    {
//        material_vscale = -1 / ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Horizontal Offset");
//    if (currentProperty != NULL)
//    {
//        material_uoffset = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Vertical Offset");
//    if (currentProperty != NULL)
//    {
//        material_voffset = ((DzFloatProperty*)currentProperty)->getValue();
//    }
//    currentProperty = material->findProperty("Specular Color");
//    if (currentProperty != NULL)
//    {
//        spec_value = ((DzColorProperty*)currentProperty)->getColorValue();
//        spec_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if ((spec_value != 1) || (spec_mapfile != ""))
//            spec_exists = true;
//    }
//    // Note that the Luxrender units for specifying bump are 1 = one meter.  So we must scale
//    //   down the value read from Daz. ie, 100% Daz strength ~ 0.01 Luxrender units
//    currentProperty = material->findProperty("Bump Strength");
//    if (currentProperty != NULL)
//    {
//        bump_value = ((DzFloatProperty*)currentProperty)->getValue() / 100;
//        bump_mapfile = propertyNumericImagetoString((DzNumericProperty*)currentProperty);
//        if (bump_mapfile != "")
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
//        if ((opacity_value != 1) || (opacity_mapfile != ""))
//            opacity_exists = true;
//    }
//    currentProperty = material->findProperty("Glossiness");
//    if (currentProperty != NULL)
//    {
//        uroughness = 1 - ((DzFloatProperty*)currentProperty)->getValue();
//        if (uroughness > 0.8) uroughness = 0.8;
//        vroughness = uroughness;
//
//        if (material_type == plastic)
//        {
//            uroughness += 0.5;
//            uroughness = (uroughness > 1.0) ? 1.0 : uroughness;
//            vroughness = uroughness;
//        }
//
//    }
//
//    // Opacity Block
//    if (opacity_exists && opacity_mapfile != "")
//        ret_str += GenerateCoreTextureBlock1(matLabel + "_o", opacity_mapfile, opacity_value);
//
//    // Diffuse Texture Block
//    if (diffuse_exists)
//        ret_str += GenerateCoreTextureBlock3(matLabel + "_d", diffuse_mapfile,
//            diffuse_value.redF(), diffuse_value.greenF(), diffuse_value.blueF(),
//            material_uscale, material_vscale, material_uoffset, material_voffset,
//            diffuse_gamma, diffuse_wrap, diffuse_channel);
//
//    // Specular Block
//    if (spec_exists)
//    {
//        // check specular strength!!!
//        QString realSpecularLabel = matLabel + "_s";
//        bool bDoMixtureTexture = false;
//        float spec_strength = 1.0;
//        if (LuxGetFloatProperty(material, "Specular Strength", spec_strength, mesg) && spec_strength < 1.0)
//        {
//            bDoMixtureTexture = true;
//            realSpecularLabel = matLabel + "_s" + "_0";
//        }
//        ret_str += GenerateCoreTextureBlock3(realSpecularLabel, spec_mapfile,
//            spec_value.redF(), spec_value.greenF(), spec_value.blueF(),
//            material_uscale, material_vscale, material_uoffset, material_voffset, spec_gamma,
//            spec_wrap, spec_channel);
//        if (bDoMixtureTexture)
//        {
//            QString specularScaleLabel = matLabel + "_s";
//            ret_str += QString("scene.textures.%1.type = \"scale\"\n").arg(specularScaleLabel);
//            ret_str += QString("scene.textures.%1.texture1 = \"%2\"\n").arg(specularScaleLabel).arg(realSpecularLabel);
//            ret_str += QString("scene.textures.%1.texture2 = %2\n").arg(specularScaleLabel).arg(spec_strength);
//        }
//    }
//
//    // Bumpmap Block
//    if (bump_exists)
//        ret_str += GenerateCoreTextureBlock1_Grey(matLabel + "_b", bump_mapfile, bump_value,
//            material_uscale, material_vscale, material_uoffset, material_voffset, bump_gamma,
//            bump_wrap, bump_channel);
//
//
//    // Material definition
//    ret_str += QString("scene.materials.%1.type = \"glossy2\"\n").arg(matLabel);
//    if (diffuse_exists) ret_str += QString("scene.materials.%1.kd = \"%2\"\n").arg(matLabel).arg(matLabel + "_d");
//    if (spec_exists) ret_str += QString("scene.materials.%1.ks = \"%2\"\n").arg(matLabel).arg(matLabel + "_s");
//
//    if (YaLuxGlobal.bDoBumpMaps)
//    {
//        if (bump_exists) ret_str += QString("scene.materials.%1.bumptex = \"%2\"\n").arg(matLabel).arg(matLabel + "_b");
//    }
//
//    ret_str += QString("scene.materials.%1.uroughness = %2\n").arg(matLabel).arg(uroughness);
//    ret_str += QString("scene.materials.%1.vroughness = %2\n").arg(matLabel).arg(vroughness);
//
//    if (opacity_exists)
//    {
//        if (opacity_mapfile != "")
//            ret_str += QString("scene.materials.%1.transparency = \"%2\"\n").arg(matLabel).arg(matLabel + "_o");
//        else
//            ret_str += QString("scene.materials.%1.transparency = %2\n").arg(matLabel).arg(opacity_value);
//    }
//
//    return ret_str;
//}

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

        // process related vertex group for this material
        QString objMatName = QString("%1_%2").arg(nodeLabel).arg(matLabel);

        QString plyFileName;
        if (geo->inherits("DzFacetMesh"))
        {
            DazToPLY dzPLYexport((DzFacetMesh*)geo, objMatName, material);
            plyFileName = dzPLYexport.LuxMakeBinPLY();
        }
        if (plyFileName == "") continue;

        matLabel = SanitizeCoreLabel(material->getLabel());
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
            attributeblock += IrayUberToLuxCoreMaterial(material, nodeLabel + matLabel).toString();
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
        lightIntensity = dynamic_cast<DzDistantLight*>(currentLight)->getIntensity() * scale;
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

        if (currentObject == NULL || currentNode->isHidden() || !currentNode->isVisible())
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
        if (currentNode->inherits("DzFigure"))
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
//            mesg += "native.threads.count = 4\n";
            mesg += "path.hybridbackforward.enable = 1\n";
            break;
        case 1: // Hybrid (Native CPU + OpenCL GPU)
//            return false;
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 0\n";
            mesg += "opencl.gpu.use = 1\n";
//            mesg += "native.threads.count = 0\n";
            mesg += "opencl.native.threads.count = 2\n";
            mesg += "path.hybridbackforward.enable = 1\n";
            mesg += "path.hybridbackforward.partition = 0.0\n";
            break;
        case 2: // OpenCL GPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 0\n";
            mesg += "opencl.gpu.use = 1\n";
//            mesg += "native.threads.count = 0\n";
            mesg += "opencl.native.threads.count = 0\n";
            break;
        case 3: // OpenCL CPU only
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 0\n";
            mesg += "opencl.gpu.use = 0\n";
//            mesg += "native.threads.count = 0\n";
            mesg += "opencl.native.threads.count = 0\n";
            break;
        case 4: // OpenCL GPU + OpenCL CPU
            mesg += "\"PATHOCL\"\n";
            mesg += "opencl.cpu.use = 1\n";
            mesg += "opencl.cpu.workgroup.size = 0\n";
            mesg += "opencl.gpu.use = 1\n";
//            mesg += "native.threads.count = 0\n";
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
    mesg += "periodicsave.film.outputs.period = 10\n";
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
