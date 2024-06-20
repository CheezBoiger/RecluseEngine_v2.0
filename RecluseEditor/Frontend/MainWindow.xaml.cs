using System;
using System.Windows;
using System.Windows.Interop;
using RecluseEditor;
using System.Windows.Media;
using System.Windows.Controls;
using Recluse.CSharp;
using Microsoft.CSharp;

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public Recluse.CSharp.IGraphicsDevice Device { get; set; }
        public Recluse.CSharp.IGraphicsContext Context { get; set; }

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
            // Cats are good.
            Context.Begin();
            Context.End();
            Context.Present();
        }

        public void OnLoaded(object sender, RoutedEventArgs e)
        {

            Device = new Recluse.CSharp.IGraphicsDevice(Recluse.CSharp.GraphicsApi.Vulkan, "", "");
            WindowInteropHelper helper = new WindowInteropHelper(this);
            Context = new Recluse.CSharp.IGraphicsContext(Device, helper.Handle, (Int32)Width, (Int32)Height, FrameBuffering.Double);
            Context.SetContextFrame(2);
            CompositionTarget.Rendering += UpdateRender;
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