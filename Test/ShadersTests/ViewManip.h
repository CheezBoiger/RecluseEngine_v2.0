#ifndef VIEW_MANIP_H
#define VIEW_MANIP_H

#include "Test.h"

vec4 getNearAndFar(inout RecluseSceneView sceneView)
{
    return vec4(sceneView.near, sceneView.far, 0, 0);
}

#endif // VIEW_MANIP_H