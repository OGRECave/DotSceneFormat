#ifndef DOT_SCENELOADER_H
#define DOT_SCENELOADER_H

// Includes
#include <OgreString.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreColourValue.h>
#include <OgreResourceGroupManager.h>
#include <OgreSceneLoader.h>

    // Forward declarations
    namespace Ogre
    {
        class SceneManager;
        class SceneNode;
        class TerrainGroup;
    }

    namespace pugi
    {
        class xml_node;
    }

    class DotSceneLoader : public Ogre::SceneLoader
    {
    public:        
        DotSceneLoader();
        virtual ~DotSceneLoader();

        void load(Ogre::DataStreamPtr& stream, const Ogre::String& groupName,
                  Ogre::SceneNode* rootNode);

        void parseDotScene(const Ogre::String& SceneName,
                           const Ogre::String& groupName,
                           Ogre::SceneNode* pAttachNode,
                           const Ogre::String& sPrependNode = "");

        Ogre::TerrainGroup* getTerrainGroup() { return mTerrainGroup; }

        const Ogre::ColourValue& getBackgroundColour() {
            return mBackgroundColour;
        }
    protected:
        void processScene(pugi::xml_node& XMLRoot);

        void processNodes(pugi::xml_node& XMLNode);
        void processExternals(pugi::xml_node& XMLNode);
        void processEnvironment(pugi::xml_node& XMLNode);
        void processTerrainGroup(pugi::xml_node& XMLNode);
        void processTerrain(pugi::xml_node& XMLNode);
        void processBlendmaps(pugi::xml_node& XMLNode);
        void processUserData(pugi::xml_node& XMLNode, Ogre::UserObjectBindings& userData);
        void processLight(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent = 0);
        void processCamera(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent = 0);

        void processNode(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent = 0);
        void processLookTarget(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);
        void processTrackTarget(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);
        void processEntity(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);
        void processParticleSystem(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);
        void processBillboardSet(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);
        void processPlane(pugi::xml_node& XMLNode, Ogre::SceneNode *pParent);

        void processFog(pugi::xml_node& XMLNode);
        void processSkyBox(pugi::xml_node& XMLNode);
        void processSkyDome(pugi::xml_node& XMLNode);
        void processSkyPlane(pugi::xml_node& XMLNode);

        void processLightRange(pugi::xml_node& XMLNode, Ogre::Light *pLight);
        void processLightAttenuation(pugi::xml_node& XMLNode, Ogre::Light *pLight);

        Ogre::SceneManager *mSceneMgr;
        Ogre::SceneNode *mAttachNode;
        Ogre::String m_sGroupName;
        Ogre::String m_sPrependNode;
        Ogre::TerrainGroup* mTerrainGroup;
        Ogre::ColourValue mBackgroundColour;
    };

#endif // DOT_SCENELOADER_H
