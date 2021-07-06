#pragma once
#include "DzMaterialToLuxCoreMaterial.h"

class IrayUberToLuxCoreMaterial : public DzMaterialToLuxCoreMaterial
{
public:
	IrayUberToLuxCoreMaterial(DzMaterial* m, QString luxMatName);
	QString toString();

protected:
    bool ImportValues();
    bool CreateTextures();
    bool CreateMaterials();

    double m_vscale = -1;
    double m_uscale = 1;
    double m_voffset = 0; // vdelta
    double m_uoffset = 0; // udelta
    double m_DiffuseGamma = 2.2;

    double m_MetallicWeight;
    double m_SpecularWeight;
    double m_RefractionWeight;
    double m_TranslucencyWeight;
    double m_GlossyLayeredWeight;

    QString m_SpecRoughness_1;
    QString m_SpecRoughness_2;
    QString m_MetallicMap = "";
//    QString m_SpecularWeightMap = ""; // use specularmap instead
    double m_RefractionIndex;

    QColor m_TranslucencyColor;
    QString m_TranslucencyMap = "";
    bool m_TranslucencyExists = false;
    QColor m_SSS_tint;

    bool m_VolumeExists = false;
    double m_TransmissionDistance;
    QColor m_TransmissionColor;
    double m_ScatteringDistance;
    QColor m_ScatteringColor;
    double m_ScatteringDirection;

    double m_NormalStrength;
    QString m_NormalMap = "";

    double m_GlossyRoughness;
    QString m_GlossyRoughnessMap = "";
    double m_GlossyReflectivity;
    QColor m_GlossyColor;
    QString m_GlossyMap = "";

    TextureBlock m_GlossyTex;
    TextureBlock m_GlossyRoughnessTex;
    TextureBlock m_NormalTex;
    TextureBlock m_TranslucencyTex;
    TextureBlock m_MetallicTex;

    QString m_VolumeName;
};

