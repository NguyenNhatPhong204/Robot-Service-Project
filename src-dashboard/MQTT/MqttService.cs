using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using System;
using System.Text;
using System.Threading.Tasks;

namespace DOAN2.MQTT
{
    public class MqttService
    {
        private IMqttClient client;
        private IMqttClientOptions options;

        public event Action<int> OnNumberReceived;
        public event Action OnConnected;

        public MqttService()
        {
            var factory = new MqttFactory();
            client = factory.CreateMqttClient();

            client.UseConnectedHandler(e =>
            {
                Console.WriteLine("[MQTT] CONNECTED!");
                OnConnected?.Invoke();
            });

            client.UseDisconnectedHandler(async e =>
            {
                Console.WriteLine("[MQTT] DISCONNECTED – reconnecting in 2s...");
                await Task.Delay(2000);
                await TryReconnect();
            });

            client.UseApplicationMessageReceivedHandler(e =>
            {
                string msg = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);

                if (int.TryParse(msg, out int number))
                {
                    Console.WriteLine($"[MQTT] Received number = {number}");
                    OnNumberReceived?.Invoke(number);
                }
            });
        }

        public async Task ConnectAsync(string host, int port, string subscribeTopic)
        {
            options = new MqttClientOptionsBuilder()
                .WithTcpServer(host, port)
                .WithCleanSession()
                .Build();

            await client.ConnectAsync(options);

            // subscribe sau khi connect
            await client.SubscribeAsync(subscribeTopic);

            Console.WriteLine($"[MQTT] Subscribed: {subscribeTopic}");
        }

        private async Task TryReconnect()
        {
            try
            {
                await client.ConnectAsync(options);
            }
            catch
            {
                Console.WriteLine("[MQTT] Reconnect failed, retry...");
            }
        }

        public void Publish(string topic, string message)
        {
            if (client == null || !client.IsConnected)
            {
                Console.WriteLine("[MQTT] Publish FAILED (not connected)");
                return;
            }

            var msg = new MqttApplicationMessageBuilder()
                .WithTopic(topic)
                .WithPayload(message)
                .WithAtLeastOnceQoS()
                .Build();

            client.PublishAsync(msg);
            Console.WriteLine($"[MQTT] Published: {topic} = {message}");
        }
    }
}
