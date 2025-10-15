#define BLYNK_TEMPLATE_ID "TMPL63xLsGVJg"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "hI8kePr-faFVyeinW8gdZCq9u4tjSuJ8"
// Necessary Libraries
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>

char ssid[] = "Cuong";
char pass[] = "000000000";
BlynkTimer timer;
// Keypad Configurations
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

// OLED Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pinouts
#define RST_PIN 36
#define SS_PIN 5
#define BUZZER 15
#define SERVO_PIN 16
#define INFRARED_PIN 17
#define LED_PIN 2

int Secret_Code = 1234;

// Necessary Variables
String ID_e;
int value = 0;
//bool done_flag = 0;
bool rfid_flag = 1;

MFRC522 mfrc522(SS_PIN, RST_PIN);

const char* NAME;

Servo myservo;
int doorState = 0;
String blynkPassword = "";

Preferences preferences;

BLYNK_WRITE(V2) {
    String tempPassword = param.asString();
    blynkPassword = tempPassword;
    Serial.print("Mật khẩu từ Blynk: ");
    Serial.println(blynkPassword);
    display.setCursor(1, 1);
    if (blynkPassword.length() == 4) {
        if (blynkPassword.toInt() == Secret_Code) {
            Serial.println("Mật khẩu Blynk chính xác!");
            Door_Open();
            display.clearDisplay();
            display.setCursor(1, 1);
            display.println("Correct!");
            display.display();
            delay(1000);
            AutoCloseDoor();
        } else {
            Serial.println("Mật khẩu Blynk không chính xác!");
            display.clearDisplay();
            display.setCursor(1, 1);
            display.println("Wrong!");
            display.display();
            delay(1000);
        }
        blynkPassword = "";
        //Blynk.virtualWrite(V2, "");
    } else {
        Serial.println("Mật khẩu không đủ 4 số!");
        display.clearDisplay();
        display.setCursor(1, 1);
        display.println("Not 4");
        display.println("\nDigit");
        display.display();
        delay(1000);
        blynkPassword = "";
    }
    Blynk.virtualWrite(V2, "*");
}

void reconnect() {
    if (WiFi.status() != WL_CONNECTED || !Blynk.connected()) {
        WiFi.begin(ssid, pass);
        Serial.println("Reconnecting...");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(1, 1);
        display.print("ReConnecting...");
        display.display();
        for (int i = 1; i < 10; i++) {
            if (WiFi.status() == WL_CONNECTED) {
                Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
                delay(2000);
                break;
            } else
                Serial.print(".");
            delay(1000);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi reconnected successfully.");
            display.print("\nReConnected WiFi!");
            display.display();
            delay(2000);
        } else {
            Serial.println("WiFi reconnection failed.");
            display.print("\nWifi failed!");
            display.display();
            delay(2000);
        }
        if (Blynk.connected()) {
            Serial.println("Blynk reconnected successfully.");
            display.print("\nReConnected blynk!");
            display.display();
            delay(2000);
        } else {
            Serial.println("Blynk reconnection failed.");
            display.print("\nBlynk failed!");
            display.display();
            delay(2000);
        }
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    myservo.attach(SERVO_PIN);
    myservo.write(90);
    digitalWrite(BUZZER, LOW);
    pinMode(INFRARED_PIN, INPUT);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(1, 1);
    WiFi.begin(ssid, pass);
    Serial.print("Connecting WiFi...");
    display.print("Connecting WiFi...");
    display.display();
    delay(2000);
    for (int i = 1; i < 10; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected WiFi!");
            display.print("\nConnected WiFi!");
            display.display();
            Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
            delay(2000);
            Serial.println("Connected Blynk!");
            display.print("\nConnected Blynk!");
            display.display();
            delay(2000);
            break;
        } else {
            Serial.print(".");
            display.print(".");
        }
        delay(1000);
    }
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(1, 1);
    display.print("RFID Door Lock");
    display.display();
    delay(2000);
    SPI.begin();
    mfrc522.PCD_Init();
    timer.setInterval(20000L, reconnect);
    timer.setInterval(5000L, intrusionAlert);

    preferences.begin("door_lock");
    Secret_Code = preferences.getInt("secret_code", 1234);
    Serial.print("Secret Code: ");
    Serial.println(Secret_Code);
}

