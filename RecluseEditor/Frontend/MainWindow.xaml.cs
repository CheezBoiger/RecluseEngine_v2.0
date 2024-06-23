using System;
using System.Windows;
using System.Windows.Interop;
using RecluseEditor;
using System.Windows.Media;
using System.Windows.Controls;
using Recluse.CSharp;
using Microsoft.CSharp;
using System.Windows.Forms;

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public Recluse.CSharp.IGraphicsDevice Device { get; set; }
        public Recluse.CSharp.IGraphicsContext Context { get; set; }

        public Recluse.CSharp.GraphicsHost GraphicsHost { get; set; }

        public float t = 0.0f;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            // The Application name 
            TitleName = RecluseEditor.MainApp.ApplicationName;
        }

        // Updates Rendering.
        void UpdateRender(object obj, EventArgs e)
        {
            t += 1.0f * 0.05f;
            // Cats are good.
            Context.Begin();
            IResource SwapchainResource = Context.GetCurrentFrame();
            UIntPtr[] arr = new UIntPtr[1];
            arr[0] = SwapchainResource.AsView(ResourceViewType.RenderTarget, ResourceViewDimension.Dim2d, ResourceFormat.R8G8B8A8_Unorm, 0, 0, 1, 1);
            Context.BindRenderTargets(arr, (UIntPtr)0);

            Recluse.CSharp.Rect RectArea = new Recluse.CSharp.Rect();
            RectArea.X = 0;
            RectArea.Y = 0;
            RectArea.Width = (float)GraphicsHost.ActualWidth;
            RectArea.Height = (float) GraphicsHost.ActualHeight;
            Context.ClearRenderTarget(0, new float[4] { (float)Math.Sin((float)t), 1, 0, 0 }, RectArea);
            Context.Transition(SwapchainResource, ResourceState.Present);
            Context.End();
            Context.Present();
        }


        void InitializeRenderer(object obj, EventArgs args)
        {
            if (GraphicsHost.IsLoaded)
            {
                Context = new Recluse.CSharp.IGraphicsContext(Device, GraphicsHost.Handle, ResourceFormat.R8G8B8A8_Unorm, (int)GraphicsHost.ActualWidth, (int)GraphicsHost.ActualHeight, FrameBuffering.Double);
                Context.SetContextFrame(2);
                CompositionTarget.Rendering -= InitializeRenderer;
                CompositionTarget.Rendering += UpdateRender;
            }
        }

        public void OnLoaded(object sender, RoutedEventArgs e)
        {

            Device = new Recluse.CSharp.IGraphicsDevice(Recluse.CSharp.GraphicsApi.Direct3D12, "", "");
            //WindowInteropHelper helper = new WindowInteropHelper(this);
            GraphicsHost = new Recluse.CSharp.GraphicsHost();
            if (ThatBorder.IsInitialized)
            {
                ThatBorder.Child = GraphicsHost;
                CompositionTarget.Rendering += InitializeRenderer;
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
    }
}