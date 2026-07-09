using DOAN2.Controls;
using MockDashboard.Controls;
using System.Diagnostics;
using System.IO;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;

namespace MockDashboard
{
    public partial class MainWindow : Window
    {
        private IMqttClient mqttClient;
        private IMqttClientOptions options;
        private const string topic_pub = "restaurant/call";

        // ✅ THÊM BIẾN LƯU PROCESS AI
        private Process aiProcess;

        public MainWindow()
        {
            InitializeComponent();
           

            BtnStop.Click += BtnStop_Click;


            

            InitializeMqtt();
            MainContent.Children.Clear();
            MainContent.Children.Add(new CameraPanel());

        }

        

        private async void InitializeMqtt()
        {
            try
            {
                var factory = new MqttFactory();
                mqttClient = factory.CreateMqttClient();

                options = new MqttClientOptionsBuilder()
                    .WithClientId("MainWindow_Client")
                    .WithTcpServer("localhost", 1883)
                    .Build();

                mqttClient.UseConnectedHandler(async e =>
                {
                    Console.WriteLine("MainWindow: MQTT connected!");
                });

                mqttClient.UseDisconnectedHandler(async e =>
                {
                    Console.WriteLine("MainWindow: MQTT disconnected!");
                    await Task.Delay(5000);
                    try { await mqttClient.ConnectAsync(options); }
                    catch { }
                });

                await mqttClient.ConnectAsync(options);
            }
            catch (Exception ex)
            {
                MessageBox.Show("MQTT lỗi: " + ex.Message);
            }
        }

        private async void BtnStop_Click(object sender, RoutedEventArgs e)
        {
            if (mqttClient != null && mqttClient.IsConnected)
            {
                var message = new MqttApplicationMessageBuilder()
                    .WithTopic(topic_pub)
                    .WithPayload("stop")
                    .WithExactlyOnceQoS()
                    .WithRetainFlag(false)
                    .Build();

                await mqttClient.PublishAsync(message);
                Console.WriteLine("MQTT Sent: stop");
            }
            else
            {
                MessageBox.Show("Chưa kết nối MQTT!");
            }
        }

        private void Overlay_MouseDown(object sender, MouseButtonEventArgs e)
        {
            if (isMenuOpen)
                ToggleMenu();
        }

        bool isMenuOpen = false;

        private void ToggleMenu()
        {
            DoubleAnimation anim = new DoubleAnimation()
            {
                Duration = TimeSpan.FromMilliseconds(250),
                EasingFunction = new CubicEase { EasingMode = EasingMode.EaseInOut },
                From = isMenuOpen ? 0 : -250,
                To = isMenuOpen ? -250 : 0
            };

            Overlay.Visibility = isMenuOpen ? Visibility.Collapsed : Visibility.Visible;
            SideMenu.Visibility = Visibility.Visible;

            MenuTransform.BeginAnimation(TranslateTransform.XProperty, anim);
            isMenuOpen = !isMenuOpen;
        }

        private void MenuButton_Click(object sender, RoutedEventArgs e) => ToggleMenu();

        private void MenuHome_Click(object sender, RoutedEventArgs e)
{
    // Xóa mọi nội dung cũ
    MainContent.Children.Clear();

    // Load lại CameraPanel
    var cam = new CameraPanel();
    cam.HorizontalAlignment = HorizontalAlignment.Stretch;
    cam.VerticalAlignment = VerticalAlignment.Stretch;

    MainContent.Children.Add(cam);

    // Khôi phục layout bình thường
    MainColumn.Width = new GridLength(1050);      
    RightPanel.Width = new GridLength(420);     
    RightContent.Visibility = Visibility.Visible;

    if (isMenuOpen) ToggleMenu();
}



        private void MenuMonitor_Click(object sender, RoutedEventArgs e)
        {
            MainContent.Children.Clear();

            var monitor = new Monitor1();
            monitor.HorizontalAlignment = HorizontalAlignment.Stretch;
            monitor.VerticalAlignment = VerticalAlignment.Stretch;
            MainContent.Children.Add(monitor);   
            MainColumn.Width = new GridLength(1, GridUnitType.Star);    
            RightPanel.Width = new GridLength(0);
            RightContent.Visibility = Visibility.Collapsed;

            if (isMenuOpen) ToggleMenu();
        }

    }
}
