#include "DotSceneLoader.h"
#include <Ogre.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainMaterialGeneratorA.h>

#include <OgreSceneLoaderManager.h>

using namespace Ogre;

namespace
{
String getAttrib(rapidxml::xml_node<> *XMLNode, const String &attrib, const String &defaultValue = "")
{
    if (XMLNode->first_attribute(attrib.c_str()))
        return XMLNode->first_attribute(attrib.c_str())->value();
    else
        return defaultValue;
}

Real getAttribReal(rapidxml::xml_node<> *XMLNode, const String &attrib, Real defaultValue = 0)
{
    if (XMLNode->first_attribute(attrib.c_str()))
        return StringConverter::parseReal(XMLNode->first_attribute(attrib.c_str())->value());
    else
        return defaultValue;
}

bool getAttribBool(rapidxml::xml_node<> *XMLNode, const String &attrib, bool defaultValue = false)
{
    if (!XMLNode->first_attribute(attrib.c_str()))
        return defaultValue;

    if (String(XMLNode->first_attribute(attrib.c_str())->value()) == "true")
        return true;

    return false;
}

Vector3 parseVector3(rapidxml::xml_node<> *XMLNode)
{
    return Vector3(
        StringConverter::parseReal(XMLNode->first_attribute("x")->value()),
        StringConverter::parseReal(XMLNode->first_attribute("y")->value()),
        StringConverter::parseReal(XMLNode->first_attribute("z")->value()));
}

Quaternion parseQuaternion(rapidxml::xml_node<> *XMLNode)
{
    //! @todo Fix this crap!

    Quaternion orientation;

    if (XMLNode->first_attribute("qw"))
    {
        orientation.w = StringConverter::parseReal(XMLNode->first_attribute("qw")->value());
        orientation.x = StringConverter::parseReal(XMLNode->first_attribute("qx")->value());
        orientation.y = StringConverter::parseReal(XMLNode->first_attribute("qy")->value());
        orientation.z = StringConverter::parseReal(XMLNode->first_attribute("qz")->value());
    }
    else if (XMLNode->first_attribute("axisX"))
    {
        Vector3 axis;
        axis.x = StringConverter::parseReal(XMLNode->first_attribute("axisX")->value());
        axis.y = StringConverter::parseReal(XMLNode->first_attribute("axisY")->value());
        axis.z = StringConverter::parseReal(XMLNode->first_attribute("axisZ")->value());
        Real angle = StringConverter::parseReal(XMLNode->first_attribute("angle")->value());
        ;
        orientation.FromAngleAxis(Angle(angle), axis);
    }
    else if (XMLNode->first_attribute("angleX"))
    {
        Matrix3 rot;
        rot.FromEulerAnglesXYZ(
            StringConverter::parseAngle(XMLNode->first_attribute("angleX")->value()),
            StringConverter::parseAngle(XMLNode->first_attribute("angleY")->value()),
            StringConverter::parseAngle(XMLNode->first_attribute("angleZ")->value()));
        orientation.FromRotationMatrix(rot);
    }
    else if (XMLNode->first_attribute("x"))
    {
        orientation.x = StringConverter::parseReal(XMLNode->first_attribute("x")->value());
        orientation.y = StringConverter::parseReal(XMLNode->first_attribute("y")->value());
        orientation.z = StringConverter::parseReal(XMLNode->first_attribute("z")->value());
        orientation.w = StringConverter::parseReal(XMLNode->first_attribute("w")->value());
    }
    else if (XMLNode->first_attribute("w"))
    {
        orientation.w = StringConverter::parseReal(XMLNode->first_attribute("w")->value());
        orientation.x = StringConverter::parseReal(XMLNode->first_attribute("x")->value());
        orientation.y = StringConverter::parseReal(XMLNode->first_attribute("y")->value());
        orientation.z = StringConverter::parseReal(XMLNode->first_attribute("z")->value());
    }

    return orientation;
}

ColourValue parseColour(rapidxml::xml_node<> *XMLNode)
{
    return ColourValue(
        StringConverter::parseReal(XMLNode->first_attribute("r")->value()),
        StringConverter::parseReal(XMLNode->first_attribute("g")->value()),
        StringConverter::parseReal(XMLNode->first_attribute("b")->value()),
        XMLNode->first_attribute("a") != NULL ? StringConverter::parseReal(XMLNode->first_attribute("a")->value()) : 1);
}
} // namespace

