//
//  optionsframe.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#include <QtGUI/QTouchEvent>
#include <QtGUI/QLayout>

#include <qfile.h>
#include <qmessagebox.h>

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

YaLuxOptionsFrame::~YaLuxOptionsFrame() 
{
    saveSettings();
    YaLuxGlobal.optFrame = NULL;

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
    argumentList->setLabel("Luxrender Commandline Arguments");
    listView->addProperty(argumentList);

    // Render Mode
    renderMode = new DzEnumProperty("yalux_render_mode", false, false);
    renderMode->setLabel("Render Engine");
    for (i=0; i<renderModeList.count(); i++) {
        renderMode->addItem(renderModeList[i]);
    }
    listView->addProperty(renderMode);

    // Custom Render String
    customRenderString = new DzStringProperty("yalux_render_custom", false);
    customRenderString->setLabel("Custom Render String (use Custom Render Engine)");
    listView->addProperty(customRenderString);

    // Max Texture Size
    maxTextureSize = new DzEnumProperty("yalux_max_texturesize", false, false);
    maxTextureSize->setLabel("Maximum Texture Size");
    for (i=0; i<maxTextureSizeList.count(); i++) {
        maxTextureSize->addItem(maxTextureSizeList[i]);
    }
    listView->addProperty(maxTextureSize);

    // Debug output level
    debugLevel = new DzEnumProperty("yalux_debug_level", true, false);
    debugLevel->setLabel("Debug Output");
    for (i=0; i<debugLevelList.count(); i++) {
        debugLevel->addItem(debugLevelList[i]);
    }
    debugLevel->setValue(3);
    listView->addProperty(debugLevel);

    // network render on/off
    networkRenderOn = new DzBoolProperty("yalux_network_render", false, false, false);
    networkRenderOn->setLabel("Enable Network Rendering");
//    listView->addProperty(networkRenderOn);

    // render server list
    renderServerList = new DzStringProperty("yalux_network_serverlist", false);
    renderServerList->setLabel("Network Render Servers (IP/hostnames)");
//    listView->addProperty(renderServerList);

    // Specular Mode
    specularMode = new DzEnumProperty("yalux_specular_mode", false, false);
    specularMode->setLabel("Specular Conversion Algorithm");
    for (i=0; i<specularModeList.count(); i++) {
        specularMode->addItem(specularModeList[i]);
    }
//    listView->addProperty(specularMode);

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
    haltThreshold = new DzFloatProperty("yalux_halt_threshold", false, false, 1.0);
    haltThreshold->setLabel("Target convergence threshold (1-haltthrehold)");
    listView->addProperty(haltTime);
    listView->addProperty(haltSPP);
    listView->addProperty(haltThreshold);

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
    tonemapExposureTime = new DzFloatProperty("yalux_tonemap_exposure", false, false, 0.5);
    tonemapExposureTime->setLabel("Linear Tonemapper Exposure time (seconds)");
    tonemapISO = new DzIntProperty("yalux_tonemap_iso", false, false, 200);
    tonemapISO->setLabel("Linear Tonemapper ISO");
    listView->addProperty(toneMapMethod);
    listView->addProperty(tonemapGamma);
    listView->addProperty(tonemapFstop);
    listView->addProperty(tonemapExposureTime);
    listView->addProperty(tonemapISO);


    addTonemapperAndEnvironment = new DzBoolProperty("yalux_add_tonemapper_environment", true, true, true);
    addTonemapperAndEnvironment->setLabel("Automatically Add Tonemapper and Environment Nodes");
    listView->addProperty(addTonemapperAndEnvironment);

    preferNormal = new DzBoolProperty("yalux_prefer_normal", true, false, false);
    preferNormal->setLabel("Prefer Normal Maps");
    listView->addProperty(preferNormal);

    /////// DEBUGGING OPTIONS ///////
    doBumpMaps = new DzBoolProperty("yalux_do_bumpmaps", true, false, true);
    doBumpMaps->setLabel("Render Bump Maps");
    listView->addProperty(doBumpMaps);

    doNormalMaps = new DzBoolProperty("yalux_do_normalmaps", true, false, false);
    doNormalMaps->setLabel("Render Normal Maps");
    listView->addProperty(doNormalMaps);

    doNormalAsBump = new DzBoolProperty("yalux_do_normalasbump", true, false, false);
    doNormalAsBump->setLabel("Render Normal As Bump");
    listView->addProperty(doNormalAsBump);

    doSpecular = new DzBoolProperty("yalux_do_specular", true, false, true);
    doSpecular->setLabel("Render Specular");
    listView->addProperty(doSpecular);

    doRoughnessMaps = new DzBoolProperty("yalux_do_roughness_maps", true, false, true);
    doRoughnessMaps->setLabel("Render Roughness Maps");
    listView->addProperty(doRoughnessMaps);

    doMetallic = new DzBoolProperty("yalux_do_metallic", true, false, true);
    doMetallic->setLabel("Render Metallic");
    listView->addProperty(doMetallic);

    doTranslucency = new DzBoolProperty("yalux_do_translucency", true, false, true);
    doTranslucency->setLabel("Render Translucency");
    listView->addProperty(doTranslucency);

    doSSS_Volume = new DzBoolProperty("yalux_do_sss_volume", true, false, true);
    doSSS_Volume->setLabel("Render SSS Volume");
    listView->addProperty(doSSS_Volume);

    doSSS_Absorption = new DzBoolProperty("yalux_do_sss_absorption", true, false, true);
    doSSS_Absorption->setLabel("Render SSS Absorption");
    listView->addProperty(doSSS_Absorption);

    doSSS_Scattering = new DzBoolProperty("yalux_do_sss_scattering", true, false, true);
    doSSS_Scattering->setLabel("Render SSS Scattering");
    listView->addProperty(doSSS_Scattering);

    doDebugSSS = new DzBoolProperty("yalux_do_debug_sss", true, false, false);
    doDebugSSS->setLabel("Debug SSS");
    listView->addProperty(doDebugSSS);

    connect(listView, SIGNAL(currentValueChanged()),
        this, SLOT(applyChanges()));

//    DzStyledFilePropertyWgt *widget = new DzStyledFilePropertyWgt(listView, "test string");
//    widget->addProperty(fileProperty);
//    layout2->addWidget(widget);
//    layout2->setAlignment(widget, Qt::AlignTop);

    this->setLayout(layout1);

    createDefaultSettings();
    loadSettings();
    applyChanges(); // call this method to update the YaLuxGlobals values with saved settings

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
    tonemapExposureTime->setDefaultValue(0.5);
    tonemapISO->setDefaultValue(200);
    toneMapMethod->setDefaultValue(0);
    renderMode->setDefaultValue(0);
    specularMode->setDefaultValue(0);
    maxTextureSize->setDefaultValue(4);
    networkRenderOn->setDefaultBoolValue(false);
    doBumpMaps->setDefaultBoolValue(false);
    doMetallic->setDefaultBoolValue(false);

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
        renderMode->setValue( settings->getIntValue( renderMode->getName()) );
        customRenderString->setValue( settings->getStringValue( customRenderString->getName()) );
        specularMode->setValue( settings->getIntValue( specularMode->getName()) );
        networkRenderOn->setBoolValue( settings->getBoolValue( networkRenderOn->getName()) );
        maxTextureSize->setValue( settings->getIntValue( maxTextureSize->getName()) );

        addTonemapperAndEnvironment->setBoolValue(settings->getBoolValue(addTonemapperAndEnvironment->getName()));
        preferNormal->setBoolValue(settings->getBoolValue(preferNormal->getName()));

        doBumpMaps->setBoolValue( settings->getBoolValue( doBumpMaps->getName()) );
        doNormalMaps->setBoolValue(settings->getBoolValue(doNormalMaps->getName()));
        doNormalAsBump->setBoolValue(settings->getBoolValue(doNormalAsBump->getName()));
        doMetallic->setBoolValue(settings->getBoolValue(doMetallic->getName()));
        doSSS_Volume->setBoolValue(settings->getBoolValue(doSSS_Volume->getName()));
        doDebugSSS->setBoolValue(settings->getBoolValue(doDebugSSS->getName()));

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
    settings->setIntValue(renderMode->getName(), renderMode->getValue());
    settings->setStringValue(customRenderString->getName(), customRenderString->getValue());
    settings->setIntValue(specularMode->getName(), specularMode->getValue());
    settings->setIntValue(maxTextureSize->getName(), maxTextureSize->getValue());
    settings->setBoolValue(networkRenderOn->getName(), networkRenderOn->getBoolValue());

    settings->setBoolValue(addTonemapperAndEnvironment->getName(), addTonemapperAndEnvironment->getBoolValue());
    settings->setBoolValue(preferNormal->getName(), preferNormal->getBoolValue());

    settings->setBoolValue(doBumpMaps->getName(), doBumpMaps->getBoolValue());
    settings->setBoolValue(doNormalMaps->getName(), doNormalMaps->getBoolValue());
    settings->setBoolValue(doNormalAsBump->getName(), doNormalAsBump->getBoolValue());
    settings->setBoolValue(doMetallic->getName(), doMetallic->getBoolValue());
    settings->setBoolValue(doSSS_Volume->getName(), doSSS_Volume->getBoolValue());
    settings->setBoolValue(doDebugSSS->getName(), doDebugSSS->getBoolValue());

    settings->setBoolValue("yalux_savedsettings_exist", true);

}

