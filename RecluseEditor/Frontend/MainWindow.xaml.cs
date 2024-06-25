using System;
using System.Windows;
using System.Windows.Interop;
using RecluseEditor;
using System.Windows.Media;
using System.Windows.Controls;
using Recluse.CSharp;
using Microsoft.CSharp;
using System.Collections;

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public Recluse.CSharp.IGraphicsDevice Device { get; set; }
        public Recluse.CSharp.IGraphicsContext Context { get; set; }
        public Recluse.CSharp.IGraphicsContext EditContext {  get; set; }

        public Recluse.CSharp.GraphicsHost GameGraphicsHost { get; set; }
        public Recluse.CSharp.GraphicsHost EditGraphicsHost { get; set; }

        public float t = 0.0f;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            // The Application name 
            TitleName = RecluseEditor.MainApp.ApplicationName;
        }

        // Updates Rendering.
        void UpdateGameRender(object obj, EventArgs e)
        {
            t += 1.0f * 0.05f;
            // Cats are good.
            Context.Begin();
            IResource SwapchainResource = Context.GetCurrentFrame();
            UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
            Context.BindRenderTargets(arr, (UIntPtr)0);

            Context.ClearRenderTarget(0, 
                new float[4] { (float)Math.Sin((float)t), 1, 0, 0 }, 
                new Recluse.CSharp.Rect(0, 0, (float)GameGraphicsHost.ActualWidth, (float)GameGraphicsHost.ActualHeight));

            Context.Transition(SwapchainResource, ResourceState.Present);
            Context.End();
            Context.Present();
        }

        void UpdateEditRender(object obj, EventArgs e)
        {
            t += 1.0f * 0.05f;
            // Cats are good.
            EditContext.Begin();
            IResource SwapchainResource = EditContext.GetCurrentFrame();
            UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
            EditContext.BindRenderTargets(arr, (UIntPtr)0);

            EditContext.ClearRenderTarget(0,
                new float[4] { 1, (float)Math.Sin((float)t), 0, 0 },
                new Recluse.CSharp.Rect(0, 0, (float)EditGraphicsHost.ActualWidth, (float)EditGraphicsHost.ActualHeight));

            EditContext.Transition(SwapchainResource, ResourceState.Present);
            EditContext.End();
            EditContext.Present();
        }


        void InitializeGameRenderer(object obj, EventArgs args)
        {
            if (GameGraphicsHost.IsLoaded)
            {
                Context = new Recluse.CSharp.IGraphicsContext(Device, GameGraphicsHost.Handle, 
                    ResourceFormat.R8G8B8A8_Unorm, 
                    (int)GameGraphicsHost.ActualWidth, 
                    (int)GameGraphicsHost.ActualHeight, 
                    2, 
                    FrameBuffering.Double);
                Context.SetContextFrame(2);
                CompositionTarget.Rendering -= InitializeGameRenderer;
                CompositionTarget.Rendering += UpdateGameRender;
            }
        }

        void InitializeEditRenderer(object obj, EventArgs args)
        {
            if (EditGraphicsHost.IsLoaded)
            {
                EditContext = new Recluse.CSharp.IGraphicsContext(Device, EditGraphicsHost.Handle, 
                    ResourceFormat.R8G8B8A8_Unorm, 
                    (int)EditGraphicsHost.ActualWidth, 
                    (int)EditGraphicsHost.ActualHeight, 
                    2, 
                    FrameBuffering.Double);
                EditContext.SetContextFrame(2);
                CompositionTarget.Rendering -= InitializeEditRenderer;
                CompositionTarget.Rendering += UpdateEditRender;
            }
        }

        public void OnLoaded(object sender, RoutedEventArgs e)
        {

            Device = new Recluse.CSharp.IGraphicsDevice(Recluse.CSharp.GraphicsApi.Direct3D12, "", "");
            //WindowInteropHelper helper = new WindowInteropHelper(this);
            GameGraphicsHost = new Recluse.CSharp.GraphicsHost("GameView");
            EditGraphicsHost = new Recluse.CSharp.GraphicsHost("EditView");
            if (GameViewBorder.IsInitialized)
            {
                GameViewBorder.Child = GameGraphicsHost;
                CompositionTarget.Rendering += InitializeGameRenderer;
            }
            if (EditViewBorder.IsInitialized)
            {
                EditViewBorder.Child = EditGraphicsHost;
                CompositionTarget.Rendering += InitializeEditRenderer;
            }

        }

        /// <summary>
        /// Close the window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public void OnExit(object sender, EventArgs e)
        {
            Close();
        }

        public void Settings_OnClick(object sender, RoutedEventArgs e)
        {
            RecluseEditor.SettingsWindow SettingsWindowW = new RecluseEditor.SettingsWindow();
            SettingsWindowW.Owner = this;
            SettingsWindowW.Show();
        }

        public void OnResizedView(object sender, EventArgs e)
        {
            CompositionTarget.Rendering -= UpdateGameRender;
            CompositionTarget.Rendering -= UpdateEditRender;
            CompositionTarget.Rendering += ResizeGameRender;
            CompositionTarget.Rendering += ResizeEditRender;
        }

        public void ResizeGameRender(object sender, EventArgs e)
        {
            if (Context != null)
            { 
                Context.ResizeSwapchain((int)GameGraphicsHost.ActualWidth, (int)GameGraphicsHost.ActualHeight);
                CompositionTarget.Rendering -= ResizeGameRender;
                CompositionTarget.Rendering += UpdateGameRender;
            }
        }

        public void ResizeEditRender(object sender, EventArgs e)
        {
            if (EditContext != null)
            { 
                EditContext.ResizeSwapchain((int)EditGraphicsHost.ActualWidth, (int)EditGraphicsHost.ActualHeight);
                CompositionTarget.Rendering -= ResizeEditRender;
                CompositionTarget.Rendering += UpdateEditRender;
            }
        }
    }
}