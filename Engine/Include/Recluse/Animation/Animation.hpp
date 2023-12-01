// Code for animation states.
// Special thanks to beaumanvienna for teaching the animation structure for GLTF.
// Check out his awesome work at https://github.com/beaumanvienna/vulkan
//
#pragma once

#include "Recluse/Math/Vector4.hpp"
#include "Recluse/Animation/Skeleton.hpp"
#include "Recluse/Time.hpp"
#include <vector>

namespace Recluse {
namespace Engine {

class AnimationClip 
{
public:
    enum Path
    {
        Path_Translation,
        Path_Rotation,
        Path_Scale
    };

    enum Interpolation
    {
        Interpolation_Linear,
        Interpolation_Step,
        Interpolation_CubicSpline
    };

    struct Channel
    {
        Path path;
        I32 samplerIndex;
        I32 node;
    };

    struct Sampler
    {
        std::vector<F32> timestamps;
        std::vector<Math::Float4> trsOut;
        Interpolation interpolation;
    };

    AnimationClip(const std::string& name);

    void start();
    void stop();
    Bool isRunning();
    Bool isRepeating() const { return m_repeat; }
    void setRepeat(Bool enable) { m_repeat = enable; }

    // Update the clip animation using the skeleton.
    void update(F32 timestep, const Skeleton& skeleton);

    F32 duration();
    F32 currentTime();
private:
    std::string             m_name;
    Bool                    m_repeat;
    
    std::vector<Sampler>    m_samplers;
    std::vector<Channel>    m_channels;
};
} // Engine
} // Recluse