void loop() {
    if (Blynk.connected()) {
        Blynk.run();
    }
    timer.run();
    handleKeypad();
    handleRFID();
    handlePinCode();
}

void handleKeypad() {
    char key = keypad.getKey();
    if (key) {
        Serial.println(key);
        beep(200);
        if (key == '*') {
            changeSecretCode();
            }
        if (key == 'A') {
            enterPinCodeMode();
        }
        if (key == 'C') {
            resetDevice();
        }
    }
}

void enterPinCodeMode() {
    Serial.println("A");
    Serial.println("Pin Code");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.println(("Pin Code"));
    display.setCursor(25, 20);
    display.println(("Mode"));
    display.display();
    beep(200);
    beep(200);
    delay(1000);
    Serial.println("Enter Code");
    rfid_flag = 0;
}

void resetDevice() {
    preferences.remove("secret_code");
    ESP.restart();
}

void handleRFID() {
    if (rfid_flag == 1) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(10, 30);
        display.print("Scan Tag");
        display.display();

        if (!mfrc522.PICC_IsNewCardPresent()) {
            return;
        }
        if (!mfrc522.PICC_ReadCardSerial()) {
            return;
        }

        String content = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        content.toUpperCase();

        if (content.substring(1) == "B0 63 CE 73" || content.substring(1) == "E7 ED D9 73") {
            Serial.println(content.substring(1));
            Door_Open();
            delay(1000);
            AutoCloseDoor();
            delay(1000);
        } else {
            Serial.println("Not Registered");
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(38, 1);
            display.print("Not");
            display.setCursor(5, 30);
            display.print("Registered");
            display.display();
            beep(200);
            beep(200);
            beep(200);
            Serial.println("You can't enter.");
        }
        content.substring(1) = "";
    }
}

void handlePinCode() {
    if (rfid_flag == 0) {
        if (Keypad_Input() == Secret_Code) {
            Serial.println("Correct Code");
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(20, 20);
            display.println("Correct");
            display.setCursor(30, 40);
            display.println("Code");
            display.display();
            Door_Open();
            beep(100);
            rfid_flag = 1;
            delay(1000);
            AutoCloseDoor();
        } 
        else if(Keypad_Input() == -1){
            Serial.println("RFID MODE");
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 20);
            display.println(("RFID MODE"));
            display.display();
            beep(200);
            delay(1000);
        }
          else {
            Serial.println("Wrong Code");
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 20);
            display.println(("Wrong Code"));
            display.display();
            beep(200);
            beep(200);
            beep(200);
            delay(1000);
        }
        ID_e = "";
    }
}

// ... (Các hàm còn lại: beep, Keypad_Input, Door_Open, Door_Close, intrusionAlert, AutoCloseDoor, updateBlynkDoorState, changeSecretCode)
// Just a normal beep sound
void beep(int duration) {
  digitalWrite(BUZZER, HIGH);
  delay(duration);
  digitalWrite(BUZZER, LOW);
  delay(30);
}

int Keypad_Input(void) {
  int i = 0;
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0); // Start at top-left corner
  display.println(("Enter Code"));
  display.display();
  i = 0;
  ID_e = "";
  value = 0;
  while (1) {
    char key = keypad.getKey();

    if (key && i < 4) {
      ID_e = ID_e + key;
      value = ID_e.toInt();
      Serial.println(value);
      display.setTextSize(2); // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE); // Draw white text
      display.setCursor(i * 35, 50); // Start at top-left corner
      display.println(("*"));
      display.display();
      beep(100);
      i++;
    }
    if (key == '#') {
      //done_flag = 1;
      Serial.println("DONE");
      i = 0;
      ID_e = "";
      return (value);
    }
    if (key == 'B') {
      rfid_flag = 1;
      ID_e = "";
      return (-1);
    }
  }
}
// Opens the Door, and close it after 5 sec if no obstacle is detected
void Door_Open() {
  beep(200);
  myservo.write(0); // Mở cửa (0 độ)
  doorState = 1;
  digitalWrite(LED_PIN, HIGH); // Bật đèn LED
  updateBlynkDoorState();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.println("Welcome ");
  display.setCursor(1, 40);
  display.print("Door Open");
  display.display();
  Serial.println("DOOR OPENED");
  delay(1000);
}

