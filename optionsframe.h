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

    QStringList debugLevelList = QStringList() << "0: Errors Only" << "1: Errors and Info" << "2: Errors, Info, Debug" << "3: Verbose Debugging Data";
    QStringList toneMapMethodList = QStringList() << "linear" << "autolinear" << "maxwhite" << "contrast" << "reinhard";
    QStringList renderModeList = QStringList() << "Software" << "Hybrid" << "(PATH)OpenCL GPU only" << "(PATH)OpenCL CPU only" << "(PATH)OpenCL GPU+CPU" << "Custom Render String...";
    QStringList specularModeList = QStringList() << "90% Diffuse + 10% Specular" << "Specular * Glossiness" << "(75% Diffuse + 25% Specular) * Glossiness" << "10% Specular" << "Full Specular (may wash-out Diffuse color)" << "Specular Off";

    // property-widget list
    DzFileProperty      *execPath;
    DzStringProperty    *argumentList;
    DzBoolProperty      *showLuxWindow;
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
    DzStringProperty    *renderServerList;
    DzEnumProperty      *specularMode;
    DzEnumProperty      *renderMode;
    DzStringProperty    *customRenderString;

};


#endif // __yaluxplug__optionsframe__