DotSceneLoader::DotSceneLoader() : mSceneMgr(0), mTerrainGroup(0), mBackgroundColour(ColourValue::Black)
{
    mTerrainGlobalOptions = OGRE_NEW TerrainGlobalOptions();
    SceneLoaderManager::getSingleton().registerSceneLoader("DotScene", {".scene"}, this);
}

DotSceneLoader::~DotSceneLoader()
{
    SceneLoaderManager::getSingleton().unregisterSceneLoader("DotScene");

    if (mTerrainGroup)
    {
        OGRE_DELETE mTerrainGroup;
    }

    OGRE_DELETE mTerrainGlobalOptions;
}

void DotSceneLoader::parseDotScene(const String &SceneName, const String &groupName,
                                   SceneNode *pAttachNode, const String &sPrependNode)
{
    m_sPrependNode = sPrependNode;
    DataStreamPtr stream = Root::openFileStream(SceneName, groupName);
    load(stream, groupName, pAttachNode);
}

void DotSceneLoader::load(DataStreamPtr &stream, const String &groupName,
                          SceneNode *rootNode)
{
    m_sGroupName = groupName;
    mSceneMgr = rootNode->getCreator();
    staticObjects.clear();
    dynamicObjects.clear();

    rapidxml::xml_document<> XMLDoc; // character type defaults to char
    rapidxml::xml_node<> *XMLRoot;

    char *scene = strdup(stream->getAsString().c_str());
    XMLDoc.parse<0>(scene);

    // Grab the scene node
    XMLRoot = XMLDoc.first_node("scene");

    // Validate the File
    if (getAttrib(XMLRoot, "formatVersion", "") == "")
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error: Invalid .scene File. Missing <scene>");
        delete scene;
        return;
    }

    // figure out where to attach any nodes we create
    mAttachNode = rootNode;

    // Process the scene
    processScene(XMLRoot);

    delete scene;
}

void DotSceneLoader::processScene(rapidxml::xml_node<> *XMLRoot)
{
    // Process the scene parameters
    String version = getAttrib(XMLRoot, "formatVersion", "unknown");

    String message = "[DotSceneLoader] Parsing dotScene file with version " + version;
    if (XMLRoot->first_attribute("ID"))
        message += ", id " + String(XMLRoot->first_attribute("ID")->value());
    if (XMLRoot->first_attribute("sceneManager"))
        message += ", scene manager " + String(XMLRoot->first_attribute("sceneManager")->value());
    if (XMLRoot->first_attribute("minOgreVersion"))
        message += ", min. Ogre version " + String(XMLRoot->first_attribute("minOgreVersion")->value());
    if (XMLRoot->first_attribute("author"))
        message += ", author " + String(XMLRoot->first_attribute("author")->value());

    LogManager::getSingleton().logMessage(message);

    // Process environment (?)
    if (auto pElement = XMLRoot->first_node("environment"))
        processEnvironment(pElement);

    // Process nodes (?)
    if (auto pElement = XMLRoot->first_node("nodes"))
        processNodes(pElement);

    // Process externals (?)
    if (auto pElement = XMLRoot->first_node("externals"))
        processExternals(pElement);

    // Process userDataReference (?)
    if (auto pElement = XMLRoot->first_node("userData"))
        processUserData(pElement, mAttachNode->getUserObjectBindings());

    // Process light (?)
    if (auto pElement = XMLRoot->first_node("light"))
        processLight(pElement);

    // Process camera (?)
    if (auto pElement = XMLRoot->first_node("camera"))
        processCamera(pElement);

    // Process terrain (?)
    if (auto pElement = XMLRoot->first_node("terrain"))
        processTerrain(pElement);
}

