# KIVO One V1 — Project Structure & Iteration 1 Guide

## 1. Mục đích

Tài liệu này giúp hiểu:

- Một project IoT dùng ESP-IDF được chia thành những tầng nào.
- Mỗi thư mục và file trong KIVO làm gì.
- Code bắt đầu từ đâu, gọi qua đâu, xuống phần cứng như thế nào.
- Quy trình phát triển và hoàn thành Iteration 1.
- Nên lưu ghi chú kỹ thuật ở Git hay Notion.

## 2. Tư duy project IoT chuyên nghiệp

Một project IoT không chỉ có `main.c`. Nó còn gồm firmware, pin map, wiring, driver, diagnostics, test, build/flash, tài liệu và lịch sử thay đổi.

Nguyên tắc chính:

> Mỗi module chỉ nên có một trách nhiệm rõ ràng.

Ví dụ:

- `kivo_board`: board, GPIO, pin.
- `kivo_display`: màn hình.
- `kivo_ui`: logic giao diện.
- `kivo_diagnostics`: sức khỏe hệ thống.
- `app_main.c`: điều phối khởi động.

## 3. Kiến trúc 5 tầng

```text
Application
    ↓
Services / Features
    ↓
Hardware Abstraction
    ↓
ESP-IDF Drivers
    ↓
Physical Hardware
```

Áp vào KIVO:

```text
main/app_main.c
    ↓
kivo_ui / kivo_diagnostics
    ↓
kivo_display / kivo_board
    ↓
ESP-IDF SPI, GPIO, FreeRTOS, esp_lcd
    ↓
ESP32-S3 + ST7789
```

### Application Layer

`main/app_main.c` là điểm bắt đầu. Nó chỉ nên:

```text
Boot
→ Board init
→ Diagnostics
→ Display init
→ UI init
→ Start task/scheduler
```

Không nhét SPI, vẽ pixel, micro, motor hoặc AI vào đây.

### Service Layer

Iteration 1:

- `kivo_ui`
- `kivo_diagnostics`

Sau này có thể thêm:

- `kivo_wifi`
- `kivo_audio`
- `kivo_ai`
- `kivo_bluetooth`
- `kivo_motion`
- `kivo_power`

### Hardware Abstraction Layer

- `kivo_board`
- `kivo_display`

UI chỉ gọi API như:

```c
kivo_display_draw_text(...);
```

UI không biết MOSI là GPIO nào hay ST7789 cần command gì.

### ESP-IDF Driver Layer

Ví dụ:

```text
driver/gpio.h
driver/spi_master.h
esp_lcd_panel_io.h
esp_lcd_panel_vendor.h
freertos/FreeRTOS.h
freertos/task.h
esp_log.h
```

### Physical Hardware Layer

Iteration 1 chỉ gồm:

```text
ESP32-S3 N16R8
ST7789 240×240
Breadboard
Jumper
USB data cable
```

## 4. Cấu trúc project

```text
kivo-firmware/
├── .vscode/
├── build/
├── components/
│   ├── kivo_board/
│   │   ├── CMakeLists.txt
│   │   ├── board_init.c
│   │   └── include/
│   │       ├── board_init.h
│   │       └── pin_config.h
│   ├── kivo_display/
│   │   ├── CMakeLists.txt
│   │   ├── display_service.c
│   │   └── include/
│   │       └── display_service.h
│   ├── kivo_ui/
│   │   ├── CMakeLists.txt
│   │   ├── ui_service.c
│   │   └── include/
│   │       └── ui_service.h
│   └── kivo_diagnostics/
│       ├── CMakeLists.txt
│       ├── diagnostics.c
│       └── include/
│           └── diagnostics.h
├── docs/
│   └── iteration-01/
│       ├── acceptance-tests.md
│       ├── boot-log-sample.txt
│       ├── pin-map.md
│       └── wiring-table.md
├── main/
│   ├── CMakeLists.txt
│   ├── app_main.c
│   └── include/
│       └── app_version.h
├── .clangd
├── CMakeLists.txt
├── diagram.json
├── sdkconfig
└── wokwi.toml
```

## 5. Nhiệm vụ từng file

### `main/app_main.c`

Điểm bắt đầu firmware. Chỉ điều phối:

```text
kivo_board_init()
→ kivo_diagnostics_run()
→ kivo_display_init()
→ kivo_ui_init()
→ create tasks
```

### `main/include/app_version.h`

Chứa:

```c
#define KIVO_FIRMWARE_NAME "KIVO One"
#define KIVO_FIRMWARE_VERSION "0.1.0-i1"
#define KIVO_HARDWARE_VERSION "V1"
```

Dùng cho boot log, diagnostics, OTA sau này.

### `kivo_board/include/pin_config.h`

Chứa tập trung:

- MOSI
- SCLK
- CS
- DC
- RST
- BL
- SPI host
- SPI clock
- Screen width/height

Không để số GPIO rải rác trong nhiều file.

