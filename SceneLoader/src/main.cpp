#include <Ogre.h>
#include <OgreApplicationContext.h>
#include <OgreCameraMan.h>

#include "DotSceneLoader.h"

class SceneLoadSample : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    SceneLoadSample();
    void setup();
    bool keyPressed(const OgreBites::KeyboardEvent& evt);
};

SceneLoadSample::SceneLoadSample() : OgreBites::ApplicationContext("SceneLoadSample")
{
    addInputListener(this);
}

bool SceneLoadSample::keyPressed(const OgreBites::KeyboardEvent& evt)
{
    if (evt.keysym.sym == SDLK_ESCAPE)
    {
        getRoot()->queueEndRendering();
    }
    return true;
}

void SceneLoadSample::setup(void)
{
    // do not forget to call the base first
    OgreBites::ApplicationContext::setup();

    // get a pointer to the already created root
    Ogre::Root* root = getRoot();
    Ogre::SceneManager* scnMgr = root->createSceneManager("DefaultSceneManager");

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    shadergen->addSceneManager(scnMgr);

    DotSceneLoader loader;
    loader.parseDotScene("example.scene", "General", scnMgr);

    // create the camera
    Ogre::Camera* cam = scnMgr->getCameras().begin()->second;
    cam->setAutoAspectRatio(true);

    OgreBites::CameraMan* camman = new OgreBites::CameraMan(cam->getParentSceneNode());
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
