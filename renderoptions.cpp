//
//  renderoptions.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 4/24/15.
//
//

#include "renderoptions.h"

YaLuxRenderOptions::YaLuxRenderOptions() : DzOptionsFrame("yaluxplug Render Options", 0, "yaluxplug Render Options")
{
    return;
}

void	YaLuxRenderOptions::applyChanges()
{
    
};

void	YaLuxRenderOptions::resetOptions()
{
    
};

bool    YaLuxRenderOptions::applyValid() const 
{
    return true; 
}

void	YaLuxRenderOptions::restoreOptions( DzRenderOptions *options )
{

};
