//
using System;
using System.Collections;

using Recluse;
using Recluse.CSharp;

namespace RecluseEditor
{
    public static class Renderer
    {
        private static IGraphicsDevice Device;
        private static IGraphicsContext Context;
        private static ISwapchain EditViewSwapchain = null;
        private static ISwapchain GameViewSwapchain = null;


        public delegate void MainRenderDelegate(ref IGraphicsContext Context, ref IResource SwapchainResource, object sender, EventArgs e);

        public enum RenderView
        {
            GameMode,
            EditMode
        }

        public static void Initialize(GraphicsApi Api, string AppName, string EngineName)
        {
            Device = new IGraphicsDevice(Api, AppName, EngineName, true);
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

            IResource SwapchainResource = Swapchain.GetCurrentFrame();

            Swapchain.Prepare(Context);
            if (MainRender != null)
            {
                //MainRender(ref Context, ref SwapchainResource, Sender, Args);
            }
            Context.Transition(SwapchainResource, ResourceState.RenderTarget);
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

        public static IGraphicsDevice GetDevice() { return Device; }
        public static IGraphicsContext GetContext() { return Context; }
    }
}