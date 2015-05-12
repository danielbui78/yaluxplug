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
#include "dzstyledfilepropertywgt.h"
#include "dzsidenavpropertylistview.h"
#include "dzstringproperty.h"
#include "dzfilternavigationbar.h"
#include "dzappsettings.h"

#include "optionsframe.h"
#include "plugin.h"

YaLuxOptionsFrame::YaLuxOptionsFrame() : DzOptionsFrame("yaluxplug Options Frame", 0, "yaluxplug Options Frame")
{
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
    fileProperty = new DzFileProperty("Program Path to Luxrender", false);
    fileProperty->setType(DzFileProperty::FileType::FileOpen);
#ifdef Q_OS_WIN
    fileProperty->setFilter("FilterName (*.exe)");
#elif define Q_WS_MAC
    fileProperty->setFilter("FilterName (*.app)");
#endif
    listView->addProperty(fileProperty);

    argumentsProperty = new DzStringProperty("Executable arguments", false);
    listView->addProperty(argumentsProperty);


//    DzStyledFilePropertyWgt *widget = new DzStyledFilePropertyWgt(listView, "test string");
//    widget->addProperty(fileProperty);
//    layout2->addWidget(widget);
//    layout2->setAlignment(widget, Qt::AlignTop);

    this->setLayout(layout1);

    loadSavedSettings();

    return;
}

void	YaLuxOptionsFrame::loadSavedSettings()
{
    fileProperty->setValue( settings->getStringValue("LuxrenderExecPath") );
    argumentsProperty->setValue( settings->getStringValue("CmdLineArgs") );

}

void	YaLuxOptionsFrame::applyChanges()
{
    // DEBUG
    dzApp->log("yaluxplug: Render Options Panel: applying Changes.");
    // DEBUG FIX: THIS IS ONLY FOR MAC platform
#ifdef Q_OS_WIN
    YaLuxGlobal.LuxExecPath = fileProperty->getValue();
#elif define Q_WS_MAC
    YaLuxGlobal.LuxExecPath = fileProperty->getValue() + "/LuxRender.app/Contents/MacOS/luxconsole";
#endif
    YaLuxGlobal.CmdLineArgs = argumentsProperty->getValue();

    settings->setStringValue("LuxrenderExecPath", fileProperty->getValue());
    settings->setStringValue("CmdLineArgs", argumentsProperty->getValue());

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