### `kivo_board/include/board_init.h`

Public API:

```c
esp_err_t kivo_board_init(void);
bool kivo_board_is_ready(void);
```

### `kivo_board/board_init.c`

Implementation:

- GPIO init.
- Trạng thái backlight ban đầu.
- Kiểm tra board.
- Trả lỗi bằng `esp_err_t`.

### `kivo_display/include/display_service.h`

Public API:

```c
esp_err_t kivo_display_init(void);
void kivo_display_clear(uint16_t color);
void kivo_display_draw_text(int x, int y, const char *text);
bool kivo_display_is_ready(void);
```

### `kivo_display/display_service.c`

Implementation:

- SPI bus.
- ST7789 init.
- Reset.
- Rotation.
- RGB/BGR.
- Backlight.
- Vẽ màu/text.

Luồng:

```text
kivo_ui
→ display_service.h
→ display_service.c
→ ESP-IDF SPI/esp_lcd
→ ST7789
```

### `kivo_ui/include/ui_service.h`

Public API:

```c
esp_err_t kivo_ui_init(void);
void kivo_ui_update(void);
void kivo_ui_set_screen(...);
void kivo_ui_set_state(...);
```

### `kivo_ui/ui_service.c`

Quản lý:

- BOOT screen.
- DIAGNOSTICS screen.
- FACE placeholder.
- Screen hiện tại.
- Frame scheduler.
- State IDLE/LISTENING/THINKING/SPEAKING.

Nguyên tắc:

```text
kivo_ui quyết định vẽ cái gì
kivo_display quyết định vẽ bằng cách nào
```

### `kivo_diagnostics/include/diagnostics.h`

Khai báo:

- Report struct.
- OK/WARNING/FATAL.
- Run diagnostics.
- Print report.

### `kivo_diagnostics/diagnostics.c`

Kiểm tra:

- Chip.
- Flash.
- PSRAM.
- Free heap.
- Minimum heap.
- Reset reason.
- Display status.
- Uptime.

Ví dụ:

```text
Flash lỗi → FATAL
Sai chip → FATAL
Display init fail → FATAL trong Iteration 1
PSRAM chưa bật → WARNING
```

## 6. `.h` và `.c`

```text
.h = component cung cấp được gì
.c = component thực hiện bằng cách nào
```

Header là hợp đồng. Source là implementation.

## 7. `CMakeLists.txt`

Ví dụ:

```cmake
idf_component_register(
    SRCS
        "display_service.c"
    INCLUDE_DIRS
        "include"
    REQUIRES
        kivo_board
    PRIV_REQUIRES
        driver
        esp_lcd
)
```

Ý nghĩa:

- `SRCS`: file compile.
- `INCLUDE_DIRS`: header công khai.
- `REQUIRES`: dependency công khai.
- `PRIV_REQUIRES`: dependency chỉ dùng trong `.c`.

Quy tắc:

```text
Header cần dependency → REQUIRES
Chỉ implementation cần → PRIV_REQUIRES
```

## 8. Dependency flow

```text
app_main
├── kivo_board
├── kivo_diagnostics
├── kivo_display
└── kivo_ui
     └── kivo_display

kivo_display
└── kivo_board
```

Không tạo vòng:

```text
kivo_ui → kivo_display
kivo_display → kivo_ui
```

Đó là circular dependency.

## 9. Luồng khởi động

```text
ESP32-S3 reset
→ bootloader
→ partition table
→ firmware image
→ FreeRTOS
→ app_main()
→ board init
→ diagnostics
→ display init
→ UI init
→ UI task
→ screen update
```

Luồng render:

```text
UI scheduler
→ kivo_ui_update()
→ render current screen
→ kivo_display_draw_xxx()
→ SPI/esp_lcd
→ ST7789
```

## 10. Quy trình phát triển

Không viết hết rồi mới build.

Dùng:

```text
Thay đổi nhỏ
→ Build
→ Chạy
→ Xem log
→ Test
→ Commit
```

### Milestone I1.1 — Structure

- Tạo folder/file.
- Sửa CMake.
- Build project rỗng.

### Milestone I1.2 — Board

- `pin_config.h`
- `board_init.h/.c`
- Build.

### Milestone I1.3 — Diagnostics

- Chip.
- Flash.
- PSRAM.
- Heap.
- Reset reason.
- Heartbeat.

### Milestone I1.4 — Mock display

- Public display API.
- Chưa dùng ST7789 thật.
- Log thao tác render ra serial.

### Milestone I1.5 — UI scheduler

- FreeRTOS task.
- BOOT.
- DIAGNOSTICS.
- FACE_PLACEHOLDER.

### Milestone I1.6 — Wokwi

- ESP32-S3 simulation.
- Serial log.
- State transition.
- Long-run simulation.

### Milestone I1.7 — ST7789 thật

- Xác minh pin.
- Kiểm tra chập.
- Nối 3.3V/GND.
- Nối SPI.
- Color test.
- Rotation.
- RGB/BGR.
- Offset.

