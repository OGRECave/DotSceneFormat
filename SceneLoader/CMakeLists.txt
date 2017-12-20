cmake_minimum_required (VERSION 2.8)
project(DotSceneLoader)

# specify which version you need
find_package(OGRE 1.10 REQUIRED)

if(MSVC)
    add_definitions(/wd4390 /wd4305)
else()
    add_definitions(-Wall)
endif()

# the search paths
include_directories(${OGRE_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include/)
link_directories(${OGRE_LIBRARY_DIRS})

add_executable(DotSceneLoader src/main.cpp src/DotSceneLoader.cpp)
target_link_libraries(DotSceneLoader ${OGRE_LIBRARIES})