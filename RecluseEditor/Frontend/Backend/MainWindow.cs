using System;
using System.Windows;
using System.Windows.Interop;
using RecluseEditor;
using System.Windows.Media;
using System.Windows.Controls;
using Recluse.CSharp;
using Microsoft.CSharp;
using System.Collections;
using System.Diagnostics;
using System.Threading;
using System.Windows.Input;

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public Recluse.CSharp.GraphicsHost GameGraphicsHost { get; set; }
        public Recluse.CSharp.GraphicsHost EditGraphicsHost { get; set; }

        public long PrevRenderingTick = 0;

        public float t = 0.0f;
        
        public bool ShouldMessage = false;
        public System.Collections.Concurrent.ConcurrentQueue<string> ConsoleQueue;
        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            // The Application name 
            TitleName = RecluseEditor.MainApp.ApplicationName;

            ConsoleQueue = new System.Collections.Concurrent.ConcurrentQueue<string>();
        }

        // Updates Rendering.
        void UpdateGameRender(object obj, EventArgs e)
        {
            if (!GameGraphicsHost.IsVisible)
            {
                return;
            }

            Renderer.Render(Renderer.RenderView.GameMode, obj, e, (IGraphicsContext Context, IResource SwapchainResource, IResource DepthBuffer, object Sender, EventArgs E) => 
                {
                    t += 1.0f * 0.008f;
                    Context.Transition(SwapchainResource, ResourceState.RenderTarget);
                    UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
                    Context.BindRenderTargets(arr, (UIntPtr)0);

                    Context.ClearRenderTarget(0,
                        new float[4] { (float)Math.Abs(Math.Sin((float)t)), 1, 0, 0 },
                        new Recluse.CSharp.Rect(0, 0, (float)GameGraphicsHost.ActualWidth, (float)GameGraphicsHost.ActualHeight));
                });
            
        }
        
        void UpdateEditRender(object obj, EventArgs e)
        {
            if (!EditGraphicsHost.IsVisible)
            {
                return;
            }

            Renderer.Render(Renderer.RenderView.EditMode, obj, e, (IGraphicsContext Context, IResource SwapchainResource, IResource DepthBuffer, object Sender, EventArgs E) =>
                {
                    RenderingEventArgs args = (RenderingEventArgs)e;

                    long RenderTick = args.RenderingTime.Ticks;
                    long framerate = RenderTick - PrevRenderingTick;
                    PrevRenderingTick = RenderTick;
                    float framesPerMillisecond = (float)framerate / 10000.0f; // Windows measurement unit of 1 cpu tick.
                    float FrameDelta = (framesPerMillisecond / 1000.0f);
                    float FramesPerSecond = 1.0f / FrameDelta;
                    t += 1.0f * FrameDelta;
                    Context.Transition(SwapchainResource, ResourceState.RenderTarget);
                    UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
                    UIntPtr depth = DepthBuffer.AsView(ResourceViewType.DepthStencil, ResourceViewDimension.Dim2d, ResourceFormat.D32_Float, 0, 0, 1, 1);
                    Context.BindRenderTargets(arr, depth);

                    Context.ClearRenderTarget(0,
                        new float[4] { 1, (float)Math.Abs(Math.Sin((float)t)), 0, 0 },
                        new Recluse.CSharp.Rect(0, 0, (float)EditGraphicsHost.ActualWidth, (float)EditGraphicsHost.ActualHeight));
                    Context.ClearDepthStencil(ClearFlags.Depth, 0.0f, 0, 
                        new Recluse.CSharp.Rect(0, 0, (float)EditGraphicsHost.ActualWidth, (float)EditGraphicsHost.ActualHeight));
                    WriteToEditorOutput("Rendering Time: " + FramesPerSecond + " Fps");
                });
        }


        void InitializeGameRenderer(object obj, EventArgs args)
        {
            if (GameGraphicsHost.IsLoaded)
            {
                Renderer.InitializeSwapchain(Renderer.RenderView.GameMode, GameGraphicsHost);
                CompositionTarget.Rendering -= InitializeGameRenderer;
            }
        }

        void InitializeEditRenderer(object obj, EventArgs args)
        {
            if (EditGraphicsHost.IsLoaded)
            {
                Renderer.InitializeSwapchain(Renderer.RenderView.EditMode, EditGraphicsHost);
                CompositionTarget.Rendering -= InitializeEditRenderer;
            }
        }

        public void OnLoaded(object sender, RoutedEventArgs e)
        {
            Renderer.Initialize(GraphicsApi.Vulkan, "RecluseEditor", "Recluse");
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


        public void OnStateChanged(object sender, EventArgs args)
        {
            Window window = sender as Window;
            if (window.WindowState == WindowState.Minimized)
            {
                CompositionTarget.Rendering -= UpdateEditRender;
                CompositionTarget.Rendering -= UpdateGameRender;
            }
            else if (window.WindowState == WindowState.Normal || window.WindowState == WindowState.Maximized)
            {
                CompositionTarget.Rendering += ResizeEditRender;
                CompositionTarget.Rendering += ResizeGameRender;
            }
        }

        public void OnWindowClosed(object sender, EventArgs args)
        {
            Renderer.Shutdown();
        }

        /// <summary>
        /// Close the window.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public void OnExit(object sender, EventArgs e)
        {
            Renderer.Shutdown();
            Close();
        }

        public void Settings_OnClick(object sender, RoutedEventArgs e)
        {
            RecluseEditor.SettingsWindow SettingsWindowW = new RecluseEditor.SettingsWindow();
            SettingsWindowW.Owner = this;
            SettingsWindowW.Show();
            WriteToEditorOutput("I am crap");
        }

        public void ResizeGameRender(object sender, EventArgs e)
        {
            if (Renderer.IsSwapchainActive(Renderer.RenderView.GameMode))
            { 
                Renderer.ResizeSwapchain(Renderer.RenderView.GameMode, GameGraphicsHost);
                CompositionTarget.Rendering -= ResizeGameRender;
                CompositionTarget.Rendering += UpdateGameRender;
            }
        }

        public void ResizeEditRender(object sender, EventArgs e)
        {
            if (Renderer.IsSwapchainActive(Renderer.RenderView.EditMode))
            { 
                Renderer.ResizeSwapchain(Renderer.RenderView.EditMode, EditGraphicsHost);
                CompositionTarget.Rendering -= ResizeEditRender;
                CompositionTarget.Rendering += UpdateEditRender;
            }
        }

        public void OnEditResized(object sender, EventArgs e)
        {
            CompositionTarget.Rendering -= UpdateEditRender;
            CompositionTarget.Rendering += ResizeEditRender;
        }

        public void OnGameResized(object sender, EventArgs e)
        {
            CompositionTarget.Rendering -= UpdateGameRender;
            CompositionTarget.Rendering += ResizeGameRender;
        }

        public delegate void UpdateMessageDelegate(string message);

        /// <summary>
        /// Updater.
        /// </summary>
        /// <param name="text"></param>
        public void UpdateEditorOutput(string text)
        {
            if (ConsoleQueue.Count >= 128)
                ConsoleQueue.TryDequeue(out var output);
            ConsoleQueue.Enqueue(text);
            string MessageOutput = "";
            foreach (string Text in ConsoleQueue)
            {
                MessageOutput += Text;
            }
            ConsoleTextOutput.Text = MessageOutput;
            ConsoleOutputScrollViewer.ScrollToEnd();
        }

        public void WriteToEditorOutput(string text)
        {
            text += "\n";
            if (ConsoleQueue.Count >= 128)
            {
                string stuff;
                ConsoleQueue.TryDequeue(out stuff);
            }
            ConsoleTextOutput.Dispatcher.Invoke(new UpdateMessageDelegate(this.UpdateEditorOutput), new object[] { text });
        }
    }
}