void DotSceneLoader::processNodes(rapidxml::xml_node<> *XMLNode)
{
    rapidxml::xml_node<> *pElement;

    // Process node (*)
    pElement = XMLNode->first_node("node");
    while (pElement)
    {
        processNode(pElement);
        pElement = pElement->next_sibling("node");
    }

    // Process position (?)
    if (auto pElement = XMLNode->first_node("position"))
    {
        mAttachNode->setPosition(parseVector3(pElement));
        mAttachNode->setInitialState();
    }

    // Process rotation (?)
    if (auto pElement = XMLNode->first_node("rotation"))
    {
        mAttachNode->setOrientation(parseQuaternion(pElement));
        mAttachNode->setInitialState();
    }

    // Process scale (?)
    if (auto pElement = XMLNode->first_node("scale"))
    {
        mAttachNode->setScale(parseVector3(pElement));
        mAttachNode->setInitialState();
    }
}

void DotSceneLoader::processExternals(rapidxml::xml_node<> *XMLNode)
{
    //! @todo Implement this
}

void DotSceneLoader::processEnvironment(rapidxml::xml_node<> *XMLNode)
{
    // Process camera (?)
    if (auto pElement = XMLNode->first_node("camera"))
        processCamera(pElement);

    // Process fog (?)
    if (auto pElement = XMLNode->first_node("fog"))
        processFog(pElement);

    // Process skyBox (?)
    if (auto pElement = XMLNode->first_node("skyBox"))
        processSkyBox(pElement);

    // Process skyDome (?)
    if (auto pElement = XMLNode->first_node("skyDome"))
        processSkyDome(pElement);

    // Process skyPlane (?)
    if (auto pElement = XMLNode->first_node("skyPlane"))
        processSkyPlane(pElement);

    // Process clipping (?)
    if (auto pElement = XMLNode->first_node("clipping"))
        processClipping(pElement);

    // Process colourAmbient (?)
    if (auto pElement = XMLNode->first_node("colourAmbient"))
        mSceneMgr->setAmbientLight(parseColour(pElement));

    // Process colourBackground (?)
    if (auto pElement = XMLNode->first_node("colourBackground"))
        mBackgroundColour = parseColour(pElement);
}

void DotSceneLoader::processTerrain(rapidxml::xml_node<> *XMLNode)
{
    Real worldSize = getAttribReal(XMLNode, "worldSize");
    int mapSize = StringConverter::parseInt(XMLNode->first_attribute("mapSize")->value());
    // TODO: unused
    // bool colourmapEnabled = getAttribBool(XMLNode, "colourmapEnabled");
    // int colourMapTextureSize = StringConverter::parseInt(XMLNode->first_attribute("colourMapTextureSize")->value());
    int compositeMapDistance = StringConverter::parseInt(XMLNode->first_attribute("tuningCompositeMapDistance")->value());
    int maxPixelError = StringConverter::parseInt(XMLNode->first_attribute("tuningMaxPixelError")->value());

    Vector3 lightdir(0, -0.3, 0.75);
    lightdir.normalise();
    Light *l = mSceneMgr->createLight("tstLight");
    l->setType(Light::LT_DIRECTIONAL);
    l->setDirection(lightdir);
    l->setDiffuseColour(ColourValue(1.0, 1.0, 1.0));
    l->setSpecularColour(ColourValue(0.4, 0.4, 0.4));
    mSceneMgr->setAmbientLight(ColourValue(0.6, 0.6, 0.6));

    mTerrainGlobalOptions->setMaxPixelError((Real)maxPixelError);
    mTerrainGlobalOptions->setCompositeMapDistance((Real)compositeMapDistance);
    mTerrainGlobalOptions->setLightMapDirection(lightdir);
    mTerrainGlobalOptions->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainGlobalOptions->setCompositeMapDiffuse(l->getDiffuseColour());

    mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z, mapSize, worldSize);
    mTerrainGroup->setOrigin(Vector3::ZERO);

    mTerrainGroup->setResourceGroup("General");

    // Process terrain pages (*)
    if (auto pElement = XMLNode->first_node("terrainPages"))
    {
        auto pPageElement = pElement->first_node("terrainPage");
        while (pPageElement)
        {
            processTerrainPage(pPageElement);
            pPageElement = pPageElement->next_sibling("terrainPage");
        }
    }
    mTerrainGroup->loadAllTerrains(true);

    mTerrainGroup->freeTemporaryResources();
    //mTerrain->setPosition(mTerrainPosition);
}

