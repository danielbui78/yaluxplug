#pragma once
#include "DzMaterialToLuxCoreMaterial.h"

class DazDefaultToLuxCoreMaterial : public DzMaterialToLuxCoreMaterial
{
public:
	DazDefaultToLuxCoreMaterial(DzMaterial* m, QString luxMatName);
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

};

