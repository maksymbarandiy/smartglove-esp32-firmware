# SmartGlove ESP32 Firmware

Прошивка для ESP32, яка зчитує дані з трьох датчиків MPU6050 (акселерометр + гіроскоп) і передає їх через Bluetooth.

## Автор
**Барандій Максим** — Hardware / Embedded (C++)  
Курсова робота з дисципліни "Інтелектуальні системи"

## Функціонал
- Зчитування 18 осей даних (6 з кожного MPU6050: ax, ay, az, gx, gy, gz)
- Підтримка трьох датчиків на двох I2C шинах (Wire та Wire1)
- Буферизація до 1000 фреймів (18 × 1000 = 18000 значень)
- Стейт-машина: IDLE / RECORDING / SENDING
- Керування через Bluetooth команди: START, STOP, SEND
- Обробка помилок читання (заповнення нулями при збої)
- Частота дискретизації: 100 Гц (10 мс між вимірами)

## Апаратне забезпечення
| Компонент | Характеристики |
|-----------|----------------|
| Мікроконтролер | ESP32 (DevKit V1) |
| Датчики | 3 × MPU6050 (акселерометр + гіроскоп) |
| Зв'язок | Bluetooth Classic |

### Підключення датчиків
| Датчик | I2C шина | Адреса | SDA | SCL |
|--------|----------|--------|-----|-----|
| MPU1 | Wire | 0x68 | 21 | 22 |
| MPU2 | Wire | 0x69 | 21 | 22 |
| MPU3 | Wire1 | 0x68 | 18 | 19 |

**Примітка:** MPU3 підключений до окремої I2C шини Wire1 через конфлікт адрес (обидва MPU1 та MPU3 мають адресу 0x68 за замовчуванням).

## Команди Bluetooth
| Команда | Опис |
|---------|------|
| `START` | Почати запис даних в буфер |
| `STOP` | Зупинити запис |
| `SEND` | Відправити буфер через Bluetooth та очистити його |

## Формат вихідних даних
ax1,ay1,az1,gx1,gy1,gz1,ax2,ay2,az2,gx2,gy2,gz2,ax3,ay3,az3,gx3,gy3,gz3
ax1,ay1,az1,gx1,gy1,gz1,ax2,ay2,az2,gx2,gy2,gz2,ax3,ay3,az3,gx3,gy3,gz3
...
END
- Кожен рядок — один фрейм (18 значень)
- Маркер `END` сигналізує про кінець передачі

## Історія розробки (Git коміти)

| Дата | Коміт |
|------|-------|
| 29.04.2026 | `feat: initial MPU6050 read on Wire` |
| 29.04.2026 | `feat: extract readMPU6050() function, add error check` |
| 30.04.2026 | `feat: add second MPU6050 on address 0x69` |
| 02.05.2026 | `feat: add third MPU6050 on second I2C bus (Wire1)` |
| 04.05.2026 | `feat: add BluetoothSerial, send data via BT` |
| 05.05.2026 | `feat: add START/STOP command handling` |
| 06.05.2026 | `feat: implement data buffer (1000 frames x 18 axes)` |
| 07.05.2026 | `feat: add IDLE/RECORDING/SENDING state machine` |
| 08.05.2026 | `fix: fill zeros on MPU read failure instead of crashing` |
| 10.05.2026 | `refactor: extract log() helper, clean up debug output` |
| 14.05.2026 | `docs: add README.md with project description` |

## Збірка та прошивка

### Необхідне ПЗ
- Arduino IDE (версія 2.x) або PlatformIO
- Пакет підтримки ESP32 (через Boards Manager)
- Бібліотеки: Wire (вбудована), BluetoothSerial (вбудована)

### Інструкція
1. Відкрити `smartglove-esp32-firmware.ino` в Arduino IDE
2. Вибрати плату: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
3. Вибрати порт: **Tools → Port → (ваш COM-порт)**
4. Натиснути **Upload** (стрілка вправо)

### Монітор порту
- Швидкість: 115200 бод
- Для отримання даних через Bluetooth — підключитись до пристрою `SmartGlove` з будь-якого телефону/П