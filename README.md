# 🔐 Smart Lock System – ESP32  
Hệ thống khóa cửa thông minh sử dụng **ESP32**, **RFID**, **Keypad 4x4**, **OLED SSD1306**, **Servo SG90** và **Blynk IoT**.  
Dự án được xây dựng theo kiến trúc **FSM – Finite State Machine (Máy trạng thái hữu hạn)** và cơ chế **Non-blocking**, đảm bảo hoạt động ổn định và không bị treo.

---

## 📌 1. Giới thiệu  
Dự án mô phỏng một hệ thống khóa cửa thông minh đa phương thức xác thực:

- Nhập mật khẩu qua Keypad  
- Quét thẻ từ RFID  
- Nhập mật khẩu từ Blynk App  
- Báo động nếu phát hiện đột nhập bằng cảm biến IR  

Tất cả được xử lý bằng FSM để đảm bảo logic rõ ràng, dễ bảo trì và dễ mở rộng.

---

## 🧩 2. Phần cứng sử dụng  

| Thiết bị | Mô tả |
|---------|-------|
| **ESP32** | Vi điều khiển chính |
| **MFRC522** | Module đọc RFID |
| **Keypad 4x4** | Nhập mật khẩu |
| **Servo SG90** | Đóng/mở chốt cửa |
| **OLED SSD1306 (I2C)** | Hiển thị |
| **IR Sensor** | Phát hiện đột nhập |
| **Buzzer** | Chuông cảnh báo |
| **LED** | Trạng thái cửa |
| **Blynk IoT** | Điều khiển từ xa |

---

## ⚙️ 3. Sơ đồ kết nối (GPIO Mapping)

### ESP32 → RFID MFRC522

| ESP32 | MFRC522 |
|-------|---------|
| 5 | SS |
| 36 | RST |
| 23 | MOSI |
| 19 | MISO |
| 18 | SCK |

### Keypad 4x4

| Hàng/Cột | GPIO |
|----------|------|
| R1 | 13 |
| R2 | 12 |
| R3 | 14 |
| R4 | 27 |
| C1 | 26 |
| C2 | 25 |
| C3 | 33 |
| C4 | 32 |

### Thiết bị khác

| Chức năng | GPIO |
|-----------|------|
| Servo SG90 | 16 |
| Buzzer | 15 |
| IR Sensor | 17 |
| LED Status | 2 |
| OLED SDA/SCL | 21 / 22 |

---

## 🧠 4. FSM – Finite State Machine  

Hệ thống sử dụng 4 trạng thái chính:

| Trạng thái | Mô tả |
|------------|-------|
| **STATE_IDLE** | Chờ thao tác |
| **STATE_VERIFYING** | Đang xác thực dữ liệu |
| **STATE_UNLOCKED** | Đã mở khóa, chờ Auto-lock |
| **STATE_ALARM** | Báo động khi phát hiện đột nhập |

FSM đảm bảo hệ thống hoạt động mượt và không bị block.

---

## 🔓 5. Tính năng nổi bật  

### ✔ Đa phương thức xác thực  
- Keypad  
- RFID card  
- Blynk App  

### ✔ Tự động đóng cửa  
Servo sẽ tự đóng sau `LOCK_TIMEOUT_MS`.

### ✔ Báo động thông minh  
Phát hiện đột nhập bằng IR khi cửa đang mở → kích hoạt cảnh báo + gửi thông tin lên Blynk.

### ✔ Non-blocking  
- Sử dụng Polling liên tục  
- Giảm thiểu delay  
- Không làm treo hệ thống khi xử lý nhiều thiết bị cùng lúc

---

## 📱 6. Blynk IoT  

| Virtual Pin | Chức năng |
|-------------|-----------|
| `V1` | Trạng thái cửa (OPEN/CLOSED) |
| `V2` | Nhập mật khẩu từ app |
| `V3` | Nhận thông báo cảnh báo |

---

## 💾 7. Lưu mật khẩu trong Flash – Preferences  

Mật khẩu được lưu vào Flash thông qua namespace:

