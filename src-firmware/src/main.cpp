#include <WiFi.h>
#include <PubSubClient.h>
#include "DFRobotDFPlayerMini.h"

// ============================ WiFi info ================================
const char* ssid = "Rand";
const char* password = "thienhieu089";

// ==================================== MQTT info ==========================
const char* mqtt_server = "172.20.10.3";
const int mqtt_port = 1883;
const char* topic_sub = "restaurant/call";
const char* topic_pub = "restaurant/status";
const char* topic_sub_response = "restaurant/response";
const char* topic_confirm = "restaurant/confirm";
const char* topic_obstacle = "restaurant/obstacle";
const char* topic_position = "restaurant/position";
const char* topic_checkwifi = "restaurant/checkwifi";
const char* topic_checkline = "restaurant/checkline";



// ====================== Hardware (Motor L298N) ==========================
const int pwmPin1 = 13;  
const int pwmPin2 = 14;  
const int pwmPin3 = 26;  
const int pwmPin4 = 27;  

const int TRIG_PIN = 18;
const int ECHO_PIN = 5;

// =============================== SENSOR LINE =========================
const int sensorPins[5] = {33, 25, 32, 34, 35};//  cũ => mới so line  : 32=>34 la 1   ; 33=>35 là 2  ; 25=>32 là 3  ; 26=>33 là 4 ; 27=>25 là 5
int sensorValues[5];

// ================== LINE TELEMETRY ==================
volatile int sharedLine[5] = {1,1,1,1,1};
volatile bool lineUpdated = false;

String lastLinePattern = "";
bool lastWifiState = false;




//=============================chống nhiễu dò cảm biến ==================

static int stableCounterEnd = 0;
static int stableCounterCross = 0;
const int stableNeeded = 8;  


// ============================= PWM CONFIG ==============================
const int ch1 = 0, ch2 = 1, ch3 = 2, ch4 = 3;
const int freq = 20000;      
const int resolution = 10;   
int baseSpeed = 550;

int obstacleThreshold = 8;

// =================== LED báo trạng thái & nút ==========================

#define BUTTON_STEP 4
#define BUTTON_CONFIRM 2
#define LED_PIN 2  

// =========================== cảm biến loa =========================
#define RX_PIN 17
#define TX_PIN 16
HardwareSerial dfSerial(2);

DFRobotDFPlayerMini dfPlayer;

// ============================ Trạng thái ==================================
volatile bool stepInterrupt = false;   
volatile bool running = false;         
volatile bool ignoreCrossLine = false;
volatile bool waitingAI = false;
volatile bool confirmRequired = false;
volatile bool obstacleDetected = false;
volatile bool mustStopAtNextCross = false;

volatile bool crossHandled = false;

// Cờ dừng toàn bộ hệ thống
volatile bool emergencyStop = false;

int Kp = 120;

// ============================ MQTT client ================================
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSerialPrint = 0;

// ======================  chương trình ngắt ===================================
void IRAM_ATTR onStepPressed(){
  stepInterrupt = true;   
}

// HÀM ĐO KHOẢNG CÁCH 

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000);  // timeout 25ms
  if (duration == 0) return 9999;

  return duration * 0.034 / 2;
}

// ====================== PHÁT HIỆN VẠCH NGANG / HẾT LINE ============================
bool detectCrossLine(){
  for (int i = 0; i < 5; i++) {
    if (sensorValues[i] != 0) return false;
  }
  return true;
}

bool detectEndOfLine() {
  for (int i = 0; i < 5; i++) {
    if (sensorValues[i] != 1) return false;
  }
  return true; 
}

// ============================= HÀM ĐIỀU KHIỂN MOTOR ===========================
void motor(int left, int right) {
  left = constrain(left, -1023, 1023);
  right = constrain(right, -1023, 1023);

  if (left >= 0) {
    ledcWrite(ch1, left);
    ledcWrite(ch2, 0);
  } else {
    ledcWrite(ch1, 0);
    ledcWrite(ch2, -left);
  }
  if (right >= 0) {
    ledcWrite(ch3, right);
    ledcWrite(ch4, 0);
  } else {
    ledcWrite(ch3, 0);
    ledcWrite(ch4, -right);
  }
}

