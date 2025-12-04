/*
 * Dự án: Hệ thống Khóa cửa Thông minh (Smart Lock)
 * Phần cứng: ESP32, MFRC522 (RFID), SG90 (Servo), SSD1306 (OLED), Keypad 4x4
 * Mô tả: Triển khai Máy trạng thái hữu hạn (FSM) với cơ chế không chặn (Non-blocking).
 */

// ================= [ CẤU HÌNH HỆ THỐNG ] =================
#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Lock"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

// Thông tin WiFi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASS";

// Định nghĩa chân phần cứng (GPIO)
#define PIN_RST         36
#define PIN_SS          5
#define PIN_BUZZER      15
#define PIN_SERVO       16
#define PIN_IR_SENSOR   17
#define PIN_LED         2

// Hằng số hệ thống
#define LOCK_TIMEOUT_MS 5000       // Thời gian tự đóng cửa (ms)
#define SERVO_OPEN_POS  0          // Góc mở servo
#define SERVO_CLOSE_POS 90         // Góc đóng servo

// ================= [ THƯ VIỆN ] =================
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>

// ================= [ KHỞI TẠO NGOẠI VI ] =================
// Cấu hình Keypad 4x4
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Màn hình OLED
Adafruit_SSD1306 display(128, 64, &Wire, 4);

// RFID & Servo
MFRC522 mfrc522(PIN_SS, PIN_RST);
Servo doorServo;

// Đối tượng hệ thống
Preferences preferences;
BlynkTimer timer;

// ================= [ BIẾN TOÀN CỤC ] =================
int secretCode = 1234;       // Mật khẩu mặc định
String inputBuffer = "";     // Bộ đệm lưu dữ liệu nhập
String rfidUID = "";         // Lưu UID thẻ từ
bool isInputReady = false;   // Cờ báo hiệu có dữ liệu cần xử lý
uint8_t inputSource = 0;     // Nguồn nhập: 1=Keypad, 2=RFID, 3=Blynk

// ================= [ MÁY TRẠNG THÁI (FSM) ] =================

// Định nghĩa các trạng thái hoạt động của hệ thống
typedef enum {
    STATE_IDLE,         // Hệ thống chờ (Rảnh)
    STATE_VERIFYING,    // Đang xác thực thông tin
    STATE_UNLOCKED,     // Đã mở khóa (Actuator kích hoạt)
    STATE_ALARM         // Báo động đột nhập
} SystemState_t;

SystemState_t currentState = STATE_IDLE;
unsigned long stateTimer = 0;

// Nguyên mẫu hàm
void FSM_Update(void);
bool Auth_Validate(void);
void Actuator_SetLock(bool lock);
void UI_ShowMessage(const char* title, const char* msg);
void Hardware_PollInputs(void);

// ================= [ LOGIC CHÍNH ] =================

void setup() {
    Serial.begin(115200);

    // Cấu hình GPIO Output/Input
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_IR_SENSOR, INPUT);

    // Khởi tạo Servo
    doorServo.attach(PIN_SERVO);
    doorServo.write(SERVO_CLOSE_POS);

    // Khởi tạo màn hình OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("[LỖI] Không khởi tạo được SSD1306"));
        for (;;); // Treo hệ thống nếu lỗi màn hình
    }
    UI_ShowMessage("System", "Booting...");

    // Kết nối WiFi & Blynk
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Blynk.config(BLYNK_AUTH_TOKEN);

    // Khởi tạo RFID (SPI)
    SPI.begin();
    mfrc522.PCD_Init();

    // Đọc cấu hình từ bộ nhớ Flash (Preferences)
    preferences.begin("door_lock", true); // Chế độ chỉ đọc
    secretCode = preferences.getInt("code", 1234);
    preferences.end();

    delay(500);
    UI_ShowMessage("Ready", "Waiting...");
}

void loop() {
    // Duy trì kết nối IoT (nếu có WiFi)
    if (WiFi.status() == WL_CONNECTED) {
        Blynk.run();
    }
    timer.run();

    // Cập nhật Máy trạng thái
    FSM_Update();
}

/* * Hàm cập nhật trạng thái FSM
 * Xử lý chuyển đổi trạng thái và logic không chặn (Non-blocking)
 */