void DotSceneLoader::processTerrainPage(rapidxml::xml_node<> *XMLNode)
{
    String name = getAttrib(XMLNode, "name");
    int pageX = StringConverter::parseInt(XMLNode->first_attribute("pageX")->value());
    int pageY = StringConverter::parseInt(XMLNode->first_attribute("pageY")->value());

    if (ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), name))
    {
        mTerrainGroup->defineTerrain(pageX, pageY, name);
    }
}

void DotSceneLoader::processLight(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String id = getAttrib(XMLNode, "id");

    // Create the light
    Light *pLight = mSceneMgr->createLight(name);
    if (pParent)
        pParent->attachObject(pLight);

    String sValue = getAttrib(XMLNode, "type");
    if (sValue == "point")
        pLight->setType(Light::LT_POINT);
    else if (sValue == "directional")
        pLight->setType(Light::LT_DIRECTIONAL);
    else if (sValue == "spot")
        pLight->setType(Light::LT_SPOTLIGHT);
    else if (sValue == "radPoint")
        pLight->setType(Light::LT_POINT);

    // lights are oriented using SceneNodes that expect -Z to be the default direction
    // exporters should not write normal or direction if they attach lights to nodes
    pLight->setDirection(Vector3::NEGATIVE_UNIT_Z);

    pLight->setVisible(getAttribBool(XMLNode, "visible", true));
    pLight->setCastShadows(getAttribBool(XMLNode, "castShadows", true));
    pLight->setPowerScale(getAttribReal(XMLNode, "powerScale", 1.0));

    // Process position (?)
    if (auto pElement = XMLNode->first_node("position"))
        pLight->setPosition(parseVector3(pElement));

    // Process normal (?)
    if (auto pElement = XMLNode->first_node("normal"))
        pLight->setDirection(parseVector3(pElement));

    if (auto pElement = XMLNode->first_node("directionVector"))
    {
        pLight->setDirection(parseVector3(pElement));
    }

    // Process colourDiffuse (?)
    if (auto pElement = XMLNode->first_node("colourDiffuse"))
        pLight->setDiffuseColour(parseColour(pElement));

    // Process colourSpecular (?)
    if (auto pElement = XMLNode->first_node("colourSpecular"))
        pLight->setSpecularColour(parseColour(pElement));

    if (sValue != "directional")
    {
        // Process lightRange (?)
        if (auto pElement = XMLNode->first_node("lightRange"))
            processLightRange(pElement, pLight);

        // Process lightAttenuation (?)
        if (auto pElement = XMLNode->first_node("lightAttenuation"))
            processLightAttenuation(pElement, pLight);
    }
    // Process userDataReference (?)
    if (auto pElement = XMLNode->first_node("userData"))
        processUserData(pElement, pLight->getUserObjectBindings());
}

