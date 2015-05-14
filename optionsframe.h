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

    QStringList debugLevelList = QStringList() << "Errors Only" << "Errors and Info" << "Errors, Info, Debug";
    QStringList toneMapMethodList = QStringList() << "linear" << "autolinear" << "maxwhite" << "contrast" << "reinhard";

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

};


#endif // __yaluxplug__optionsframe__
