# Robot Dịch Vụ Thông Minh (Smart Service Robot)

**Thiết kế và Triển khai Mô hình Robot Ngành Dịch Vụ F&B**

> Một giải pháp tự động hóa chi phí tối ưu cho nhà hàng, quán cà phê sử dụng AI, IoT và hệ thống nhúng.

![Status](https://img.shields.io/badge/Status-Active-brightgreen)
![License](https://img.shields.io/badge/License-MIT-blue)
![Python](https://img.shields.io/badge/Python-3.8%2B-blue)
![ESP32](https://img.shields.io/badge/ESP32-Based-orange)

---

## Mục Lục

- [Tổng Quan Dự Án](#-tổng-quan-dự-án)
- [Chức Năng Chính](#-chức-năng-chính)
- [Yêu Cầu Kỹ Thuật](#-yêu-cầu-kỹ-thuật)
- [Cấu Trúc Thư Mục](#-cấu-trúc-thư-mục)
- [Cài Đặt & Triển Khai](#️-cài-đặt--triển-khai)
- [Hướng Dẫn Sử Dụng](#-hướng-dẫn-sử-dụng)
- [Kiến Trúc Hệ Thống](#-kiến-trúc-hệ-thống)
- [Kết Quả & Đánh Giá](#-kết-quả--đánh-giá)
- [Hướng Phát Triển](#-hướng-phát-triển)
- [Tác Giả](#-tác-giả)

---

## Tổng Quan Dự Án

### Mô Tả

Dự án này phát triển một **mô hình robot dịch vụ thông minh** dành cho lĩnh vực F&B (Food & Beverage), kết hợp:

- **Vi điều khiển ESP32** – Điều khiển thời gian thực
- **AI Thị giác máy tính (ESP32-CAM)** – Nhận diện số bàn
- **IoT & MQTT** – Giao tiếp không dây
- **Cảm biến hồng ngoại** – Dò line và tránh vật cản
- **Âm thanh thông minh** – Tương tác giọng nói

### Ứng Dụng

Robot có khả năng:
- Nhận order từ Desktop App  
- Di chuyển tự động bám line đến bàn  
- Nhận diện chính xác số bàn bằng AI  
- Phát lời thoại mời khách nhận đồ  
- Quay về trạm chờ tự động  
- Tránh vật cản an toàn

---

## Chức Năng Chính

| Tính Năng | Mô Tả | Công Nghệ |
|-----------|--------|-----------|
| **Nhận Lệnh Từ Xa** | Điều khiển qua Desktop App | MQTT + WiFi |
| **Di Chuyển Tự Động** | Bám line đến bàn chính xác | Cảm biến HW-871 (5 kênh) |
| **Nhận Diện Số Bàn** | AI nhận diện từ hình ảnh | TensorFlow + OpenCV |
| **Tương Tác Giọng Nói** | Phát câu thoại có ngữ cảnh | DFPlayer Mini + PAM8403 |
| **Tránh Vật Cản** | Phát hiện & dừng an toàn | Cảm biến HC-SR04 siêu âm |
| **Giám Sát Thời Gian Thực** | Theo dõi mức pin, trạng thái | Desktop App (Python) |

---

## Yêu Cầu Kỹ Thuật

### Phần Cứng (Hardware)

```
Bộ Xử Lý Chính:
├── ESP32-WROOM-32 (MCU điều khiển)
├── ESP32-CAM (Camera + WiFi)
└── L298N (Motor Driver)

Cảm Biến:
├── HW-871 (Dò line – 5 kênh)
├── HC-SR04 (Siêu âm – Phát hiện vật cản)
└── Khác

Chấp Hành:
├── 2x DC Motor (Bánh xe)
└── DFPlayer Mini + PAM8403 (Âm thanh)

Năng Lượng:
└── Pin Li-ion 24V (BMS tích hợp)
```

### Phần Mềm (Software)

**ESP32/Firmware:**
- Arduino IDE / ESP-IDF
- C/C++
- Libraries: WiFi.h, PubSubClient.h, esp_camera.h

**Desktop App:**
- Python 3.8+
- PyQt/Tkinter (GUI)
- paho-mqtt
- OpenCV (cv2)
- TensorFlow / PyTorch

**Giao Tiếp:**
- MQTT (HiveMQ / Mosquitto Broker)
- WiFi 802.11 b/g/n

---

## Cấu Trúc Thư Mục

```
robot-service-project/
│
├── README.md                    # Tài liệu chính
├── LICENSE                      # MIT License
├── INSTALLATION.md              # Hướng dẫn cài đặt chi tiết
├── ARCHITECTURE.md              # Kiến trúc hệ thống
│
├── firmware/                       # Code ESP32
│   ├── esp32-main/                 # ESP32-WROOM-32 (điều khiển chính)
│   │   ├── main.cpp                # Chương trình chính
│   │   ├── lineFollower.cpp        # Logic bám line (PID)
│   │   ├── motorControl.cpp        # Điều khiển động cơ
│   │   ├── mqttHandler.cpp         # Giao tiếp MQTT
│   │   └── sensorReader.cpp        # Đọc cảm biến
│   │
│   └── esp32-cam/                  # ESP32-CAM (camera + AI)
│       ├── main.cpp                # Camera server
│       ├── camera_config.h         # Cấu hình camera
│       └── wifi_setup.cpp          # Kết nối WiFi
│
├── ai-ml/                          # Mô hình AI & Xử lý Hình Ảnh
│   ├── models/
│   │   ├── number_recognition.h5   # Mô hình nhận diện số bàn
│   │   └── obstacle_detection.h5   # Nhận diện vật cản
│   │
│   ├── preprocessing.py            # Tiền xử lý ảnh
│   ├── inference.py                # Dự đoán AI
│   └── train_model.py              # Huấn luyện mô hình
│
├── desktop-app/                    # Ứng dụng Desktop (GUI)
│   ├── main.py                     # Entry point
│   ├── ui/
│   │   ├── main_window.py          # Cửa sổ chính
│   │   ├── camera_panel.py         # Panel camera
│   │   └── control_panel.py        # Panel điều khiển
│   │
│   ├── mqtt_client.py              # Client MQTT
│   ├── config.py                   # Cấu hình app
│   └── requirements.txt            # Python dependencies
│
├── hardware/                       # Sơ đồ & Tài liệu H/W
│   ├── schematic.pdf               # Sơ đồ nguyên lý
│   ├── circuit_design/
│   │   └── proteus_design.pdsprj   # Proteus Design
│   │
│   └── 3d-models/                  # Model CAD
│       └── robot_frame.step        # Khung robot
│
├── docs/                           # Tài liệu & Hướng dẫn
│   ├── thesis.pdf                  # Đồ án hoàn chỉnh
│   ├── user_manual.md              # Hướng dẫn sử dụng
│   ├── api_reference.md            # Tài liệu API
│   ├── troubleshooting.md          # Xử lý sự cố
│   └── images/                     # Ảnh tài liệu
│
├── tests/                          # Unit Tests
│   ├── test_motor.py
│   ├── test_sensors.py
│   └── test_mqtt.py
│
└── config/                         # File cấu hình
    ├── mqtt_broker.json            # MQTT settings
    ├── wifi_credentials.json       # WiFi config (gitignore)
    └── robot_parameters.yaml       # Thông số robot
```

---

## Cài Đặt & Triển Khai

### 1. **Chuẩn Bị Phần Cứng**

```bash
# Linh kiện cần chuẩn bị:
- ESP32-WROOM-32 Development Board
- ESP32-CAM Module
- Module L298N Motor Driver
- Cảm biến HW-871 (5 kênh dò line)
- Cảm biến HC-SR04 (Siêu âm)
- DFPlayer Mini + PAM8403 (Âm thanh)
- Pin Li-ion 24V với BMS
- 2x DC Motor + Bánh xe
```

### 2. **Flash Firmware ESP32**

```bash
# Clone repository
git clone https://github.com/your-username/robot-service-project.git
cd robot-service-project

# Cài đặt Arduino IDE / PlatformIO
# Arduino IDE: https://www.arduino.cc/en/software
# Hoặc PlatformIO: https://platformio.org/

# Flash ESP32-WROOM-32
cd firmware/esp32-main
# Dùng Arduino IDE hoặc:
# pio run -t upload

# Flash ESP32-CAM
cd ../esp32-cam
# pio run -e esp32-cam -t upload
```

### 3. **Cài Đặt Desktop App**

```bash
cd desktop-app

# Tạo virtual environment
python -m venv venv
source venv/bin/activate  # Linux/Mac
# hoặc: venv\Scripts\activate  # Windows

# Cài đặt dependencies
pip install -r requirements.txt

# Chạy ứng dụng
python main.py
```

### 4. **Thiết Lập MQTT Broker**

```bash
# Sử dụng HiveMQ Cloud (miễn phí) hoặc cài đặt Mosquitto locally
# HiveMQ: https://www.hivemq.cloud/

# Hoặc cài đặt Mosquitto:
# Ubuntu/Debian:
sudo apt-get install mosquitto mosquitto-clients

# macOS:
brew install mosquitto

# Khởi động broker
mosquitto

# Kiểm tra kết nối
mosquitto_sub -h localhost -t "#"
```

### 5. **Cấu Hình WiFi & MQTT**

```bash
# Sửa file config/wifi_credentials.json
{
  "ssid": "YOUR_WIFI_SSID",
  "password": "YOUR_WIFI_PASSWORD",
  "mqtt_broker": "YOUR_MQTT_BROKER_IP",
  "mqtt_port": 1883
}
```

---

## Hướng Dẫn Sử Dụng

### Khởi Động Hệ Thống

1. **Cấp Nguồn Robot**
   ```
   Bật nguồn Pin 24V → Robot khởi động → Kết nối WiFi tự động
   ```

2. **Mở Desktop App**
   ```bash
   python desktop-app/main.py
   ```

3. **Đăng Nhập & Nhập Order**
   ```
   Tên đăng nhập: admin
   Mật khẩu: 123456
   
   Chọn số bàn → Nhấn "Gọi Robot" → Robot di chuyển đến bàn
   ```

4. **Giám Sát Trạng Thái**
   - Xem vị trí robot
   - Theo dõi mức pin
   - Nhận cảnh báo vật cản

### Các Lệnh MQTT Quan Trọng

```
# Gửi lệnh di chuyển đến bàn 5:
mosquitto_pub -h <broker> -t "robot/command" -m '{"table": 5, "action": "move"}'

# Nhận phản hồi trạng thái:
mosquitto_sub -h <broker> -t "robot/status"

# Dừng robot khẩn cấp:
mosquitto_pub -h <broker> -t "robot/command" -m '{"action": "stop"}'
```

---

## Kiến Trúc Hệ Thống

```
┌─────────────────────────────────────────────────────────────┐
│                        DESKTOP APP (Python)                 │
│                   (Điều khiển & Giám sát)                   │
└──────────────────────────┬──────────────────────────────────┘
                           │
                     WiFi + MQTT
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
┌───────▼────────┐  ┌──────▼────────┐  ┌──────▼────────┐
│ ESP32-WROOM-32 │  │  ESP32-CAM    │  │   MQTT Broker │
│  (Main MCU)    │  │  (Camera+AI)  │  │   (HiveMQ)    │
└────┬───────────┘  └───────────────┘  └───────────────┘
     │
     └─► Điều khiển:
         • 2x DC Motor (L298N)
         • Âm thanh (DFPlayer)
         • Cảm biến (HW-871, HC-SR04)
         • LED & Nút nhấn
```

---

## Kết Quả & Đánh Giá

### Hiệu Suất

| Tiêu Chí | Kết Quả | Đánh Giá |
|----------|---------|---------|
| Di chuyển chính xác đến bàn | 95% | Xuất sắc |
| Nhận diện số bàn (AI) | 90% | Tốt |
| Thời gian phản hồi MQTT | <500ms | Nhanh |
| Thời gian hoạt động (pin) | 6-8h | Đủ dùng |
| Chi phí sản xuất | ~$200-300 | Tối ưu |

### Ưu Điểm

- Chi phí cực rẻ so với robot thương mại  
- Tích hợp AI & IoT hiệu quả  
- Dễ mở rộng & phát triển thêm  
- Code mã nguồn mở, dễ hiểu  
- Phù hợp với môi trường F&B nhỏ  

### Nhược Điểm

- Chỉ hoạt động trên đường line cố định  
- Khả năng tránh vật cản cơ bản (chỉ phía trước)  
- Phụ thuộc chất lượng WiFi (có thể bị trễ)  
- Nhận diện AI cần light control tốt  

---

## Hướng Phát Triển

- [ ] **SLAM Navigation** – Loại bỏ line, di chuyển tự do
- [ ] **Multi-Robot Fleet** – Quản lý nhiều robot cùng lúc
- [ ] **Voice Recognition** – Khách gọi robot bằng giọng nói
- [ ] **Advanced Obstacle Detection** – LiDAR thay siêu âm
- [ ] **Mobile App** – Ứng dụng điều khiển trên điện thoại
- [ ] **Edge Computing** – Xử lý AI trực tiếp trên robot

---

## Tài Liệu Tham Khảo

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/)
- [Arduino IDE Guide](https://www.arduino.cc/en/Guide)
- [MQTT Protocol](https://mqtt.org/)
- [TensorFlow Lite](https://www.tensorflow.org/lite)
- [OpenCV Python](https://opencv.org/)

---

## Tác Giả

- **Nguyễn Nhất Phong** (22161166)  
  📧 phongnguyen.15122004@gmail.com  
  📱 0797502945

- **Trần Thiện Hiệu** (22161122)  
  📧 thienhieu2222@gmail.com  
  📱 0799216049

**Giáo viên Hướng dẫn:** ThS. Nguyễn Ngô Lâm

**Trường:** Đại học Công nghệ Kỹ Thuật TP.HCM  
**Khoa:** Điện - Điện tử  
**Bộ môn:** Điện tử - Viễn Thông

---

## License

Dự án này được cấp phép theo **MIT License** - xem file [LICENSE](LICENSE) để chi tiết.

---

## Đóng Góp

Chúng tôi hoan nghênh các đóng góp! Vui lòng:

1. Fork repository
2. Tạo branch feature (`git checkout -b feature/AmazingFeature`)
3. Commit thay đổi (`git commit -m 'Add some AmazingFeature'`)
4. Push lên branch (`git push origin feature/AmazingFeature`)
5. Mở Pull Request

---

## Liên Hệ & Hỗ Trợ

- **Email:** phongnguyen.15122004@gmail.com
- **GitHub Issues:** [Report a Bug](https://github.com/your-username/robot-service-project/issues)
- **Wiki:** [Docs & Tutorials](https://github.com/your-username/robot-service-project/wiki)

---

## ⭐ Nếu bạn thích dự án này, vui lòng cho một **Star!** ⭐

---

**Last Updated:** Tháng 12, 2025  
**Status:** Active Development 🚀
