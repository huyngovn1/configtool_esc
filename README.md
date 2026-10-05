# ESC Config

Ứng dụng cấu hình, sao lưu và cập nhật firmware cho ESC dựa trên AM32.

![ESC Config](docs/esc_config.png)

## Giới thiệu

**ESC Config** là công cụ dành cho Windows giúp kết nối và làm việc với ESC thông qua cổng Serial.

Ứng dụng hỗ trợ cấu hình thông số ESC, đọc/ghi EEPROM và cập nhật firmware trực tiếp từ giao diện đồ họa.

## Tính năng

- Kết nối ESC qua cổng COM.
- Hỗ trợ USB-UART như CH340, CP2102.
- Giao tiếp Direct Bootloader ở `19200 baud`.
- Tự động quét cổng Serial.
- Đọc thông tin thiết bị, firmware và EEPROM.
- Đọc và chỉnh sửa thông số ESC.
- Ghi cấu hình trở lại ESC.
- Sao lưu dữ liệu ESC.
- Đọc và ghi dữ liệu EEPROM.
- Cập nhật firmware ESC.
- Theo dõi quá trình hoạt động qua Log.
- Giao diện trực quan, dễ sử dụng.

## Các nhóm cấu hình

Ứng dụng cho phép điều chỉnh nhiều thông số của ESC, bao gồm:

- PWM Frequency
- Motor Timing
- Motor Poles
- Motor KV
- Startup Power
- Startup Speed
- Brake
- Drag Brake
- Running Brake
- Sine Startup
- Control Signal
- Protection Settings

## Yêu cầu

- Windows 10 / Windows 11
- Hệ điều hành 64-bit
- USB-UART CH340 / CP2102 hoặc thiết bị tương thích
- ESC sử dụng firmware/bootloader tương thích

## Cài đặt

Không cần cài đặt.

1. Vào mục **Releases**.
2. Tải phiên bản mới nhất.
3. Giải nén toàn bộ file ZIP.
4. Chạy:

```text
BasicEscConfig.exe
