using MockDashboard;
using System.Windows;

namespace DOAN2
{
    public partial class IntroWindow : Window
    {
        public IntroWindow()
        {
            InitializeComponent();
            PlayIntro();
        }

        private void PlayIntro()
        {
            // Đường dẫn video mp4
            // Ví dụ: "Videos/intro.mp4" trong thư mục project
            IntroVideo.Source = new System.Uri(System.IO.Path.Combine(System.AppDomain.CurrentDomain.BaseDirectory, "intro.mp4"));
            IntroVideo.Play();
        }

        // Khi video kết thúc
        private void IntroVideo_MediaEnded(object sender, RoutedEventArgs e)
        {
            MainWindow main = new MainWindow();
            main.Show();
            this.Close(); // đóng Intro
        }
    }
}