void Door_Close() {
  beep(200);
  myservo.write(90); // Đóng cửa (90 độ)
  doorState = 0;

  digitalWrite(LED_PIN, LOW);
  updateBlynkDoorState();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(1, 1);
  display.print("Door");
  display.setCursor(1, 20);
  display.print("Closed");
  display.display();
  delay(1000);
  Serial.println("DOOR CLOSED");
}

void intrusionAlert() {
  Serial.println(digitalRead(INFRARED_PIN));
  if (digitalRead(INFRARED_PIN) == HIGH && doorState == 0) {
    Blynk.virtualWrite(V3, "CANH BAO");
    Blynk.virtualWrite(V0, 1); // Bật đèn LED V0
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(1, 1);
    display.print("CANH BAO"); 
    display.display();
    delay(2000);
      for (int i = 0; i < 5; i++) {
      beep(200);
      delay(300);
    }
  } else {
    Blynk.virtualWrite(V3, "NORMAL");
    Blynk.virtualWrite(V0, 0); // Tắt đèn LED V0
  }
}
void AutoCloseDoor() {
  if (doorState == 1) {
    int obstacle_time = 0; // Thời gian phát hiện vật cản
    while (obstacle_time < 5000) { // Kiểm tra trong 5 giây
      if (digitalRead(INFRARED_PIN) == LOW) { // Nếu có vật cản
        obstacle_time += 100; // Tăng thời gian phát hiện
        delay(100);
      } else {
        obstacle_time = 0; // Reset thời gian nếu không có vật cản
        delay(100);
      }
      char key = keypad.getKey(); // Kiểm tra nút nhấn
      if (key == 'D') { // Nếu nhấn D, đóng cửa ngay
        break;
      }
    }
    Door_Close();
    delay(1000);
  }
}
void updateBlynkDoorState() {
  if (doorState == 1) {
    Blynk.virtualWrite(V1, "OPEN");
    Serial.println("Door: OPEN");
  } else {
    Blynk.virtualWrite(V1, "CLOSED");
    Serial.println("Door: CLOSED");
  }
}
void changeSecretCode() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1);
  display.println("Old Code:");
  display.display();
  delay(2000);
  int oldCode = Keypad_Input(); // Nhập mật khẩu cũ từ bàn phím
  if (oldCode == Secret_Code) { // Kiểm tra mật khẩu cũ
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1, 1);
    display.println("New Code:");
    display.display();
    delay(2000);
    int newCode = Keypad_Input(); // Nhập mật khẩu mới từ bàn phím

    bool fourDigits = false; // Biến cờ

    // Kiểm tra xem người dùng có nhập đủ 4 chữ số hay không
    String tempCode = String(newCode);
    if (tempCode.length() == 4) {
      fourDigits = true;
    } else {
      // Vì Keypad_input() trả về 0 nếu người dùng ấn B, nên ta cần kiểm tra lại, nếu newCode = 0 và tempCode.length() = 1 thì đó là 0000
      if (newCode == 0 && tempCode.length() <=4) {
        fourDigits = true;
      }
    }

    if (fourDigits) { // Kiểm tra biến cờ
    // Kiểm tra biến cờ thay vì (newCode != 0)
      Secret_Code = newCode;
      preferences.putInt("secret_code", Secret_Code);
      Serial.print("Secret Code changed to: ");
      Serial.println(Secret_Code);
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1, 1);
      display.println("Code Saved");
      display.display();
      delay(2000);
    } else {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1, 1);
      display.println("Invalid Code");
      display.display();
      delay(2000);
    }
  } else {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1, 1);
    display.println("Wrong Code");
    display.display();
    delay(2000);
  }
}