void motorStop() {
  motor(0, 0);
  vTaskDelay(pdMS_TO_TICKS(10));
}
// ============================== MQTT CALLBACK ============================
void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();
  Serial.print("MQTT nhận: ");
  Serial.println(msg);


   // --- LỆNH DỪNG TOÀN BỘ ---
    if (msg.equalsIgnoreCase("stop")) {
        emergencyStop = true;       
        running = false;            
        waitingAI = false;          
        ignoreCrossLine = false;
        confirmRequired = false;
        obstacleDetected = true;    
        motorStop();                
        Serial.println("!!! EMERGENCY STOP KÍCH HOẠT !!!");
        return;
    }

    if (!emergencyStop) {
      if (String(topic) == topic_sub) {
        if (msg.toInt() >= 1 && msg.toInt() <= 10) {
          if (client.connected()) {
                client.publish(topic_position, "START");   
                }
          running = true;
          if (client.connected()) {
                client.publish(topic_confirm, "  Unconfirmed");   
                }
          Serial.println("Xe bắt đầu bám line (MQTT)");
        } else if (msg == "0") {
          running = false;
          motorStop();
          Serial.println("Xe dừng (MQTT)");
        } else {
          Serial.println("Lệnh không hợp lệ (MQTT)");
        }
      } else if (String(topic) == topic_sub_response) {
        
        if (msg == "true") {
          ignoreCrossLine = true;
          waitingAI = false;
          running = false;
          confirmRequired = true;
          dfPlayer.playMp3Folder(2);
          delay(2000);
          dfPlayer.playMp3Folder(1);
          Serial.println("AI xác nhận đúng -> Bỏ qua vạch, về nhà");
        
          vTaskDelay(pdMS_TO_TICKS(600));
        } else if (msg == "false") {
          ignoreCrossLine = false;
          mustStopAtNextCross = true;
          waitingAI = false;
          running = true;
          //motor(baseSpeed/4, baseSpeed/4);
          //delay(400);
          vTaskDelay(pdMS_TO_TICKS(400));
          motorStop();
          Serial.println("AI sai -> Tiếp tục bám line, sẽ dừng ở vạch tiếp theo");
        }
  }
}
}
// =========================== KẾT NỐI WIFI ================================
void setup_wifi() {
  Serial.print("Kết nối WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(200));
    Serial.print(".");
    if (millis() - start > 30000) { 
      Serial.println("\nKhông thể kết nối WiFi sau 30s");
      return;
    }
  }
  Serial.println("\nWiFi OK!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());
}
// ============================== MQTT non-blocking reconnect ==========================
unsigned long lastReconnectAttempt = 0;
bool reconnect_mqtt_nonblock() {
  if (client.connected()) return true;
  unsigned long now = millis();
  if (now - lastReconnectAttempt > 5000) {  
    lastReconnectAttempt = now;
    Serial.print("Thử kết nối lại MQTT... ");
    String clientId = "ESP32_LineBot-" + WiFi.macAddress();
    if (client.connect(clientId.c_str())) {
      Serial.println("Thành công!");
      client.subscribe(topic_sub);
      client.subscribe(topic_sub_response);
      client.subscribe(topic_confirm);
      client.subscribe(topic_checkwifi);
    } else {
      Serial.print("Thất bại (code ");
      Serial.print(client.state());
      Serial.println(")");
    }
  }
  return client.connected();
}
// ============================= HÀM BÁM LINE DÙNG ========================
 

void readSensors() {
  for (int i = 0; i < 5; i++) {
    sensorValues[i] = digitalRead(sensorPins[i]);
    sharedLine[i]   = sensorValues[i];   // <-- COPY sang vùng gửi MQTT
  }
  lineUpdated = true;                     // <-- báo có dữ liệu mới
}

// ============================= HÀM BÁM LINE ========================
void lineFollowPID() {
  for (int i = 0; i < 5; i++) {
    sensorValues[i] = digitalRead(sensorPins[i]);
  }
  String pattern = "";
  for (int i = 0; i < 5; i++) pattern += String(sensorValues[i]);
  Serial.println("Sensor: " + pattern);
  if (sensorValues[2] == 0 ) {
    motor(baseSpeed, baseSpeed);
  }
  else if (sensorValues[1] == 0) {
    motor(baseSpeed / 2, baseSpeed);
  }
  else if (sensorValues[3] == 0) {
    motor(baseSpeed, baseSpeed / 3);
  }
  else if (sensorValues[0] == 0) {
    motor(baseSpeed / 3, baseSpeed);
  }
  else if (sensorValues[4] == 0) {
    motor(baseSpeed, baseSpeed / 2);
  }
  else {
    motor(baseSpeed / 2, baseSpeed / 2);
  }
}

// ========================= TASKS ========================
// Task cao: đọc cảm biến + điều khiển motor (10ms loop)
void lineTask(void* pvParameters) {
  (void) pvParameters;
  const TickType_t xDelay = pdMS_TO_TICKS(10); 
  for (;;) {

    if (emergencyStop) {
        motorStop();                  
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;                      
    }

    if (obstacleDetected) {
    motorStop();
    vTaskDelay(pdMS_TO_TICKS(20));
    continue;
    }
    // xử lý ngắt nút STEP (flag set bởi ISR)
    if (stepInterrupt) {
      stepInterrupt = false;
    
      static unsigned long lastStep = 0;
      if (millis() - lastStep > 300) {
        lastStep = millis();
        Serial.println("Ngắt: Nút STEP được nhấn (xử lý trong lineTask)");
        running = false;
        waitingAI = false;
        ignoreCrossLine = false;
        confirmRequired = false;
        motorStop();
        motor(baseSpeed, baseSpeed);
        vTaskDelay(pdMS_TO_TICKS(200));
        motorStop();
        Serial.println("Hoàn tất hành động tiến-lùi (từ ngắt)");
      }
    }

    // --------- ĐANG CHỜ XÁC NHẬN ---------
if (confirmRequired) {
    motorStop();
    
    delay(500);
    
    if (digitalRead(BUTTON_CONFIRM) == LOW) {
        vTaskDelay(pdMS_TO_TICKS(60));
        if (digitalRead(BUTTON_CONFIRM) == LOW) {
            confirmRequired = false;
            running = true;
            //Serial.println("ĐÃ XÁC NHẬN → XE CHẠY TIẾP !");
            dfPlayer.playMp3Folder(3);
           if (client.connected()) {
                client.publish(topic_confirm, "  Confirmed");  
            }
        }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    continue;
}

    if (running) {
      readSensors();
      if (millis() - lastSerialPrint > 200) {
        String pat = "";
        for (int i = 0; i < 5; i++) pat += String(sensorValues[i]);
        Serial.println("Sensor: " + pat);
        lastSerialPrint = millis();
      }
      /*bool endLine = true;
      for (int i = 0; i < 5; i++) if (sensorValues[i] != 1) { endLine = false; break; }
      if (endLine) {
        motorStop();
        Serial.println("okokok - Hết line");
        if (client.connected()) {
            client.publish(topic_position, "END");   
            }
        running = false;
        waitingAI = false;
        ignoreCrossLine = false;
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }
      bool cross = true;
      for (int i = 0; i < 5; i++) if (sensorValues[i] != 0) { cross = false; break; }
      if (cross) {
    if (!ignoreCrossLine && !waitingAI) {
        motorStop();
        waitingAI = true;
        running = false;
    } else if (ignoreCrossLine) {
        
      lineFollowPID();
      }
        Serial.println("Phát hiện vạch ngang -> chờ AI / MQTT");
        vTaskDelay(pdMS_TO_TICKS(500));
        continue;
      }
      lineFollowPID();*/


// ====================== KIỂM TRA HẾT LINE (11111) ======================
bool endLine = true;
for (int i = 0; i < 5; i++) if (sensorValues[i] != 1) { endLine = false; break; }

if (endLine) {
    stableCounterEnd++;
    if (stableCounterEnd >= stableNeeded) {
        stableCounterEnd = 0;

        motorStop();
        Serial.println("okokok - Hết line (ỔN ĐỊNH)");
        if (client.connected()) {
            client.publish(topic_position, "END");
        }

        running = false;
        waitingAI = false;
        ignoreCrossLine = false;
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
    }
} else {
    stableCounterEnd = 0;
}

// ====================== KIỂM TRA VẠCH NGANG (00000) ======================
bool cross = true;
for (int i = 0; i < 5; i++) {
    if (sensorValues[i] != 0) {
        cross = false;
        break;
    }
}

if (cross) {
    stableCounterCross++;

    //  CHỈ XỬ LÝ 1 LẦN KHI VỪA VÀO VẠCH
    if (stableCounterCross >= stableNeeded && !crossHandled) {
        crossHandled = true;         
        stableCounterCross = 0;

        // ====== ƯU TIÊN: DỪNG BẮT BUỘC ======
        if (mustStopAtNextCross) {
            motorStop();
            running = false;
            waitingAI = true;
            mustStopAtNextCross = false;

            Serial.println("DỪNG tại vạch ngang kế tiếp theo yêu cầu AI");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ====== DỪNG BÌNH THƯỜNG ======
        if (!ignoreCrossLine && !waitingAI) {
            motorStop();
            running = false;
            waitingAI = true;

            Serial.println("Phát hiện vạch ngang → chờ AI");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ====== ĐƯỢC PHÉP BỎ QUA ======
        if (ignoreCrossLine) {
            lineFollowPID();
        }
    }
}
else {
    // QUAN TRỌNG: CHỈ RESET KHI RỜI KHỎI VẠCH
    stableCounterCross = 0;
    crossHandled = false;
}


/*bool cross = true;
for (int i = 0; i < 5; i++) {
    if (sensorValues[i] != 0) {
        cross = false;
        break;
    }
}

if (cross) {
    stableCounterCross++;

    if (stableCounterCross >= stableNeeded) {
        stableCounterCross = 0;

        // ==================== ƯU TIÊN: bắt buộc dừng ở vạch ngang kế tiếp ======================
        if (mustStopAtNextCross) {
            motorStop();
            running = false;
            waitingAI = true;
            mustStopAtNextCross = false;

            Serial.println("DỪNG tại vạch ngang kế tiếp theo yêu cầu AI");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ==================== Dừng thông thường ===================================
        if (!ignoreCrossLine && !waitingAI) {
            motorStop();
            waitingAI = true;
            running = false;

            Serial.println("Phát hiện vạch ngang → chờ AI / MQTT");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // ======================= Bỏ qua vạch (được phép) =============================
        if (ignoreCrossLine) {
            lineFollowPID();
        }
    }
} else {
    stableCounterCross = 0;
}*/


// ====================== TIẾP TỤC BÁM LINE NẾU KHÔNG CÓ VẠCH ======================
lineFollowPID();

    } else {
      motorStop();
    }

    vTaskDelay(xDelay);
  }
}

// Task trung: xử lý MQTT, nhận tin, giữ kết nối
void mqttTask(void* pvParameters) {
  (void) pvParameters;
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  const TickType_t xDelay = pdMS_TO_TICKS(200); 
  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    }
    
    reconnect_mqtt_nonblock();

    
    if (client.connected()) {
      client.loop();
    }

    vTaskDelay(xDelay);
  }
}

// Task thấp: gửi trạng thái, LED, debug nhẹ (1s)
void statusTask(void* pvParameters) {
  (void) pvParameters;
  const TickType_t xDelay = pdMS_TO_TICKS(1000);
  for (;;) {
    digitalWrite(LED_PIN, (running ? HIGH : LOW));

    
    if (client.connected()) {
      String st = running ? "running" : "stopped";
      if (waitingAI) st = "waitingAI";
      client.publish(topic_pub, st.c_str());
    }

    vTaskDelay(xDelay);
  }
}

// TẠO TASK VẬT CẢN 

void obstacleTask(void* pvParameters) {
  (void) pvParameters;
  const TickType_t xDelay = pdMS_TO_TICKS(80);

  static bool previousState = false;

  for (;;) {
    long d = readDistanceCM();

    if (d < obstacleThreshold) {
      if (!previousState) {
        previousState = true;
        obstacleDetected = true;
        running = false;
        motorStop();
       
        Serial.printf("!!! Vật cản: %ld cm → DỪNG XE !!!\n", d);

        if (client.connected()) {
          client.publish(topic_obstacle, "detected");
        }
        dfPlayer.playMp3Folder(4);
      }
    }
    else {
      if (previousState) {
        previousState = false;
        obstacleDetected = false;
        Serial.println(">> ĐÃ HẾT VẬT CẢN → XE CHẠY TIẾP");

        if (client.connected()) {
          client.publish(topic_obstacle, "clear");
        }

        running = true;
      }
    }

    vTaskDelay(xDelay);
  }
}

void telemetryTask(void* pvParameters) {
  (void) pvParameters;
  const TickType_t xDelay = pdMS_TO_TICKS(200);

  for (;;) {

    // ================= WIFI =================
    bool wifiNow = (WiFi.status() == WL_CONNECTED);
    if (wifiNow != lastWifiState) {
      lastWifiState = wifiNow;

      if (client.connected()) {
                client.publish(topic_checkwifi, wifiNow ? "connected" : "disconnected");
                }

      Serial.printf("[TEL] WiFi: %s\n",
                    wifiNow ? "CONNECTED" : "DISCONNECTED");
    }

    // ================= LINE =================
    if (lineUpdated) {
      lineUpdated = false;

      String pattern = "";
      for (int i = 0; i < 5; i++) {
        pattern += String(sharedLine[i]);
      }

      if (pattern != lastLinePattern) {
        lastLinePattern = pattern;

        if (client.connected()) {
          client.publish(topic_checkline, pattern.c_str());
        }

        Serial.printf("[TEL] Line: %s\n", pattern.c_str());
      }
    }

    vTaskDelay(xDelay);
  }
}


// ======================== SETUP =============================
void setup() {
  Serial.begin(9600);
  dfSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(1000);

  if (!dfPlayer.begin(dfSerial)) {
    Serial.println(" Không kết nối DFPlayer!");  
    while (true);
  }
  Serial.println(" DFPlayer OK!"); 
  dfPlayer.volume(25);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_STEP, INPUT_PULLUP);
  pinMode(BUTTON_CONFIRM, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


  attachInterrupt(digitalPinToInterrupt(BUTTON_STEP), onStepPressed, FALLING);

  for (int i = 0; i < 5; i++) pinMode(sensorPins[i], INPUT);

  ledcSetup(ch1, freq, resolution);
  ledcSetup(ch2, freq, resolution);
  ledcSetup(ch3, freq, resolution);
  ledcSetup(ch4, freq, resolution);

  ledcAttachPin(pwmPin1, ch1);
  ledcAttachPin(pwmPin2, ch2);
  ledcAttachPin(pwmPin3, ch3);
  ledcAttachPin(pwmPin4, ch4);

  setup_wifi();
  
  xTaskCreatePinnedToCore(lineTask, "LineTask", 4096, NULL, 3, NULL, 1);   
  xTaskCreatePinnedToCore(mqttTask, "MQTTTask", 4096, NULL, 2, NULL, 0);   
  xTaskCreatePinnedToCore(statusTask, "StatusTask", 3072, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(obstacleTask, "ObstacleTask", 4096, NULL, 3, NULL, 1);

  Serial.println("Hệ thống sẵn sàng! Tasks đã khởi tạo.");
}

// ========================= LOOP ========================

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
