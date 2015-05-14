//
//  optionsframe.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#include <QtGUI/QTouchEvent>
#include <QtGUI/QLayout>

#include "dzbasicpropertywgt.h"
#include "dzfilepropertywgt.h"
#include "dzproperty.h"
#include "dzfileproperty.h"
#include "dzboolproperty.h"
#include "dzintproperty.h"
#include "dzfloatproperty.h"
#include "dzenumproperty.h"
#include "dzstringproperty.h"

#include "dzstyledfilepropertywgt.h"
#include "dzsidenavpropertylistview.h"
#include "dzfilternavigationbar.h"

#include "dzfileio.h"
#include "dzappsettings.h"

#include "optionsframe.h"
#include "plugin.h"

#if defined( Q_OS_WIN )
const char *ExecExt = "exe";
#elif defined( Q_WS_MAC )
const char *ExecExt = "app";
#endif


void YaLuxOptionsFrame::execPathChanged()
{
    QString path;
    path = execPath->getValue();
    if ( DzFileIO::getFileExtension(path).contains(ExecExt) )
    {
        path = DzFileIO::getFilePath(path);
        execPath->setValue(path);
    }
}

YaLuxOptionsFrame::YaLuxOptionsFrame() : DzOptionsFrame("yaluxplug Options Frame", 0, "yaluxplug Options Frame")
{
    int i;

    settings = new DzAppSettings("yaluxplug");

    QVBoxLayout *layout1 = new QVBoxLayout(this);

    filterBar = new DzFilterNavigationBar(this);
    filterBar->setAutoHidePageNavigation(true);
    filterBar->setPageSizeVisible(false);
    filterBar->setPageViewVisible(false);
    filterBar->setPageNavigationVisible(false);
    filterBar->setPageLabelVisible(false);

    listView = new DzSideNavPropertyListView(this);
    connect(filterBar, SIGNAL(filterChanged(const QString &)),
            listView, SLOT(setFilterString(const QString &)) );

    layout1->addWidget(filterBar);
    layout1->addWidget(listView);

  //  QVBoxLayout *layout2 = new QVBoxLayout(listView);

    // Luxrender executable path
    execPath = new DzFileProperty("yalux_exec_path", false);
    execPath->setLabel("Folder to Luxrender Application");
    execPath->setType(DzFileProperty::FileType::FileOpen);
    execPath->setFilter( QString("FilterName (*.%1)").arg(ExecExt));
    connect(execPath, SIGNAL(currentValueChanged()),
            this, SLOT(execPathChanged()) );
    listView->addProperty(execPath);

    // Luxrender executable arguments
    argumentList = new DzStringProperty("yalux_exec_arguments", false);
    argumentList->setLabel("Executable Arguments");
    listView->addProperty(argumentList);

    // Show Luxrender window
    showLuxWindow = new DzBoolProperty("yalux_showwindow", true, false, false);
    showLuxWindow->setLabel("Show Luxrender window");
    listView->addProperty(showLuxWindow);

    // Save-related settings
    // Save Alpha Channel
    saveAlphaChannel = new DzBoolProperty("yalux_save_alpha", true, false, true);
    saveAlphaChannel->setLabel("Save alpha channel in PNG");
    listView->addProperty(saveAlphaChannel);

    // Halt Conditions to stop rendering
    haltTime = new DzIntProperty("yalux_halt_time", false, false, 300);
    haltTime->setLabel("[halttime] Maximum time to render a frame (seconds)");
    haltSPP = new DzIntProperty("yalux_halt_spp", false, false, 0);
    haltSPP->setLabel("[haltspp] Target samples per pixel");
    haltThreshold = new DzFloatProperty("yalux_halt_threshold", false, false, 1.0);
    haltThreshold->setLabel("Target convergence threshold (1-haltthrehold)");
    listView->addProperty(haltTime);
    listView->addProperty(haltSPP);
    listView->addProperty(haltThreshold);

    debugLevel = new DzEnumProperty("yalux_debug_level", true, false);
    debugLevel->setLabel("Debug Output");
    for (i=0; i<debugLevelList.count(); i++) {
        debugLevel->addItem(debugLevelList[i]);
    }
    listView->addProperty(debugLevel);

    // camera/film lighting correction
    toneMapMethod = new DzEnumProperty("yalux_tonemap_method", false, false);
    toneMapMethod->setLabel("Tonemapping Method");
    for (i=0; i<toneMapMethodList.count(); i++) {
        toneMapMethod->addItem(toneMapMethodList[i]);
    }
    tonemapGamma = new DzFloatProperty("yalux_tonemap_gamma", false, false, 2.2);
    tonemapGamma->setLabel("Linear Tonemapper Gamma");
    tonemapFstop = new DzFloatProperty("yalux_tonemap_fstop", false, false, 2.8);
    tonemapFstop->setLabel("Linear Tonemapper F-stop");
    tonemapExposureTime = new DzFloatProperty("yalux_tonemap_exposure", false, false, 3);
    tonemapExposureTime->setLabel("Linear Tonemapper Exposure time (seconds)");
    tonemapISO = new DzIntProperty("yalux_tonemap_iso", false, false, 400);
    tonemapISO->setLabel("Linear Tonemapper ISO");
    listView->addProperty(toneMapMethod);
    listView->addProperty(tonemapGamma);
    listView->addProperty(tonemapFstop);
    listView->addProperty(tonemapExposureTime);
    listView->addProperty(tonemapISO);

    // render server list
    renderServerList = new DzStringProperty("yalux_network_serverlist", false);
    renderServerList->setLabel("Network Render Servers (IP/hostnames)");
    listView->addProperty(renderServerList);

//    DzStyledFilePropertyWgt *widget = new DzStyledFilePropertyWgt(listView, "test string");
//    widget->addProperty(fileProperty);
//    layout2->addWidget(widget);
//    layout2->setAlignment(widget, Qt::AlignTop);

    this->setLayout(layout1);

    createDefaultSettings();
    loadSettings();

    return;
}

