//
//  renderoptions.h
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#ifndef __yaluxplug__optionsframe__
#define __yaluxplug__optionsframe__

#include <dzoptionsframe.h>

class DzFilterNavigationBar;
class DzSideNavPropertyListView;
class DzFileProperty;
class DzStringProperty;
class DzAppSettings;
class DzBoolProperty;
class DzIntProperty;
class DzFloatProperty;
class DzEnumProperty;

class YaLuxOptionsFrame : public DzOptionsFrame {
    Q_OBJECT
public:
//    YaLuxOptions(const QString &label, QWidget *parent=0, const QString &name=QString::null);
    YaLuxOptionsFrame();
    ~YaLuxOptionsFrame();

public slots:
    virtual void	applyChanges();
    virtual void	resetOptions();
    virtual bool	applyValid() const;
    virtual void	restoreOptions( DzRenderOptions *options );

signals:
//    void	optionsChanged();

public:
    DzAppSettings *settings;

public slots:
    void execPathChanged();

private:
    void createDefaultSettings();
    void loadSettings();
    void saveSettings();

private:
    DzFilterNavigationBar *filterBar;
    DzSideNavPropertyListView *listView;

    QStringList maxTextureSizeList = QStringList() << "256" << "512" << "1024" << "1536" << "2048" << "2560" << "3072" << "3584" << "4096" << "none";
    QStringList debugLevelList = QStringList() << "Level 0: Errors Only" << "Level 1: Errors and Info" << "Level 2: Errors, Info, Debug" << "Level 3: Verbose Debugging Data";
    QStringList toneMapMethodList = QStringList() << "linear" << "autolinear" << "maxwhite" << "contrast" << "reinhard";
    QStringList renderModeList = QStringList() << "Software" << "Hybrid" << "(PATH)OpenCL GPU only" << "(PATH)OpenCL CPU only" << "(PATH)OpenCL GPU+CPU" << "Custom Render String...";
    QStringList specularModeList = QStringList() << "90% Diffuse + 10% Specular" << "Specular * Glossiness" << "(75% Diffuse + 25% Specular) * Glossiness" << "10% Specular" << "Full Specular (may wash-out Diffuse color)" << "Specular Off";

    // property-widget list
    DzFileProperty      *execPath;
    DzStringProperty    *argumentList;
    DzBoolProperty      *showLuxWindow;
    DzEnumProperty       *maxTextureSize;
    DzBoolProperty      *saveAlphaChannel;
    DzIntProperty       *haltTime;
    DzIntProperty       *haltSPP;
    DzFloatProperty     *haltThreshold;
    DzEnumProperty      *debugLevel;
    DzFloatProperty     *tonemapGamma;
    DzFloatProperty     *tonemapFstop;
    DzFloatProperty     *tonemapExposureTime;
    DzIntProperty       *tonemapISO;
    DzEnumProperty      *toneMapMethod;
    DzBoolProperty      *networkRenderOn;
    DzStringProperty    *renderServerList;
    DzEnumProperty      *specularMode;
    DzEnumProperty      *renderMode;
    DzStringProperty    *customRenderString;

    DzBoolProperty      *addTonemapperAndEnvironment;
    DzBoolProperty      *preferNormal;
    DzBoolProperty      *overrideTransmissionColor;
    DzBoolProperty      *darkenDiffuseTexture;
    DzBoolProperty      *saturateDiffuseTexture;

    DzBoolProperty      *doBumpMaps;
    DzBoolProperty      *doNormalMaps;
    DzBoolProperty      *doNormalAsBump;
    DzBoolProperty      *doMetallic;
    DzBoolProperty      *doSpecular;
    DzBoolProperty      *doRoughnessMaps;
    DzBoolProperty      *doTranslucency;
    DzBoolProperty      *doSSS_Volume;
    DzBoolProperty      *doSSS_Absorption;
    DzBoolProperty      *doSSS_Scattering;
    DzBoolProperty      *doDebugSSS;

};


#endif // __yaluxplug__optionsframe__
