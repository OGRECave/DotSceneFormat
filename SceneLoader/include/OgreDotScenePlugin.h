/*
 * OgrePlugin.h
 *
 *  Created on: 21.04.2018
 *      Author: pavel
 */

#pragma once

#include "OgrePlugin.h"
#include "OgreSceneLoader.h"

class DotScenePlugin : public Ogre::Plugin
{
    const Ogre::String& getName() const;

    void install() {}
    void initialise();
    void shutdown();
    void uninstall() {}
protected:
    Ogre::SceneLoader* mDotSceneLoader;
};
