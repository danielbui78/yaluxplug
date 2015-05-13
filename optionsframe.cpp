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
    saveAlphaChannel = new DzBoolProperty("yalux_save_alpha", true, false, false);
    saveAlphaChannel->setLabel("Save alpha channel in PNG");
    listView->addProperty(saveAlphaChannel);

    // Halt Conditions to stop rendering
    haltTime = new DzIntProperty("yalux_halt_time", false, false, 300);
    haltTime->setLabel("[halttime] Maximum time to render a frame (seconds)");
    haltSPP = new DzIntProperty("yalux_halt_spp", false, false, 0);
    haltSPP->setLabel("[haltspp] Target samples per pixel");
    haltThreshold = new DzFloatProperty("yalux_halt_threshold", false, false, 0.0);
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
    tonemapFstop = new DzFloatProperty("yalux_tonemap_fstop", false, false, 5);
    tonemapFstop->setLabel("Linear Tonemapper F-stop");
    tonemapExposureTime = new DzFloatProperty("yalux_tonemap_exposure", false, false, 0.5);
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

    loadSettings();

    return;
}

///////////
// Load Settings
///////////
void	YaLuxOptionsFrame::loadSettings()
{
    execPath->setValue( settings->getStringValue("LuxrenderExecPath") );
    argumentList->setValue( settings->getStringValue("CmdLineArgs") );

}

///////////
// Save Settings
///////////
void	YaLuxOptionsFrame::saveSettings()
{
    settings->setStringValue("LuxrenderExecPath", execPath->getValue());
    settings->setStringValue("CmdLineArgs", argumentList->getValue());

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
