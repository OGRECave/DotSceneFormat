cmake_minimum_required (VERSION 2.8)
project(DotSceneLoader)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# specify which version you need
find_package(OGRE 1.11 REQUIRED)

if(MSVC)
    add_definitions(/wd4390 /wd4305)
else()
    add_definitions(-Wall)
endif()

# the search paths
include_directories(${OGRE_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include/)
link_directories(${OGRE_LIBRARY_DIRS})

add_library(Plugin_DotSceneLoader SHARED src/DotSceneLoader.cpp src/OgreDotScenePlugin.cpp)
target_link_libraries(Plugin_DotSceneLoader OgreTerrain)
set_target_properties(Plugin_DotSceneLoader PROPERTIES PREFIX "")

add_executable(DotSceneLoader src/main.cpp )
target_link_libraries(DotSceneLoader Plugin_DotSceneLoader ${OGRE_LIBRARIES} )