void DotSceneLoader::processCamera(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String id = getAttrib(XMLNode, "id");
    // Real fov = getAttribReal(XMLNode, "fov", 45);
    Real aspectRatio = getAttribReal(XMLNode, "aspectRatio", 1.3333);
    String projectionType = getAttrib(XMLNode, "projectionType", "perspective");

    // Create the camera
    Camera *pCamera = mSceneMgr->createCamera(name);

    // construct a scenenode is no parent
    if (!pParent)
        pParent = mAttachNode->createChildSceneNode(name);

    pParent->attachObject(pCamera);

    // Set the field-of-view
    //! @todo Is this always in degrees?
    //pCamera->setFOVy(Degree(fov));

    // Set the aspect ratio
    pCamera->setAspectRatio(aspectRatio);

    // Set the projection type
    if (projectionType == "perspective")
        pCamera->setProjectionType(PT_PERSPECTIVE);
    else if (projectionType == "orthographic")
        pCamera->setProjectionType(PT_ORTHOGRAPHIC);

    // Process clipping (?)
    if (auto pElement = XMLNode->first_node("clipping"))
    {
        Real nearDist = getAttribReal(pElement, "near");
        pCamera->setNearClipDistance(nearDist);

        Real farDist = getAttribReal(pElement, "far");
        pCamera->setFarClipDistance(farDist);
    }

    // Process position (?)
    if (auto pElement = XMLNode->first_node("position"))
        pCamera->setPosition(parseVector3(pElement));

    // Process rotation (?)
    if (auto pElement = XMLNode->first_node("rotation"))
        pCamera->setOrientation(parseQuaternion(pElement));

    // Process normal (?)
    if (auto pElement = XMLNode->first_node("normal"))
        ; //!< @todo What to do with this element?

    // Process lookTarget (?)
    if (auto pElement = XMLNode->first_node("lookTarget"))
        ; //!< @todo Implement the camera look target

    // Process trackTarget (?)
    if (auto pElement = XMLNode->first_node("trackTarget"))
        ; //!< @todo Implement the camera track target

    // Process userDataReference (?)
    if (auto pElement = XMLNode->first_node("userData"))
        processUserData(pElement, static_cast<MovableObject *>(pCamera)->getUserObjectBindings());
}

