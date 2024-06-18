using System;
using System.Windows;
using System.Windows.Interop;
using RecluseEditor;
using System.Windows.Media;
using System.Windows.Controls;

namespace RecluseEditor
{
    public partial class MainWindow : Window
    {
        public string TitleName { get; set; }

        public MainWindow()
        {
            InitializeComponent();
            DataContext = this;
            // The Application name 
            TitleName = RecluseEditor.MainApp.ApplicationName;
            CompositionTarget.Rendering += UpdateRender;
        }

        // Updates Rendering.
        void UpdateRender(object obj, EventArgs e)
        {
            // Cats are good.
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