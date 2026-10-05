# Basic ESC Config — bản cơ bản 0.1

Dự án Qt Widgets C++ riêng để bạn tự xây dựng configtool. Giao diện mới nằm trong `src/mainwindow.ui`, sửa bằng chế độ Design của Qt Creator. Hai lớp đóng gói lệnh bootloader và 4way được tách từ bốn file bạn cung cấp; phần cửa sổ, quản lý kết nối và mô hình cấu hình được viết riêng.

## Mở và chạy trong Qt Creator

1. Chọn **File → Open File or Project**, mở `BasicEscConfig.pro`.
2. Chọn kit **Desktop Qt 5.15.2 MinGW 64-bit** đã có trên máy.
3. Chọn **Configure Project**, rồi **Build** và **Run**.
4. Bấm **Xem thử giao diện** để chỉnh dữ liệu mẫu khi chưa cắm ESC.
5. Mở **Forms → src/mainwindow.ui** và chọn **Design** để kéo thả, đổi bố cục hoặc tên hiển thị.

Dự án cũng có `CMakeLists.txt`. Chỉ mở một trong hai file dự án cho mỗi thư mục build. Các module cần dùng: **Core, Gui, Widgets, SerialPort**, C++17. Đã kiểm tra với Qt 5.15.2/MinGW 8.1; chưa kiểm tra bằng Qt 6.

## Chạy bản Windows đã dựng

Giải nén toàn bộ `BasicEscConfig-Windows.zip`, rồi mở `BasicEscConfig.exe`. Giữ các DLL và thư mục `platforms` cạnh file EXE. Chế độ xem thử không mở cổng COM và không gửi lệnh tới ESC.

## Chức năng hiện có

- Quét cổng COM, kết nối một ESC, nhận diện và đọc cấu hình.
- Direct Bootloader: 19200 baud, 8N1; dùng adapter phù hợp với bootloader đang có.
- 4way qua FC: 115200 baud, bật passthrough qua MSP; chọn kênh ESC từ 1 đến 8.
- Adapter đã ở chế độ 4way: 115200 baud, bỏ qua bước MSP của FC.
- Chỉnh PWM, công suất khởi động, số cực, đảo chiều, hai chiều, sine startup, phanh và âm lượng bíp.
- Đọc/ghi 48 byte cấu hình. Chỉ báo ghi thành công sau khi đọc lại đủ 48 byte và so khớp.
- Hoàn tác thay đổi, xem dữ liệu hex và nhật ký.
- **Sao lưu bản đã đọc** xuất 48 byte đã đọc gần nhất thành `.bin`, không xuất những sửa đổi còn chưa ghi. Trong chế độ xem thử, file có tiền tố `DEMO`.

Luồng sử dụng: **chọn COM và giao tiếp → Kết nối → Sao lưu bản đã đọc → chỉnh thông số → Ghi ESC**. Khi kết nối thành công, ứng dụng tự đọc cấu hình; nút **Đọc ESC** dùng để đọc lại. Chỉ một phần cấu hình được đưa ra giao diện ở bản đầu.

## Phạm vi bản đầu

Đã biên dịch và kiểm tra phần mềm; **chưa thử đọc/ghi với ESC thật**. Cần đối chiếu lần đầu trên bộ ESC/adapter thực tế của bạn. Chế độ mẫu là dữ liệu tổng hợp để thử giao diện, không phải bộ thông số mặc định của ESC.

Bản này chưa có nạp firmware, điều khiển ga/motor, phục hồi từ file hay các mục cấu hình nâng cao. File `.bin` sao lưu chỉ chứa vùng cấu hình 48 byte, không phải toàn bộ firmware/flash. Những chức năng đó có thể thêm sau khi xác nhận phần kết nối cơ bản.

Ứng dụng chỉ cho chỉnh khi vùng cấu hình có marker `1`, phiên bản EEPROM `1–3`, và tám thông số hiển thị có giá trị được nhận biết. Những byte khác được giữ nguyên theo bản đọc. Nếu cấu trúc hoặc giá trị chưa hỗ trợ, chỉ xem và sao lưu dữ liệu; không tự tạo lại thông số.

Các mã flash được nhận diện trong phiên bản này:

