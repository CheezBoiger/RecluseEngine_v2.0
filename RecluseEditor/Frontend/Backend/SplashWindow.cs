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

        public void OnLoaded(object sender, RoutedEventArgs e)
        {
            if (VersionTitle.IsLoaded)
            {
                VersionTitle.Text = "Version " + RecluseEditor.MainApp.VersionString;
            }

            if (LicenseTitle.IsLoaded)
            {
                LicenseTitle.Text = "Recluse Engine (c) Do what you want with it!";
            }
        }
    }
}