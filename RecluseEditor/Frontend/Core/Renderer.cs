//
using System;
using System.Collections;

using Recluse;
using Recluse.CSharp;

namespace RecluseEditor
{
    /// <summary>
    /// Renderer provides live game view and edit view modes, to which must run in realtime.
    /// </summary>
    public static class Renderer
    {
        private static IGraphicsDevice Device;
        private static IGraphicsContext Context;
        private static ISwapchain EditViewSwapchain = null;
        private static ISwapchain GameViewSwapchain = null;
        private static IResource DepthBuffer = null;

        public delegate void MainRenderDelegate(IGraphicsContext Context, IResource SwapchainResource, IResource DepthBuffer, object sender, EventArgs e);

        public enum RenderView
        {
            GameMode,
            EditMode
        }

        public static void Initialize(GraphicsApi Api, string AppName, string EngineName)
        {
            Device = new IGraphicsDevice(Api, AppName, EngineName, false);
            Context = new IGraphicsContext(Device);
            Database.Initialize(Device);
            Context.SetContextFrame(3);
        }

        /// <summary>
        /// Initialize the swapchain with the given host.
        /// </summary>
        /// <param name="View"></param>
        /// <param name="Host"></param>
        public static void InitializeSwapchain(RenderView View, GraphicsHost Host)
        {
            switch (View)
            {
                case RenderView.EditMode:
                    EditViewSwapchain = new ISwapchain(Device, Host.Handle, ResourceFormat.R8G8B8A8_Unorm, 
                        (int)Host.ActualWidth, 
                        (int)Host.ActualHeight, 
                        3, FrameBuffering.Triple);
                    break;
                case RenderView.GameMode:
                default:
                    GameViewSwapchain = new ISwapchain(Device, Host.Handle, ResourceFormat.R8G8B8A8_Unorm, 
                        (int)Host.ActualWidth, 
                        (int)Host.ActualHeight, 
                        3, FrameBuffering.Triple);
                    break;
            }

            CreateDepthBuffer((uint)Host.ActualWidth, (uint)Host.ActualHeight);
        }

        public static void ResizeSwapchain(RenderView View, GraphicsHost Host)
        {
            ISwapchain Swapchain;
            switch (View)
            {
                case RenderView.EditMode:
                    Swapchain = EditViewSwapchain;
                    break;
                case RenderView.GameMode:
                default:
                    Swapchain = GameViewSwapchain;
                    break;
            }
            Context.Wait();
            Swapchain.ResizeSwapchain((int)Host.ActualWidth, (int)Host.ActualHeight);

            // Must recreate the depthbuffer.
            CreateDepthBuffer((uint)Host.ActualWidth, (uint)Host.ActualHeight);
        }

        public static void ShutdownSwapchain(RenderView View)
        {

        }

        public static void Shutdown()
        {
            Database.CleanUp();
            Context.Dispose();
            Device.Dispose();
            Context = null;
            Device = null;
        }

        /// <summary>
        /// Render using the View context. 
        /// </summary>
        /// <param name="View"></param>
        public static void Render(RenderView View, object Sender, EventArgs Args, MainRenderDelegate MainRender = null)
        {
            ISwapchain Swapchain = null;
            switch (View)
            {
                case RenderView.EditMode:
                    Swapchain = EditViewSwapchain;
                    break;
                case RenderView.GameMode:
                default:
                    Swapchain = GameViewSwapchain;
                    break;
            }

            Swapchain.Prepare(Context);
            IResource SwapchainResource = Swapchain.GetCurrentFrame();
            if (MainRender != null)
            {
                MainRender(Context, SwapchainResource, DepthBuffer, Sender, Args);
            }

            #region Grid Rendering
            //Context.Transition(SwapchainResource, ResourceState.RenderTarget);
            //Context.EnableDepth(true);
            //Context.EnableDepthWrite(true);
            //Context.EnableStencil(false);
            //Context.BindShaderProgram((ulong)Database.Shaders.Grid, 0);
            //Context.SetInputVertexLayout((uint)Database.Vertex.PositionOnly);
            #endregion
            Context.Transition(SwapchainResource, ResourceState.Present);
            Context.End();
            Swapchain.Present(Context);
        }


        public static bool IsSwapchainActive(RenderView View)
        {
            ISwapchain Swapchain = null;
            switch (View)
            {
                case RenderView.EditMode:
                    Swapchain = EditViewSwapchain;
                    break;
                case RenderView.GameMode:
                default:
                    Swapchain = GameViewSwapchain;
                    break;
            }

            return (Swapchain != null);
        }


        private static void CreateDepthBuffer(uint Width, uint Height)
        {
            if (DepthBuffer != null)
            {
                DepthBuffer.ReleaseImmediatelyOnDispose();
                DepthBuffer.Dispose();
            }
            ResourceCreateInformation ResourceCreateInfo = new ResourceCreateInformation();
            ResourceCreateInfo.Name = "DepthBuffer";
            ResourceCreateInfo.Width = Width;
            ResourceCreateInfo.Height = Height;
            ResourceCreateInfo.MipLevels = 1;
            ResourceCreateInfo.DepthOrArraySize = 1;
            ResourceCreateInfo.Samples = 1;
            ResourceCreateInfo.Format = ResourceFormat.D32_Float;
            ResourceCreateInfo.Dimension = ResourceDimension.Dim2d;
            ResourceCreateInfo.Usage = ResourceUsage.DepthStencil | ResourceUsage.ShaderResource;
            ResourceCreateInfo.MemoryUsage = ResourceMemoryUsage.GpuOnly;

            DepthBuffer = new IResource(Device, ResourceCreateInfo, ResourceState.DepthStencilWrite);
        }

        public static IGraphicsDevice GetDevice() { return Device; }
        public static IGraphicsContext GetContext() { return Context; }
    }
}