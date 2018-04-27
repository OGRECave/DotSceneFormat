#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

#include <iostream>

#include <OgreSceneLoaderManager.h>
#include "OgreDotScenePlugin.h"

class SceneLoadSample : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
    Ogre::String mSceneFile;
    Ogre::String mResourcePath;
public:
    SceneLoadSample(const Ogre::String& scene);
    void setup();
    void locateResources();
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
};

SceneLoadSample::SceneLoadSample(const Ogre::String& scene)
    : OgreBites::ApplicationContext("SceneLoadSample")
{

    Ogre::StringUtil::splitFilename(scene, mSceneFile, mResourcePath);
}

bool SceneLoadSample::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    using namespace OgreBites;
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}

void SceneLoadSample::locateResources()
{
    OgreBites::ApplicationContext::locateResources();
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(mResourcePath, "FileSystem", "Scene");
}

void SceneLoadSample::setup(void)
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();

    addInputListener(this);

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    // register the scene loader
    getRoot()->installPlugin(new DotScenePlugin);

    Ogre::SceneManager* scnMgr = root->createSceneManager("DefaultSceneManager");

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    Ogre::SceneLoaderManager::getSingleton().load(mSceneFile, "Scene", scnMgr->getRootSceneNode());

    // create the camera
    Ogre::Camera* cam = scnMgr->getCameras().begin()->second;
    cam->setAutoAspectRatio(true);

    OgreBites::CameraMan* camman = new OgreBites::CameraMan(cam->getParentSceneNode());
    camman->setStyle(OgreBites::CS_ORBIT);
    addInputListener(camman);

    // and tell it to render into the main window
    getRenderWindow()->addViewport(cam);
}

int main(int argc, char* argv[])
{
    if(argc != 2) {
        std::cout << "usage: " << argv[0] << " file.scene" << std::endl;
        return 1;
    }

    SceneLoadSample app(argv[1]);
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
    return 0;
}