void	YaLuxOptionsFrame::applyChanges()
{
    // DEBUG
    if (YaLuxGlobal.debugLevel >= 1) // user info
    dzApp->log("yaluxplug: Render Options Panel: applying Changes.");
    // DEBUG FIX: THIS IS ONLY FOR MAC platform
#if defined( Q_OS_WIN )
    //YaLuxGlobal.LuxExecPath = execPath->getValue() + "/luxconsole.exe";
    YaLuxGlobal.LuxExecPath = execPath->getValue() + "/luxcoreconsole.exe";
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

    YaLuxGlobal.bNetworkRenderOn = networkRenderOn->getBoolValue();
    YaLuxGlobal.slaveNodeList = renderServerList->getValue().replace(" ",",").split(",", QString::SkipEmptyParts);
    YaLuxGlobal.renderMode = renderMode->getValue();
    YaLuxGlobal.customRenderString = customRenderString->getValue();
    YaLuxGlobal.specularMode = specularMode->getValue();
    if (maxTextureSize->getStringValue().toLower() == "none")
        YaLuxGlobal.maxTextureSize = -1;
    else
        YaLuxGlobal.maxTextureSize = maxTextureSize->getStringValue().toInt();

    YaLuxGlobal.bAddTonemapperAndEnvironement = addTonemapperAndEnvironment->getBoolValue();
    YaLuxGlobal.bPreferNormal = preferNormal->getBoolValue();

    YaLuxGlobal.bDoBumpMaps = doBumpMaps->getBoolValue();
    YaLuxGlobal.bDoNormalMaps = doNormalMaps->getBoolValue();
    YaLuxGlobal.bDoNormalAsBump = doNormalAsBump->getBoolValue();

    YaLuxGlobal.bDoMetallic = doMetallic->getBoolValue();
    YaLuxGlobal.bDoSpecular = doSpecular->getBoolValue();
    YaLuxGlobal.bDoRoughnessMaps = doRoughnessMaps->getBoolValue();
    YaLuxGlobal.bDoTranslucency = doTranslucency->getBoolValue();
    YaLuxGlobal.bDoSSSVolume = doSSS_Volume->getBoolValue();
    YaLuxGlobal.bDoSSSAbsorption = doSSS_Absorption->getBoolValue();
    YaLuxGlobal.bDoSSSScattering = doSSS_Scattering->getBoolValue();
    YaLuxGlobal.bDoDebugSSS = doDebugSSS->getBoolValue();

};

