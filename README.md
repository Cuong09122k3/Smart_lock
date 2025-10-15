## 🔐 Dự án: Hệ thống khóa cửa thông minh sử dụng ESP32 và nền tảng Blynk

**Mục tiêu:**  
Thiết kế và xây dựng **hệ thống khóa cửa thông minh toàn diện**, vừa đảm bảo an ninh cao, vừa mang lại sự tiện lợi cho người dùng trong sinh hoạt hằng ngày.

---

### 🎯 Các chức năng chính
- **Xác thực đa phương thức:**  
  Người dùng có thể mở khóa bằng **thẻ RFID** hoặc **mật khẩu** nhập từ bàn phím **Keypad 4x4**.  

- **Điều khiển và giám sát từ xa:**  
  Cho phép mở khóa, khóa cửa và theo dõi trạng thái (đóng/mở, cảnh báo) thông qua **ứng dụng di động Blynk**.  

- **Cảnh báo an ninh:**  
  Phát hiện hành vi xâm nhập trái phép và gửi **thông báo cảnh báo ngay lập tức** lên ứng dụng.  

- **Tự động hóa:**  
  Cửa **tự động khóa lại** khi đóng, dựa trên tín hiệu từ **cảm biến hồng ngoại**.  

---

### ⚙️ Thiết kế hệ thống

#### 🧠 Khối xử lý trung tâm
- **ESP32** làm vi điều khiển chính, xử lý toàn bộ logic xác thực, điều khiển và kết nối IoT.

#### 🪪 Khối xác thực
- **RFID-RC522** đọc thẻ từ hợp lệ.  
- **Keypad 4x4** cho phép người dùng nhập mật khẩu.

#### 🔩 Khối chấp hành
- **Servo SG90** điều khiển chốt khóa (đóng/mở cửa).  

#### 👁️ Khối cảm biến
- **Cảm biến hồng ngoại (IR Sensor)** xác định trạng thái cửa (đang mở hay đóng).  

#### 🔔 Khối thông báo
- **Màn hình OLED 0.96"** hiển thị thông tin (trạng thái cửa, thông báo xác thực).  
- **Đèn LED và còi báo động (buzzer)** phát tín hiệu cảnh báo khi có truy cập trái phép.

#### 🔋 Khối nguồn
- **Mạch YX850** giúp chuyển nguồn tự động, đảm bảo hệ thống hoạt động liên tục ngay cả khi mất điện.  

---

### 💻 Phần mềm và Nền tảng
- **Arduino IDE:** Viết và nạp chương trình cho ESP32.  
- **Blynk IoT:** Tạo giao diện điều khiển và giám sát trên điện thoại (bật/tắt khóa, nhận thông báo cảnh báo).  
- Giao tiếp giữa phần cứng và ứng dụng thông qua **kết nối Wi-Fi** và **Blynk Cloud API**.

---

### 🧠 Kết quả đạt được
- Hoàn thiện **mô hình khóa cửa thông minh** với đầy đủ phần cứng và phần mềm.  
- **Độ ổn định cao**, phản hồi nhanh khi thao tác khóa/mở.  
- **Kết nối Blynk ổn định**, thông báo gửi tức thời khi có sự kiện an ninh.  
- Hệ thống đạt **100% các mục tiêu đề ra** trong phạm vi thử nghiệm.  

---

### 🧰 Tóm tắt phần cứng chính
| Thành phần | Vai trò |
|-------------|----------|
| ESP32 | Vi điều khiển trung tâm |
| RFID-RC522 | Xác thực bằng thẻ từ |
| Keypad 4x4 | Nhập mật khẩu |
| Servo SG90 | Chốt khóa cửa |
| Cảm biến IR | Phát hiện trạng thái cửa |
| OLED 0.96" | Hiển thị thông tin |
| LED & Buzzer | Báo trạng thái và cảnh báo |
| Mạch YX850 | Chuyển nguồn tự động |

---

### 🖼️ Mô tả tổng thể
Hệ thống được thiết kế theo cấu trúc **modular**, đảm bảo tính mở rộng, dễ bảo trì và có thể tích hợp thêm các công nghệ như **nhận diện khuôn mặt**, **bảo mật OTP**, hoặc **kết nối MQTT** trong tương lai.

---

