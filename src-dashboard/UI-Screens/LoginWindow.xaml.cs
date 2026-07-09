using DOAN2;
using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Animation;

namespace MockDashboard
{
    public partial class LoginWindow : Window
    {
        private bool isShowingPassword = false;

        public LoginWindow()
        {
            InitializeComponent();
            FadeInWindow();
        }

        // ===== FADE IN =====
        private void FadeInWindow()
        {
            DoubleAnimation fade = new DoubleAnimation(0, 1, new Duration(TimeSpan.FromMilliseconds(500)));
            this.BeginAnimation(Window.OpacityProperty, fade);
        }

        // ===== EXIT =====
        private void Exit_Click(object sender, RoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        // ===== TOGGLE PASSWORD =====
        private void TogglePassword_Click(object sender, RoutedEventArgs e)
        {
            isShowingPassword = !isShowingPassword;

            if (isShowingPassword)
            {
                txtPassVisible.Text = txtPass.Password;
                txtPassVisible.Visibility = Visibility.Visible;
                txtPass.Visibility = Visibility.Collapsed;
            }
            else
            {
                txtPass.Password = txtPassVisible.Text;
                txtPassVisible.Visibility = Visibility.Collapsed;
                txtPass.Visibility = Visibility.Visible;
            }
        }

        // ===== LOGIN BUTTON =====
        private async void Login_Click(object sender, RoutedEventArgs e)
        {
            string user = txtUser.Text.Trim();
            string pass = isShowingPassword ? txtPassVisible.Text : txtPass.Password;

            if (string.IsNullOrWhiteSpace(user) || string.IsNullOrWhiteSpace(pass))
            {
                Shake();
                MessageBox.Show("Fields cannot be empty!", "Warning",
                                MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            loadingBar.Visibility = Visibility.Visible;
            btnLogin.IsEnabled = false;

            for (int i = 0; i <= 100; i += 20)
            {
                loadingBar.Value = i;
                await Task.Delay(120);
            }

           

            if (user == "Rand" && pass == "1234")
            {
                // Mở IntroWindow
                IntroWindow intro = new IntroWindow();
                intro.Show();

                this.Close(); // đóng LoginWindow
            }
            else
            {
                MessageBox.Show("Sai tài khoản hoặc mật khẩu!", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            loadingBar.Visibility = Visibility.Collapsed;
            btnLogin.IsEnabled = true;
        }

        // ===== SHAKE EFFECT =====
        private void Shake()
        {
            ThicknessAnimation shake = new ThicknessAnimation
            {
                From = new Thickness(-10, 0, 10, 0),
                To = new Thickness(10, 0, -10, 0),
                Duration = TimeSpan.FromMilliseconds(80),
                AutoReverse = true,
                RepeatBehavior = new RepeatBehavior(3)
            };
            this.BeginAnimation(MarginProperty, shake);
        }
    }
}
