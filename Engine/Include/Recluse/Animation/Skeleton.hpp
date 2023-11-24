//
#pragma once
#include "Recluse/Types.hpp"
#include "Recluse/RGUID.hpp"
#include "Recluse/Animation/Joint.hpp"

#include <map>

namespace Recluse {
namespace Engine {


struct AnimSkinningShaderData
{
    std::vector<Math::Matrix44> finalJointMatrices;
};

// 
class Skeleton : public Serializable
{
public:
    const static I32 kNoParent = -1;
    const static I32 kRootJoint = 0;

    Skeleton(const std::string& name);

    ResultCode traverse();
    ResultCode update();
    ResultCode updateJoint(U32 joint);

    Joint& operator[] (U32 jointIndex) { return m_joints[jointIndex]; }
    const Joint& operator[] (U32 jointIndex) const { return m_joints[jointIndex]; }

    Bool        isAnimated() const { return m_isAnimated; }
    std::string operator()() const { return m_name; }
    std::string getName() const { return m_name; } 
    
private:
    Bool                    m_isAnimated;
    std::string             m_name;
    std::map<I32, I32>      m_globalNodeToJointIndex;
    std::vector<Joint>      m_joints;
    RGUID                   m_guid;
    AnimSkinningShaderData  m_shaderData;
};
} // Engine
} // Recluse