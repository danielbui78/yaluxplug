//
//  pluginmain.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#include "dzplugin.h"
#include "dzapp.h"

#include "version.h"
#include "renderer.h"



DZ_PLUGIN_DEFINITION( "yaluxplug" );


DZ_PLUGIN_AUTHOR( "DB" );


DZ_PLUGIN_VERSION( PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD );


DZ_PLUGIN_DESCRIPTION( "Yet another Luxrender plugin for Daz Studio.  You will see it as an additional Renderer option on the Render Pane." );

DZ_PLUGIN_CLASS_GUID( YaLuxRender, 3E8A430D-956C-41CB-967F-F82797B575FF );


