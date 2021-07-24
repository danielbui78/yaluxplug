#pragma once
#include "dzmaterial.h"
#include <qcolor.h>

class DzMaterialToLuxCoreMaterial
{
public:
	class TextureBlock
	{
	public:
		QString name = "";
		QString data= "";

	};

	class MaterialBlock
	{
	public:
		QString name = "";
		QString data = "";
	};

	DzMaterialToLuxCoreMaterial(DzMaterial* m, QString luxMatName)
	{
		m_Material = m;
		m_LuxMaterialName = luxMatName;
	};

	virtual QString toString() { return QString(); };

	bool hasDiffuseMap() const { return (m_DiffuseMap != ""); }
	bool hasDiffuseColor() const { return m_DiffuseExists; }
	bool hasSpecularMap() const { return (m_SpecularMap != ""); }
	bool hasSpecularColor() const { return m_SpecularExists; }
	bool hasBumpMap() const { return (m_BumpMap != ""); }
	bool hasRoughness() const { return m_RoughnessExists; }
	bool hasOpacityMap() const { return (m_OpacityMap != ""); }
	bool hasOpacityValue() const { return m_OpacityExists; }

	friend bool operator==(const DzMaterialToLuxCoreMaterial&a, const DzMaterialToLuxCoreMaterial&b);

protected:
	DzMaterial *m_Material;
	QString		m_LuxMaterialName;

	MaterialBlock m_PrimaryMaterialBlock;

	TextureBlock m_DiffuseTex;
	TextureBlock m_SpecularTex;
	TextureBlock m_BumpTex;
	TextureBlock m_NormalTex;
	TextureBlock m_OpacityTex;

	bool m_DiffuseExists = false;
	bool m_SpecularExists = false;
	bool m_BumpExists = false;
	bool m_NormalExists = false;
	bool m_RoughnessExists = false;
	bool m_OpacityExists = false;

	QString		m_DiffuseMap = "";
	QColor		m_DiffuseColor = QColor(255,255,255);
	QString		m_SpecularMap = "";
	QColor		m_SpecularColor = QColor(0,0,0);
	QString		m_BumpMap = "";
	QString		m_NormalMap = "";
	double		m_BumpStrength = 0;
	double		m_Roughness = 0.5;
	QString		m_OpacityMap = "";
	double		m_OpacityValue = 1.0;

};

