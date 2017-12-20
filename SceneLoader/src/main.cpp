#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

#include "DotSceneLoader.h"

class SceneLoadSample : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    SceneLoadSample();
    void setup();
    void locateResources();
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
};

SceneLoadSample::SceneLoadSample() : OgreBites::ApplicationContext("SceneLoadSample")
{
}

bool SceneLoadSample::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}

void SceneLoadSample::locateResources()
{
    OgreBites::ApplicationContext::locateResources();
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(".", "FileSystem", "Scene");
}

void SceneLoadSample::setup(void)
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();

    addInputListener(this);

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager("DefaultSceneManager");

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    DotSceneLoader loader;
    loader.parseDotScene("example.scene", "Scene", scnMgr);

    // create the camera
    Ogre::Camera* cam = scnMgr->getCameras().begin()->second;
    cam->setAutoAspectRatio(true);

    OgreBites::CameraMan* camman = new OgreBites::CameraMan(cam->getParentSceneNode());
    camman->setStyle(OgreBites::CS_ORBIT);
    addInputListener(camman);

    // and tell it to render into the main window
    getRenderWindow()->addViewport(cam)->setBackgroundColour(loader.getBackgroundColour());
}

int main(void)
{
    SceneLoadSample app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();
}
