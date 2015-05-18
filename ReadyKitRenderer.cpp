//
//  ReadyKitRenderer.cpp
//  yaluxplug
//
//  Created by Daniel Bui on 5/17/15.
//  Copyright (c) 2015 Daniel Bui. All rights reserved.
//

#include "ReadyKitRenderer.h"


////////////////////
// RendererGraphicsState Class
//      Method definitions for the GrapicsState Class.  Subclass this class to store all of your
//      relevant state-related information.
////////////////////

// Constructor for the GraphicsState object, which will manage all of our graphics contexts
RendererGraphicsState::RendererGraphicsState()
{
    // Initialize the graphics states
    rendererName = "ReadyKitRenderer";
    renderOptions = new DzRenderOptions;
//    renderOptionsFrame = new DzOptionsFrame;

    // Initialize Default Object Attributes
    ObjectAttributes *oa = new ObjectAttributes;
    attributeStack.append(*oa);

}

// Returns the currently active attribute from the graphics stack
QString RendererGraphicsState::getObjectAttribute()
{
    ObjectAttributes *activeAttributes = &attributeStack.last();
    return activeAttributes->objectAttribute;
}




////////////////////
// ReadyKitRenderer Class
//      This is the main renderer plugin class.  Subclass this class and override whatever DzRenderer API calls
//      you will need to make your renderer work.  Ex, you will need to override at least some of the renderman API calls
//      to implement a renderer that uses the renderman pipeline.
/////////////////////

// Constructor
ReadyKitRenderer::ReadyKitRenderer()
{
    // Initialize the plugin and instantiate the graphics state
    // Remember, the graphicsState should contain all the scene-related information your specific renderer will need.
    // See RendererGrahpicsState Class above.
    graphicsState = new RendererGraphicsState;

}

// DzOptionsFrame* getOptionsFrame
//      Returns a pointer to an OptionsFrame object.  This will be used by DazGUI to draw the Renderer Advanced Settings tab
DzOptionsFrame* ReadyKitRenderer::getOptionsFrame() const
{
    return graphicsState->renderOptionsFrame;
}

