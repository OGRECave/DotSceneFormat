/*
 * OgrePlugin.cpp
 *
 *  Created on: 21.04.2018
 *      Author: pavel
 */

#include "OgreDotScenePlugin.h"
#include "DotSceneLoader.h"
#include "OgreRoot.h"

const Ogre::String& DotScenePlugin::getName() const {
    static Ogre::String name = "DotScene Loader";
    return name;
}

void DotScenePlugin::initialise() {
    mDotSceneLoader = new DotSceneLoader();
}

void DotScenePlugin::shutdown() {
    delete mDotSceneLoader;
}

#ifndef OGRE_STATIC_LIB
    extern "C" void dllStartPlugin();
    extern "C" void dllStopPlugin();

    static DotScenePlugin plugin;

    extern "C" void dllStartPlugin()
    {
        Ogre::Root::getSingleton().installPlugin(&plugin);
    }
    extern "C" void dllStopPlugin()
    {
        Ogre::Root::getSingleton().uninstallPlugin(&plugin);
    }
#endif