void YaLuxOptionsFrame::createDefaultSettings()
{
    showLuxWindow->setDefaultBoolValue(false);
    saveAlphaChannel->setDefaultBoolValue(true);
    haltTime->setDefaultValue(300);
    haltSPP->setDefaultValue(0);
    haltThreshold->setDefaultValue(1.0);
    debugLevel->setDefaultValue(0);
    tonemapGamma->setDefaultValue(2.2);
    tonemapFstop->setDefaultValue(2.8);
    tonemapISO->setDefaultValue(400);
    toneMapMethod->setDefaultValue(0);
}

///////////
// Load Settings
///////////
void	YaLuxOptionsFrame::loadSettings()
{
    if ( settings->getBoolValue("yalux_savedsettings_exist") )
    {
        execPath->setValue( settings->getStringValue( execPath->getName()) );
        argumentList->setValue( settings->getStringValue( argumentList->getName()) );
        showLuxWindow->setBoolValue( settings->getBoolValue( showLuxWindow->getName()) );
        saveAlphaChannel->setBoolValue( settings->getBoolValue( saveAlphaChannel->getName()) );
        haltTime->setValue( settings->getIntValue( haltTime->getName()) );
        haltSPP->setValue( settings->getIntValue( haltSPP->getName()) );
        haltThreshold->setValue( settings->getFloatValue( haltThreshold->getName()) );
        debugLevel->setValue( settings->getIntValue( debugLevel->getName()) );
        tonemapGamma->setValue( settings->getFloatValue( tonemapGamma->getName()) );
        tonemapFstop->setValue( settings->getFloatValue( tonemapFstop->getName()) );
        tonemapExposureTime->setValue( settings->getFloatValue( tonemapExposureTime->getName()) );
        tonemapISO->setValue( settings->getIntValue( tonemapISO->getName()) );
        toneMapMethod->setValue( settings->getIntValue( toneMapMethod->getName()) );
        renderServerList->setValue( settings->getStringValue( renderServerList->getName()) );
    }
}

///////////
// Save Settings
///////////
void	YaLuxOptionsFrame::saveSettings()
{
    settings->setStringValue(execPath->getName(), execPath->getValue());
    settings->setStringValue(argumentList->getName(), argumentList->getValue());
    settings->setIntValue(showLuxWindow->getName(), showLuxWindow->getBoolValue());
    settings->setBoolValue(saveAlphaChannel->getName(), saveAlphaChannel->getBoolValue());
    settings->setIntValue(haltTime->getName(), haltTime->getValue());
    settings->setIntValue(haltSPP->getName(), haltSPP->getValue());
    settings->setFloatValue(haltThreshold->getName(), haltThreshold->getValue());
    settings->setIntValue(debugLevel->getName(), debugLevel->getValue());
    settings->setFloatValue(tonemapGamma->getName(), tonemapGamma->getValue());
    settings->setFloatValue(tonemapFstop->getName(), tonemapFstop->getValue());
    settings->setFloatValue(tonemapExposureTime->getName(), tonemapExposureTime->getValue());
    settings->setIntValue(tonemapISO->getName(), tonemapISO->getValue());
    settings->setIntValue(toneMapMethod->getName(), toneMapMethod->getValue());
    settings->setStringValue(renderServerList->getName(), renderServerList->getValue());

    settings->setBoolValue("yalux_savedsettings_exist", true);

}

void	YaLuxOptionsFrame::applyChanges()
{
    // DEBUG
    dzApp->log("yaluxplug: Render Options Panel: applying Changes.");
    // DEBUG FIX: THIS IS ONLY FOR MAC platform
#if defined( Q_OS_WIN )
    YaLuxGlobal.LuxExecPath = execPath->getValue() + "/luxconsole.exe";
#elif defined( Q_WS_MAC )
    YaLuxGlobal.LuxExecPath = execPath->getValue() + "/Luxrender.app/Contents/MacOS/luxconsole";
#endif
    YaLuxGlobal.CmdLineArgs = argumentList->getValue();

    YaLuxGlobal.bShowLuxRenderWindow = showLuxWindow->getBoolValue();
    YaLuxGlobal.bSaveAlphaChannel = saveAlphaChannel->getBoolValue();
    YaLuxGlobal.haltAtTime = haltTime->getValue();
    YaLuxGlobal.haltAtSamplesPerPixel = haltSPP->getValue();
    YaLuxGlobal.haltAtThreshold = haltThreshold->getValue();
    YaLuxGlobal.debugLevel = debugLevel->getValue();
    YaLuxGlobal.tonemapGamma = tonemapGamma->getValue();
    YaLuxGlobal.tonemapFstop = tonemapFstop->getValue();
    YaLuxGlobal.tonemapExposureTime = tonemapExposureTime->getValue();
    YaLuxGlobal.tonemapISO = tonemapISO->getValue();
    YaLuxGlobal.LuxToneMapper = toneMapMethod->getStringValue();
    YaLuxGlobal.slaveNodeList = renderServerList->getValue().split(",");

    // DEBUG - find a place for this
    saveSettings();
};

void	YaLuxOptionsFrame::resetOptions()
{
    // DEBUG
    dzApp->log("yaluxplug: Render Options Panel: resetting options.");
};

bool    YaLuxOptionsFrame::applyValid() const
{
    return true; 
}

void	YaLuxOptionsFrame::restoreOptions( DzRenderOptions *options )
{

};
