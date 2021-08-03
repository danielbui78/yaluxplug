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

    double m_MetallicWeight = 0;
    double m_SpecularWeight = 0;
    double m_RefractionWeight = 0;
    double m_TranslucencyWeight = 0;
    double m_GlossyLayeredWeight = 0;

    QString m_SpecRoughness_1 = "";
    QString m_SpecRoughness_2 = "";
    QString m_MetallicMap = "";
//    QString m_SpecularWeightMap = ""; // use specularmap instead
    double m_RefractionIndex = 1.0;

    QColor m_TranslucencyColor = QColor(255,255,255);
    QString m_TranslucencyMap = "";
    bool m_TranslucencyExists = false;
    QColor m_SSS_tint = QColor(255,255,255);

    bool m_VolumeExists = false;
    double m_TransmissionDistance = 1;
    QColor m_TransmissionColor = QColor(0,0,0);
    double m_ScatteringDistance = 1;
    QColor m_ScatteringColor = QColor(0,0,0);
    double m_ScatteringDirection = 0;

    double m_NormalStrength = 0;
    QString m_NormalMap = "";

    double m_GlossyRoughness = 0.5;
    QString m_GlossyRoughnessMap = "";
    double m_GlossyReflectivity = 0.5;
    QColor m_GlossyColor = QColor(0,0,0);
    QString m_GlossyMap = "";

    TextureBlock m_GlossyTex;
    TextureBlock m_GlossyRoughnessTex;
    TextureBlock m_NormalTex;
    TextureBlock m_TranslucencyTex;
    TextureBlock m_MetallicTex;

    TextureBlock m_TranslucencyTex_MASK;

    QString m_VolumeName = "";
};

