//
#pragma once

#include "WxContext.hpp"

#include "Recluse/Graphics/GraphicsInstance.hpp"
#include "Recluse/Graphics/GraphicsAdapter.hpp"
#include "Recluse/Graphics/GraphicsDevice.hpp"

#include "Recluse/Math/MathCommons.hpp"
#include "Recluse/Math/Vector4.hpp"

namespace Recluse {


class RenderPanel : public wxPanel
{
public:
    RenderPanel(wxWindow* window)
        : wxPanel(window)
        , pContext(nullptr) { }
    virtual ~RenderPanel() { destroy(); }

    void initialize()
    {
        ResultCode result = RecluseResult_Ok;
        pInstance = GraphicsInstance::createInstance(GraphicsApi_Vulkan);
        ApplicationInfo appInfo = { };
        appInfo.appName = "RecluseEditor";
        appInfo.engineName = "RecluseEngine";
        result = pInstance->initialize(appInfo, LayerFeatureFlag_DebugValidation);

        if (result != RecluseResult_Ok)
        {
            
        }

        pAdapter = pInstance->getGraphicsAdapters()[0];
        DeviceCreateInfo info = { };
        result = pAdapter->createDevice(info, &pDevice);
        if (result == RecluseResult_Ok)
        {
            int width = 0;
            int height = 0;
            GetSize(&width, &height);
            SwapchainCreateDescription swapchainDescription = { };
            swapchainDescription.buffering = FrameBuffering_Double;
            swapchainDescription.desiredFrames = 3;
            swapchainDescription.format = ResourceFormat_R8G8B8A8_Unorm;
            swapchainDescription.renderWidth = width;
            swapchainDescription.renderHeight = height;
            swapchainDescription.preferHDR = false;
            pSwapchain = pDevice->createSwapchain(swapchainDescription, GetHandle());
            pContext = pDevice->createContext();
            pContext->setFrames(3);
        }
    }

    void onDraw()
    {
        static F32 t = 0.f;
        t += 1 * 0.01f;
        Math::Float4 rgba(1.0f, 0.5f, 0.0f, 1.0f);
        Math::Float4 rgba2(0.1f, 0.1f, 0.1f, 1.0f);
        pSwapchain->prepare(pContext);
        GraphicsResource* sf = pSwapchain->getFrame(pSwapchain->getCurrentFrameIndex());
        ResourceViewDescription desc = { };
        desc.type = ResourceViewType_RenderTarget;
        desc.format = pSwapchain->getDesc().format;
        desc.baseArrayLayer = 0;
        desc.baseMipLevel = 0;
        desc.dimension = ResourceViewDimension_2d;
        desc.layerCount = 1;
        desc.mipLevelCount = 1;
        ResourceViewId id = sf->asView(desc);

        Rect rect = { 0, 0, pSwapchain->getDesc().renderWidth, pSwapchain->getDesc().renderHeight };
        Math::Float4 clearColor = Math::lerp(rgba, rgba2, fabsf(sinf(t)));
        pContext->transition(sf, ResourceState_RenderTarget);
        pContext->bindRenderTargets(1, &id);
        pContext->clearRenderTarget(0, &clearColor.x, rect);
        pContext->transition(sf, ResourceState_Present);
        pContext->end();
        ResultCode result = pSwapchain->present(pContext);
        if (result == RecluseResult_NeedsUpdate)
        {
            pContext->wait();
            pSwapchain->rebuild(pSwapchain->getDesc());
        }
    }

    void destroy()
    {
        if (pContext)
        {
            pContext->wait();
            pDevice->releaseContext(pContext);
            pDevice->destroySwapchain(pSwapchain);
            pAdapter->destroyDevice(pDevice);
            GraphicsInstance::destroyInstance(pInstance);
        }
    }

private:
    GraphicsInstance* pInstance;
    GraphicsAdapter* pAdapter;
    GraphicsDevice* pDevice;
    GraphicsSwapchain* pSwapchain;
    GraphicsContext* pContext;
};
} // Recluse