void DotSceneLoader::processNode(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Construct the node's name
    String name = m_sPrependNode + getAttrib(XMLNode, "name");

    // Create the scene node
    SceneNode *pNode;
    if (name.empty())
    {
        // Let Ogre choose the name
        if (pParent)
            pNode = pParent->createChildSceneNode();
        else
            pNode = mAttachNode->createChildSceneNode();
    }
    else
    {
        // Provide the name
        if (pParent)
            pNode = pParent->createChildSceneNode(name);
        else
            pNode = mAttachNode->createChildSceneNode(name);
    }

    // Process other attributes
    String id = getAttrib(XMLNode, "id");
    //bool isTarget = getAttribBool(XMLNode, "isTarget"); // TODO: unused

    // Process position (?)
    if (auto pElement = XMLNode->first_node("position"))
    {
        pNode->setPosition(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process rotation (?)
    if (auto pElement = XMLNode->first_node("rotation"))
    {
        pNode->setOrientation(parseQuaternion(pElement));
        pNode->setInitialState();
    }

    // Process scale (?)
    if (auto pElement = XMLNode->first_node("scale"))
    {
        pNode->setScale(parseVector3(pElement));
        pNode->setInitialState();
    }

    // Process lookTarget (?)
    if (auto pElement = XMLNode->first_node("lookTarget"))
        processLookTarget(pElement, pNode);

    // Process trackTarget (?)
    if (auto pElement = XMLNode->first_node("trackTarget"))
        processTrackTarget(pElement, pNode);

    rapidxml::xml_node<> *pElement;
    // Process node (*)
    pElement = XMLNode->first_node("node");
    while (pElement)
    {
        processNode(pElement, pNode);
        pElement = pElement->next_sibling("node");
    }

    // Process entity (*)
    pElement = XMLNode->first_node("entity");
    while (pElement)
    {
        processEntity(pElement, pNode);
        pElement = pElement->next_sibling("entity");
    }

    // Process light (*)
    pElement = XMLNode->first_node("light");
    while (pElement)
    {
        processLight(pElement, pNode);
        pElement = pElement->next_sibling("light");
    }

    // Process camera (*)
    pElement = XMLNode->first_node("camera");
    while (pElement)
    {
        processCamera(pElement, pNode);
        pElement = pElement->next_sibling("camera");
    }

    // Process particleSystem (*)
    pElement = XMLNode->first_node("particleSystem");
    while (pElement)
    {
        processParticleSystem(pElement, pNode);
        pElement = pElement->next_sibling("particleSystem");
    }

    // Process billboardSet (*)
    pElement = XMLNode->first_node("billboardSet");
    while (pElement)
    {
        processBillboardSet(pElement, pNode);
        pElement = pElement->next_sibling("billboardSet");
    }

    // Process plane (*)
    pElement = XMLNode->first_node("plane");
    while (pElement)
    {
        processPlane(pElement, pNode);
        pElement = pElement->next_sibling("plane");
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode->first_node("userData"))
        processUserData(pElement, pNode->getUserObjectBindings());
}

void DotSceneLoader::processLookTarget(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    //! @todo Is this correct? Cause I don't have a clue actually

    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

    Node::TransformSpace relativeTo = Node::TS_PARENT;
    String sValue = getAttrib(XMLNode, "relativeTo");
    if (sValue == "local")
        relativeTo = Node::TS_LOCAL;
    else if (sValue == "parent")
        relativeTo = Node::TS_PARENT;
    else if (sValue == "world")
        relativeTo = Node::TS_WORLD;

    // Process position (?)
    Vector3 position;
    if (auto pElement = XMLNode->first_node("position"))
        position = parseVector3(pElement);

    // Process localDirection (?)
    Vector3 localDirection = Vector3::NEGATIVE_UNIT_Z;
    if (auto pElement = XMLNode->first_node("localDirection"))
        localDirection = parseVector3(pElement);

    // Setup the look target
    try
    {
        if (!nodeName.empty())
        {
            SceneNode *pLookNode = mSceneMgr->getSceneNode(nodeName);
            position = pLookNode->_getDerivedPosition();
        }

        pParent->lookAt(position, relativeTo, localDirection);
    }
    catch (Exception & /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a look target!");
    }
}

void DotSceneLoader::processTrackTarget(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Process attributes
    String nodeName = getAttrib(XMLNode, "nodeName");

    rapidxml::xml_node<> *pElement;

    // Process localDirection (?)
    Vector3 localDirection = Vector3::NEGATIVE_UNIT_Z;
    pElement = XMLNode->first_node("localDirection");
    if (pElement)
        localDirection = parseVector3(pElement);

    // Process offset (?)
    Vector3 offset = Vector3::ZERO;
    if (auto pElement = XMLNode->first_node("offset"))
        offset = parseVector3(pElement);

    // Setup the track target
    try
    {
        SceneNode *pTrackNode = mSceneMgr->getSceneNode(nodeName);
        pParent->setAutoTracking(true, pTrackNode, localDirection, offset);
    }
    catch (Exception & /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error processing a track target!");
    }
}

void DotSceneLoader::processEntity(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String id = getAttrib(XMLNode, "id");
    String meshFile = getAttrib(XMLNode, "meshFile");
    String material = getAttrib(XMLNode, "material");
    bool isStatic = getAttribBool(XMLNode, "static", false);
    bool castShadows = getAttribBool(XMLNode, "castShadows", true);

    // TEMP: Maintain a list of static and dynamic objects
    if (isStatic)
        staticObjects.push_back(name);
    else
        dynamicObjects.push_back(name);

    // Create the entity
    Entity *pEntity = 0;
    try
    {
        MeshManager::getSingleton().load(meshFile, m_sGroupName);
        pEntity = mSceneMgr->createEntity(name, meshFile);
        pEntity->setCastShadows(castShadows);
        pParent->attachObject(pEntity);

        if (!material.empty())
            pEntity->setMaterialName(material);
    }
    catch (Exception & /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error loading an entity!");
    }

    // Process userDataReference (?)
    if (auto pElement = XMLNode->first_node("userData"))
        processUserData(pElement, pEntity->getUserObjectBindings());
}

void DotSceneLoader::processParticleSystem(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    // Process attributes
    String name = getAttrib(XMLNode, "name");
    String id = getAttrib(XMLNode, "id");
    String templateName = getAttrib(XMLNode, "template");

    if (templateName.empty())
        templateName = getAttrib(XMLNode, "file"); // compatibility with old scenes

    // Create the particle system
    try
    {
        ParticleSystem *pParticles = mSceneMgr->createParticleSystem(name, templateName);
        pParent->attachObject(pParticles);
    }
    catch (Exception & /*e*/)
    {
        LogManager::getSingleton().logMessage("[DotSceneLoader] Error creating a particle system!");
    }
}

void DotSceneLoader::processBillboardSet(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    //! @todo Implement this
}

void DotSceneLoader::processPlane(rapidxml::xml_node<> *XMLNode, SceneNode *pParent)
{
    String name = getAttrib(XMLNode, "name");
    Real distance = getAttribReal(XMLNode, "distance");
    Real width = getAttribReal(XMLNode, "width");
    Real height = getAttribReal(XMLNode, "height");
    int xSegments = StringConverter::parseInt(getAttrib(XMLNode, "xSegments"));
    int ySegments = StringConverter::parseInt(getAttrib(XMLNode, "ySegments"));
    int numTexCoordSets = StringConverter::parseInt(getAttrib(XMLNode, "numTexCoordSets"));
    Real uTile = getAttribReal(XMLNode, "uTile");
    Real vTile = getAttribReal(XMLNode, "vTile");
    String material = getAttrib(XMLNode, "material");
    bool hasNormals = getAttribBool(XMLNode, "hasNormals");
    Vector3 normal = parseVector3(XMLNode->first_node("normal"));
    Vector3 up = parseVector3(XMLNode->first_node("upVector"));

    Plane plane(normal, distance);
    MeshPtr res = MeshManager::getSingletonPtr()->createPlane(
        name + "mesh", "General", plane, width, height, xSegments, ySegments, hasNormals,
        numTexCoordSets, uTile, vTile, up);
    Entity *ent = mSceneMgr->createEntity(name, name + "mesh");

    ent->setMaterialName(material);

    pParent->attachObject(ent);
}

void DotSceneLoader::processFog(rapidxml::xml_node<> *XMLNode)
{
    // Process attributes
    Real expDensity = getAttribReal(XMLNode, "density", 0.001);
    Real linearStart = getAttribReal(XMLNode, "start", 0.0);
    Real linearEnd = getAttribReal(XMLNode, "end", 1.0);

    FogMode mode = FOG_NONE;
    String sMode = getAttrib(XMLNode, "mode");
    if (sMode == "none")
        mode = FOG_NONE;
    else if (sMode == "exp")
        mode = FOG_EXP;
    else if (sMode == "exp2")
        mode = FOG_EXP2;
    else if (sMode == "linear")
        mode = FOG_LINEAR;
    else
        mode = (FogMode)StringConverter::parseInt(sMode);

    // Process colourDiffuse (?)
    ColourValue colourDiffuse = ColourValue::White;
    
    if (auto pElement = XMLNode->first_node("colour"))
        colourDiffuse = parseColour(pElement);

    // Setup the fog
    mSceneMgr->setFog(mode, colourDiffuse, expDensity, linearStart, linearEnd);
}

void DotSceneLoader::processSkyBox(rapidxml::xml_node<> *XMLNode)
{
    // Process attributes
    String material = getAttrib(XMLNode, "material", "BaseWhite");
    Real distance = getAttribReal(XMLNode, "distance", 5000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if (!active)
        return;

    // Process rotation (?)
    Quaternion rotation = Quaternion::IDENTITY;
    
    if (auto pElement = XMLNode->first_node("rotation"))
        rotation = parseQuaternion(pElement);

    // Setup the sky box
    mSceneMgr->setSkyBox(true, material, distance, drawFirst, rotation, m_sGroupName);
}

void DotSceneLoader::processSkyDome(rapidxml::xml_node<> *XMLNode)
{
    // Process attributes
    String material = XMLNode->first_attribute("material")->value();
    Real curvature = getAttribReal(XMLNode, "curvature", 10);
    Real tiling = getAttribReal(XMLNode, "tiling", 8);
    Real distance = getAttribReal(XMLNode, "distance", 4000);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);
    bool active = getAttribBool(XMLNode, "active", false);
    if (!active)
        return;

    // Process rotation (?)
    Quaternion rotation = Quaternion::IDENTITY;
    if (auto pElement = XMLNode->first_node("rotation"))
        rotation = parseQuaternion(pElement);

    // Setup the sky dome
    mSceneMgr->setSkyDome(true, material, curvature, tiling, distance, drawFirst, rotation, 16, 16, -1, m_sGroupName);
}

void DotSceneLoader::processSkyPlane(rapidxml::xml_node<> *XMLNode)
{
    // Process attributes
    String material = getAttrib(XMLNode, "material");
    Real planeX = getAttribReal(XMLNode, "planeX", 0);
    Real planeY = getAttribReal(XMLNode, "planeY", -1);
    Real planeZ = getAttribReal(XMLNode, "planeX", 0);
    Real planeD = getAttribReal(XMLNode, "planeD", 5000);
    Real scale = getAttribReal(XMLNode, "scale", 1000);
    Real bow = getAttribReal(XMLNode, "bow", 0);
    Real tiling = getAttribReal(XMLNode, "tiling", 10);
    bool drawFirst = getAttribBool(XMLNode, "drawFirst", true);

    // Setup the sky plane
    Plane plane;
    plane.normal = Vector3(planeX, planeY, planeZ);
    plane.d = planeD;
    mSceneMgr->setSkyPlane(true, plane, material, scale, tiling, drawFirst, bow, 1, 1, m_sGroupName);
}

void DotSceneLoader::processClipping(rapidxml::xml_node<> *XMLNode)
{
    //! @todo Implement this

    // Process attributes
    // Real fNear = getAttribReal(XMLNode, "near", 0);
    // Real fFar = getAttribReal(XMLNode, "far", 1);
}

void DotSceneLoader::processLightRange(rapidxml::xml_node<> *XMLNode, Light *pLight)
{
    // Process attributes
    Real inner = getAttribReal(XMLNode, "inner");
    Real outer = getAttribReal(XMLNode, "outer");
    Real falloff = getAttribReal(XMLNode, "falloff", 1.0);

    // Setup the light range
    pLight->setSpotlightRange(Angle(inner), Angle(outer), falloff);
}

void DotSceneLoader::processLightAttenuation(rapidxml::xml_node<> *XMLNode, Light *pLight)
{
    // Process attributes
    Real range = getAttribReal(XMLNode, "range");
    Real constant = getAttribReal(XMLNode, "constant");
    Real linear = getAttribReal(XMLNode, "linear");
    Real quadratic = getAttribReal(XMLNode, "quadratic");

    // Setup the light attenuation
    pLight->setAttenuation(range, constant, linear, quadratic);
}

void DotSceneLoader::processUserData(rapidxml::xml_node<> *XMLNode, UserObjectBindings &userData)
{
    // Process node (*)
    rapidxml::xml_node<> *pElement = XMLNode->first_node("property");
    while (pElement)
    {
        String name = getAttrib(pElement, "name");
        String type = getAttrib(pElement, "type");
        String data = getAttrib(pElement, "data");

        Any value;
        if (type == "bool")
            value = StringConverter::parseBool(data);
        else if (type == "float")
            value = StringConverter::parseReal(data);
        else if (type == "int")
            value = StringConverter::parseInt(data);
        else
            value = data;

        userData.setUserAny(name, value);
        pElement = pElement->next_sibling("property");
    }
}