void FSM_Update(void) {
    switch (currentState) {
        case STATE_IDLE:
            Hardware_PollInputs(); // Quét thiết bị ngoại vi

            // Phát hiện đột nhập: Cảm biến IR kích hoạt khi Servo đang đóng
            if (digitalRead(PIN_IR_SENSOR) == HIGH && doorServo.read() > 10) {
                 currentState = STATE_ALARM;
            }

            // Chuyển trạng thái nếu có dữ liệu nhập
            if (isInputReady) {
                currentState = STATE_VERIFYING;
            }
            break;

        case STATE_VERIFYING:
            UI_ShowMessage("Auth", "Verifying...");
            
            if (Auth_Validate()) {
                Actuator_SetLock(false); // Mở khóa
                stateTimer = millis();   // Lưu thời điểm mở
                currentState = STATE_UNLOCKED;
                isInputReady = false;
            } else {
                UI_ShowMessage("Access", "Denied!");
                // Phản hồi âm thanh báo lỗi
                digitalWrite(PIN_BUZZER, HIGH); delay(200); digitalWrite(PIN_BUZZER, LOW);
                delay(1000); // Giữ thông báo hiển thị một chút
                currentState = STATE_IDLE;
                isInputReady = false;
            }
            break;

        case STATE_UNLOCKED:
            // Logic tự động đóng sau khoảng thời gian Timeout
            if (millis() - stateTimer > LOCK_TIMEOUT_MS) {
                // An toàn: Kiểm tra vật cản trước khi đóng
                if (digitalRead(PIN_IR_SENSOR) == LOW) {
                     // Có vật cản -> Reset timer, tiếp tục chờ
                     stateTimer = millis(); 
                } else {
                    Actuator_SetLock(true); // Đóng khóa
                    currentState = STATE_IDLE;
                    UI_ShowMessage("Door", "Auto Locked");
                }
            }
            break;

        case STATE_ALARM:
            UI_ShowMessage("ALERT", "INTRUSION!");
            Blynk.virtualWrite(V3, "CANH BAO: DOT NHAP");
            
            // Chuông báo động
            for(int i=0; i<3; i++) {
                digitalWrite(PIN_BUZZER, HIGH); delay(100);
                digitalWrite(PIN_BUZZER, LOW); delay(100);
            }

            // Điều kiện tắt báo động (Ví dụ: hết vật cản)
            if (digitalRead(PIN_IR_SENSOR) == LOW) {
                 currentState = STATE_IDLE;
                 Blynk.virtualWrite(V3, "SAFE");
            }
            break;
    }
}

// ================= [ HÀM HỖ TRỢ & XỬ LÝ ] =================

/* * Xác thực thông tin đăng nhập
 * Trả về true nếu hợp lệ, false nếu sai
 */
bool Auth_Validate(void) {
    bool isValid = false;

    // Cách 1: Kiểm tra mật khẩu (Keypad / Blynk)
    if (inputSource == 1 || inputSource == 3) {
        if (inputBuffer.toInt() == secretCode) isValid = true;
    }
    // Cách 2: Kiểm tra thẻ RFID
    else if (inputSource == 2) {
        // Danh sách UID hợp lệ (Cần lưu trong EEPROM thực tế)
        if (rfidUID == "B0 63 CE 73" || rfidUID == "E7 ED D9 73") isValid = true;
    }
    return isValid;
}

/* * Điều khiển cơ cấu chấp hành (Servo/LED)
 * Biến lock: true = Đóng, false = Mở
 */
void Actuator_SetLock(bool lock) {
    if (!lock) { // Mở khóa
        doorServo.write(SERVO_OPEN_POS);
        digitalWrite(PIN_LED, HIGH);
        Blynk.virtualWrite(V1, "OPEN");
    } else { // Đóng khóa
        doorServo.write(SERVO_CLOSE_POS);
        digitalWrite(PIN_LED, LOW);
        Blynk.virtualWrite(V1, "CLOSED");
    }
}

// Hiển thị thông tin lên OLED
void UI_ShowMessage(const char* title, const char* msg) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(title);
    display.setCursor(0, 20);
    display.println(msg);
    display.display();
}

/* * Quét tín hiệu từ phần cứng (Polling)
 * Hàm này không chặn (Non-blocking), gọi liên tục trong loop
 */
void Hardware_PollInputs(void) {
    // 1. Quét RFID
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        rfidUID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            rfidUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            rfidUID.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        rfidUID.toUpperCase();
        rfidUID.trim(); // Xóa khoảng trắng thừa
        
        inputSource = 2; // Đánh dấu nguồn RFID
        isInputReady = true;
        
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        return;
    }

    // 2. Quét bàn phím (Keypad)
    char key = keypad.getKey();
    if (key) {
        // Phản hồi âm thanh khi nhấn phím
        digitalWrite(PIN_BUZZER, HIGH); delay(50); digitalWrite(PIN_BUZZER, LOW); 
        
        if (key == '#') { // Phím # để xác nhận
            inputSource = 1;
            isInputReady = true;
        } else if (key == 'C') { // Phím C để xóa
            inputBuffer = "";
            UI_ShowMessage("Input", "Cleared");
        } else {
            inputBuffer += key;
            // Hiển thị dạng ẩn (Masking) *****
            String mask = "";
            for(int i=0; i<inputBuffer.length(); i++) mask += "*";
            UI_ShowMessage("Enter Code", mask.c_str());
        }
    }
}

// Xử lý dữ liệu từ Blynk (Virtual Pin V2)
BLYNK_WRITE(V2) {
    inputBuffer = param.asString();
    inputSource = 3; // Đánh dấu nguồn Blynk
    isInputReady = true;
}