| Mã | Nhóm profile trong nguồn tham chiếu | Địa chỉ EEPROM trong giao thức |
|---|---|---|
| `0x1F` | F051/F053 | `0x7C00` |
| `0x2B` | G071, địa chỉ chia 4 | `0x7E00` (offset flash `0x1F800`) |
| `0x35` | F3 | `0xF800` |
| `0x15` | NXP, chỉ qua 4way | `0xE000` |

Đây là danh sách profile trong mã nguồn, chưa phải danh sách phần cứng đã thử. Mã khác sẽ báo chưa hỗ trợ và không truy cập EEPROM.

## Sửa phần nào khi phát triển tiếp

| File | Vai trò |
|---|---|
| `src/mainwindow.ui` | Bố cục và widget; bắt đầu sửa giao diện ở đây |
| `src/mainwindow.cpp` | Nối các nút, hiển thị dữ liệu, trạng thái và chế độ mẫu |
| `src/model/settingsmodel.*` | Ánh xạ thông số với byte EEPROM và kiểm tra dữ liệu |
| `src/backend/escsession.*` | Cổng serial, handshake, đọc/ghi, CRC/ACK, timeout, đọc lại xác minh |
| `src/backend/serialframes.h` | Ghép phản hồi bị chia thành nhiều gói và xử lý echo |
| `src/backend/memoryaddress.h` | Tính địa chỉ theo đơn vị của từng bootloader |
| `src/protocol/BF_ROOTLOADER.*` | Tạo lệnh giao thức bootloader trực tiếp |
| `src/protocol/fourwayif.*` | Tạo lệnh giao thức 4way |
| `tests/` | Kiểm tra giao thức, dữ liệu, phản hồi và giao diện |

Giữ `objectName` của widget khi đổi bố cục. Nếu đổi `objectName`, sửa các chỗ sử dụng tương ứng trong `mainwindow.cpp`. Serial chạy trong worker thread để chờ phản hồi không chặn cửa sổ. Không sửa file `ui_mainwindow.h` trong thư mục build vì Qt tạo lại file đó.

Các byte được chỉnh: `17` đảo chiều, `18` hai chiều, `19` sine startup, `24` PWM, `25` công suất khởi động, `27` số cực, `28` kiểu phanh, `30` âm lượng. Các giới hạn trong mô hình là giới hạn biểu diễn của vùng cấu hình tham chiếu; chưa phải bộ preset khuyến nghị cho một motor cụ thể.

## Các sửa lỗi khi tách giao thức

Hoàn thiện include guard, trạng thái khởi tạo, hàm ACK và kiểm tra độ dài; bỏ phụ thuộc giao diện khỏi hai lớp giao thức. Sửa phép tính địa chỉ đọc phần tên firmware trước EEPROM trên G071: trừ 8 đơn vị địa chỉ để lùi 32 byte. Sửa lệnh Direct SET_BUFFER 256 byte dùng `FE 00 01 00` trước CRC. Ghi cấu hình hiện tại chỉ dùng 48 byte.

Nguồn gốc được ghi trong `src/protocol/PROVENANCE.md`. Tham khảo kỹ thuật: [Qt Designer UI files](https://doc.qt.io/qt-6/designer-using-a-ui-file.html), [AM32 bootloader](https://github.com/am32-firmware/AM32-bootloader/blob/master/bootloader/main.c), [Betaflight 4way](https://github.com/betaflight/betaflight/blob/master/src/main/io/serial_4way.c).

## Kiểm tra đã thực hiện

- Build Release bằng qmake `.pro` và CMake, Qt 5.15.2/MinGW 8.1 x64.
- `protocol_tests`: mẫu lệnh/CRC, dữ liệu sai hoặc thiếu, block 256 byte.
- `settings_tests`: giữ nguyên byte không chỉnh, các giới hạn và cấu trúc không hỗ trợ.
- `backend_tests`: phản hồi phân mảnh/echo, tính địa chỉ và nhả trạng thái bận khi đầu vào lỗi.
- `ui_smoke`: xem thử, chỉnh thông số, hoàn tác, áp dụng mẫu và thoát xem thử.

Các bài kiểm tra này không mở kết nối ESC thật. Chưa có kiểm tra tích hợp toàn bộ luồng serial với phần cứng hoặc thiết bị giả lập.
