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

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public Recluse.CSharp.IGraphicsDevice Device { get; set; }
        public Recluse.CSharp.IGraphicsContext Context { get; set; }

        public Recluse.CSharp.GraphicsHost GameGraphicsHost { get; set; }
        public Recluse.CSharp.GraphicsHost EditGraphicsHost { get; set; }
        public Recluse.CSharp.ISwapchain GameSwapchain { get; set; }

        public long PrevRenderingTick = 0;

        /// <summary>
        /// Swapchain.
        /// </summary>
        public Recluse.CSharp.ISwapchain EditSwapchain { get; set; }

        public float t = 0.0f;
        
        public bool ShouldMessage = false;
        public System.Collections.Concurrent.ConcurrentQueue<string> ConsoleQueue;

        Stopwatch sw;
        uint frameCounter = 0;
        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            // The Application name 
            TitleName = RecluseEditor.MainApp.ApplicationName;

            ConsoleQueue = new System.Collections.Concurrent.ConcurrentQueue<string>();

            sw = new Stopwatch();
            sw.Start();
        }

        // Updates Rendering.
        void UpdateGameRender(object obj, EventArgs e)
        {
            if (!GameGraphicsHost.IsVisible)
            {
                return;
            }
            t += 1.0f * 0.05f;
            // Cats are good.
            GameSwapchain.Prepare(Context);
            IResource SwapchainResource = GameSwapchain.GetCurrentFrame();
            UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
            Context.BindRenderTargets(arr, (UIntPtr)0);

            Context.ClearRenderTarget(0, 
                new float[4] { (float)Math.Sin((float)t), 1, 0, 0 }, 
                new Recluse.CSharp.Rect(0, 0, (float)GameGraphicsHost.ActualWidth, (float)GameGraphicsHost.ActualHeight));
            Context.Transition(SwapchainResource, ResourceState.Present);
            Context.End();
            GameSwapchain.Present(Context);
        }
        
        void UpdateEditRender(object obj, EventArgs e)
        {
            if (!EditGraphicsHost.IsVisible)
            {
                return;
            }
            RenderingEventArgs args = (RenderingEventArgs)e;
            
            long RenderTick = args.RenderingTime.Ticks;
            long framerate = RenderTick - PrevRenderingTick;
            PrevRenderingTick = RenderTick;
            float framesPerMillisecond = (float)framerate / 10000.0f; // Windows measurement unit of 1 cpu tick.
            float FrameDelta = (framesPerMillisecond / 1000.0f);
            float FramesPerSecond = 1.0f / FrameDelta;
            t += 1.0f * FrameDelta;

            EditSwapchain.Prepare(Context);
            IResource SwapchainResource = EditSwapchain.GetCurrentFrame();
            UIntPtr[] arr = new UIntPtr[1] { SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1) };
            Context.BindRenderTargets(arr, (UIntPtr)0);

            Context.ClearRenderTarget(0,
                new float[4] { 1, (float)Math.Abs(Math.Sin((float)t)), 0, 0 },
                new Recluse.CSharp.Rect(0, 0, (float)EditGraphicsHost.ActualWidth, (float)EditGraphicsHost.ActualHeight));

            Context.Transition(SwapchainResource, ResourceState.Present);
            Context.End();
            EditSwapchain.Present(Context);
            WriteToEditorOutput("Rendering Time: " + FramesPerSecond + " Fps");
        }


        void InitializeGameRenderer(object obj, EventArgs args)
        {
            if (GameGraphicsHost.IsLoaded)
            {
                GameSwapchain = new ISwapchain(Device, GameGraphicsHost.Handle, ResourceFormat.R8G8B8A8_Unorm, (int)GameGraphicsHost.ActualWidth, (int)GameGraphicsHost.ActualHeight, 3, FrameBuffering.Double);
                CompositionTarget.Rendering -= InitializeGameRenderer;
            }
        }

        void InitializeEditRenderer(object obj, EventArgs args)
        {
            if (EditGraphicsHost.IsLoaded)
            {
                EditSwapchain = new ISwapchain(Device, EditGraphicsHost.Handle, ResourceFormat.R8G8B8A8_Unorm, (int)EditGraphicsHost.ActualWidth, (int)EditGraphicsHost.ActualHeight, 3, FrameBuffering.Double);
                CompositionTarget.Rendering -= InitializeEditRenderer;
            }
        }

        public void OnLoaded(object sender, RoutedEventArgs e)
        {

            Device = new Recluse.CSharp.IGraphicsDevice(Recluse.CSharp.GraphicsApi.Direct3D12, "", "");
            Context = new Recluse.CSharp.IGraphicsContext(Device);      
            Context.SetContextFrame(2);
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
            System.Windows.Media.Animation.Timeline.DesiredFrameRateProperty.OverrideMetadata(typeof(System.Windows.Media.Animation.Timeline), 
                new FrameworkPropertyMetadata { DefaultValue = 30 });
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
            WriteToEditorOutput("I am crap");
        }

        public void ResizeGameRender(object sender, EventArgs e)
        {
            if (GameSwapchain != null)
            { 
                Context.Wait();
                GameSwapchain.ResizeSwapchain((int)GameGraphicsHost.ActualWidth, (int)GameGraphicsHost.ActualHeight);
                CompositionTarget.Rendering -= ResizeGameRender;
                CompositionTarget.Rendering += UpdateGameRender;
            }
        }

        public void ResizeEditRender(object sender, EventArgs e)
        {
            if (EditSwapchain != null)
            { 
                Context.Wait();
                EditSwapchain.ResizeSwapchain((int)EditGraphicsHost.ActualWidth, (int)EditGraphicsHost.ActualHeight);
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
        }

        public void WriteToEditorOutput(string text)
        {
            text += "\n";
            if (ConsoleQueue.Count >= 128)
            {
                string stuff;
                ConsoleQueue.TryDequeue(out stuff);
            }
            ConsoleQueue.Enqueue(text);
            ConsoleTextOutput.Dispatcher.Invoke(new UpdateMessageDelegate(this.UpdateEditorOutput), new object[] { text });
        }
    }
}