void	YaLuxOptionsFrame::resetOptions()
{
    // TODO: should I set to default or reload from saved data?
    // DEBUG
    if (YaLuxGlobal.debugLevel >=1) // user data
    dzApp->log("yaluxplug: Render Options Panel: resetting options.");

};

bool    YaLuxOptionsFrame::applyValid() const
{
    if (QFile(execPath->getValue()).exists() == false)
    {
        return false;
    }

    return true;
}

void	YaLuxOptionsFrame::restoreOptions( DzRenderOptions *options )
{
    //execPath->setValue("");
    argumentList->setValue("");
    showLuxWindow->setBoolValue(false);
    saveAlphaChannel->setBoolValue(false);
    haltTime->setValue(600);
    haltSPP->setValue(1000);
    haltThreshold->setValue(0.99);
    debugLevel->setValue(3);
    tonemapGamma->setValue(2.2);
    tonemapFstop->setValue(2.8);
    tonemapExposureTime->setValue(0.001);
    tonemapISO->setValue(100);
    toneMapMethod->setValue(1);
    renderServerList->setValue("");
    renderMode->setValue(0);
    customRenderString->setValue("");
    specularMode->setValue(0);
    networkRenderOn->setBoolValue(false);
    maxTextureSize->setValue(4);
    doBumpMaps->setBoolValue(false);
    doMetallic->setBoolValue(false);

    applyChanges();
};

#include "moc_optionsframe.cpp"
