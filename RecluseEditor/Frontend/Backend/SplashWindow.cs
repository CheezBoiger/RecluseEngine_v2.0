using System;
using System.Windows;

namespace RecluseEditor
{
    public partial class SplashWindow : Window
    {
        public SplashWindow()
        {
            InitializeComponent();
        }

        public void LoadMainWindow()
        {
            RecluseEditor.MainWindow Main = new MainWindow();
            Main.Show();
            Application.Current.MainWindow = Main;
        }

        public void OnClick(object sender, RoutedEventArgs e) 
        {
            LoadMainWindow();
            Close();
        }
    }
}