### Milestone I1.8 — Acceptance

T1-01:

- 20 cold boots.
- 20/20 hiển thị.

T1-02:

- Red/Green/Blue/White/Black.
- Checkerboard.
- Text 4 góc.
- Mũi tên UP.

T1-03:

- Chạy 30 phút.
- Không watchdog reset.
- Không blank screen.
- Heap không giảm liên tục.
- UI vẫn chạy.

## 11. Build, flash, monitor

```powershell
idf.py build
idf.py fullclean
idf.py set-target esp32s3
idf.py -p COMx flash
idf.py -p COMx monitor
idf.py -p COMx flash monitor
```

Thoát monitor:

```text
Ctrl + ]
```

## 12. Git workflow

Commit theo mốc:

```text
chore: create iteration 1 project structure
build: register KIVO components in ESP-IDF
feat(board): add centralized pin configuration
feat(diag): add boot diagnostics and heartbeat
feat(display): add display abstraction and mock backend
feat(ui): add UI scheduler and placeholder screens
test(wokwi): add ESP32-S3 simulation
feat(display): add ST7789 driver
docs(i1): add wiring and acceptance evidence
```

Không commit:

- `build/`
- `sdkconfig.old`
- log tạm.
- API key.
- Wi-Fi password.
- token.

## 13. Nên note ở đâu?

### Git repository = nguồn sự thật kỹ thuật

Lưu trong `docs/`:

- Architecture.
- Project structure.
- Pin map.
- Wiring.
- Build/flash.
- Test plan.
- Boot log.
- Troubleshooting.
- Technical decisions.

Đề xuất:

```text
docs/
├── 00-project-overview.md
├── 01-project-structure.md
├── 02-architecture-overview.md
├── 03-development-workflow.md
├── 04-coding-conventions.md
├── adr/
│   ├── ADR-001-use-esp-idf.md
│   ├── ADR-002-use-two-esp32.md
│   └── ADR-003-display-abstraction.md
└── iterations/
    └── iteration-01/
        ├── iteration-plan.md
        ├── pin-map.md
        ├── wiring-table.md
        ├── acceptance-tests.md
        ├── boot-log-sample.txt
        └── evidence/
```

### Notion = quản lý và tư duy

Lưu:

- Roadmap.
- Backlog.
- Task.
- Research.
- Ý tưởng.
- Chi phí.
- Shopping.
- Nhật ký.
- Risk.
- Meeting note.

Cấu trúc:

```text
KIVO One
├── Product Vision
├── Roadmap
├── Iterations
├── Engineering Backlog
├── Hardware BOM
├── Purchases
├── Research
├── Risks & Issues
├── Decisions
├── Test Results
└── Development Journal
```

### Code comment = giải thích cục bộ

Comment nên nói “vì sao”, không nói lại code.

Tốt:

```c
// Keep SPI at 20 MHz during breadboard bring-up to reduce signal integrity issues.
```

## 14. Quy tắc chọn nơi lưu

```text
Cần để build/test lại sản phẩm → Git docs
Quản lý công việc, ý tưởng → Notion
Giải thích sát đoạn code → Code comments
```

## 15. Chỉnh cấu trúc hiện tại

1. Đổi:

```text
main/include/app.version.h
```

thành:

```text
main/include/app_version.h

```

2. Đổi:

```text
docs/iter-01
```

thành:

```text
docs/iteration-01
```

1. `build/` không sửa tay và không commit.

2. `.clangd` có thể giữ.

3. `sdkconfig` giữ sau khi cấu hình ổn.

4. `.gitignore` nên có:

```gitignore
build/
sdkconfig.old
*.log
.vscode/.browse.VC.db*
```

7. Không lưu secret trong source.

## 16. Exit criteria Iteration 1

Code:
- Build được.
- Component rõ.
- Board init.
- Diagnostics.
- Display abstraction.
- ST7789 driver.
- UI scheduler.
- Ba screen.
- Fatal/non-fatal status.

Tài liệu:

- Pin map.
- Wiring.
- Boot log.
- Test plan.
- Evidence.

Test:

- 20 cold boots.
- Color/orientation pass.
- 30-minute stability.
- Không watchdog.
- Flash lặp lại được.

## 17. Câu hỏi tự kiểm tra

1. `app_main.c` làm gì?
2. Vì sao không để SPI trong `kivo_ui`?
3. `.h` và `.c` khác nhau thế nào?
4. `REQUIRES` và `PRIV_REQUIRES` khác gì?
5. Vì sao cần `pin_config.h`?
6. Vì sao mock display trước?
7. Luồng UI đến ST7789 qua file nào?
8. Display init fail được báo về đâu?
9. Wiring lưu Git hay Notion?
10. Vì sao build sau thay đổi nhỏ?
11. `build/` có commit không?
12. Exit criteria Iteration 1 là gì?
