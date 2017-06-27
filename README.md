# DotScene Overview

DotScene (aka .scene) is just a standardized XML file format.

This file format is meant to be used to set up a scene in Ogre. It is meant to be useful for any type of application/game. Editors can export to .scene format, and apps can load the format.

## What is DotScene?
DotScene file does not contain any mesh data, texture data, etc. It just contains elements that describe a scene.

A simple .scene file example:
```xml
<scene formatVersion="">
    <nodes>
        <node name="Robot" id="3">
            <position x="10.0" y="5" z="10.5" />
            <scale x="1" y="1" z="1" />
            <entity name="Robot" meshFile="robot.mesh" static="false" />
        </node>
        <node name="Omni01" id="5">
            <position x="-23" y="49" z="18" />
            <rotation qx="0" qy="0" qz="0" qw="1" />
            <scale x="1" y="1" z="1" />
            <light name="Omni01" type="point">
                <colourDiffuse r="0.4" g="0.4" b="0.5" />
                <colourSpecular r="0.5" g="0.5" b="0.5" />
            </light>
        </node>
    </nodes>
</scene>
```