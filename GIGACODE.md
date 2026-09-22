# G474VC — Цифровой осциллограф на базе STM32G474

## Обзор

**G474VC** — встроенное приложение-осциллограф на микроконтроллере **STM32G474VCT6** (LQFP100).  
Приложение обеспечивает **2 канала входа** и **1 канал генератора** сигналов на основе DAC.

- **Ядро:** ARM Cortex-M4 @ 170 МГц с FPU (single-precision)
- **Среда сборки:** CMake + GCC (arm-none-eabi)
- **Инструмент конфигурации** STM32CubeMX
- **Библиотеки:** STM32 HAL/LL, CMSIS, ARM DSP

---

## Аппаратная платформа

| Компонент | Чип / Модуль | Примечание |
|-----------|-------------|------------|
| MCU | STM32G474VCT6 | LQFP100, 170 МГц |
| Дисплей | TFT 480×320 | Интерфейс FMC (16-bit) |
| Тачскрин | FT6x36 (capacitive) | I2C1 (PB9/PA15) |
| Flash (внешняя) | Winbond W25Qxx (16 MB) | QSPI, режим QPI |
| USB | USB_DM/DP (PA11/PA12) | — |
| SWD | PA13/PA14 | Отладка |
| UART | USART2 (PA2/PA3) | 115200 бод, debug trace |

---

## Аналоговый подсистема

### ADC (5 модулей, 2 независимых группы)

| ADC | Канал | Пин | Разрешение | Режим | DMA |
|-----|-------|-----|------------|-------|-----|
| ADC1+ADC2 | IN1 | PA0 | 8 бит | Dual Interlaced | DMA2_Ch1 |
| ADC3+ADC4 | IN1 / IN3 | PB1 / PB12 | 8 бит | Dual Interlaced | DMA1_Ch3 |
| ADC5 | IN8, IN9 | PD11, PD12 | 12 бит | Single | — |

- ADC1/2 и ADC3/4 работают в **двойном интерлированном режиме** (`ADC_DUALMODE_INTERL`)
- Частота ADC-такта: **113.3 МГц** (от PLL, P делитель ÷3)
- Время выборки: 2.5 такта
- DMA: режим Normal, полу-словное выравнивание

### DAC (1 модуль)

| DAC | Канал | Пин | Триггер | DMA |
|-----|-------|-----|---------|-----|
| DAC1 | OUT1 | PA4 | TIM4 TRGO | DMA1_Ch2 (Circular) |

- Генерация сигналов (синусоиды и др.) через DMA circular buffer
- Триггер от TIM4 TRGO для синхронизации

---

## Таймеры

| Таймер | Назначение | Режим | Каналы | Примечание |
|--------|-----------|-------|--------|------------|
| TIM1 | Генератор / тактирование | PWM1 | CH1 (PC0) | PSC=99, ARR=99, CCR=40 |
| TIM4 | Триггер DAC / PWM | PWM1 | CH1 (PB6) | PSC=10, ARR=5, TRGO=UPDATE |
| TIM5 | Энкодер ENC1 | Encoder Mode X4 | CH1/CH2 (PB2/PC12) | ARR=255 |
| TIM8 | Энкодер ENC2 | Encoder Mode X4 | CH1/CH2 (PC6/PC7) | ARR=255 |
| TIM15 | PWM | PWM1 | CH1/CH2 (PB14/PB15) | PSC=169, ARR=99 |

---

## Интерфейсы и периферия

### FMC — LCD дисплей

- **Интерфейс:** 16-bit parallel (FSMC/FMC)
- **Пины данных:** PD0-1, PD8-15, PE7-15 (D0-D15)
- **Управление:**
  - FMC_NE1 → PD7 (LCD CS)
  - FMC_NWE → PD5 (WR)
  - FMC_NOE → PD4 (RD)
  - FMC_A18 → PD13 (RS/DC)
- **Дисплей:** 480×320 пикселей, 16-bit RGB
- **Цвета:** WHITE, BLACK, BLUE, RED, GREEN, CYAN, YELLOW, MAGENTA и др.

### QSPI — Внешняя Flash (Winbond W25Qxx)

- **Чип:** Winbond W25Qxx (16 МБайт, 128 Мбит)
- **Пины:** PF10 (CLK), PD3 (CS), PC1-4 (IO0-IO3)
- **Режимы:**
  - Indirect Write/Read
  - Auto Polling
  - Memory-Mapped (executing code from Flash)
  - QPI (Quad Peripheral Interface)
- **Команды:** Read (0x0B), QPI Enter (0x38), Page Program (0x32), Sector Erase (0x20) и др.

### I2C1 — Тачскрин FT6x36

- **Пины:** PA15 (SCL), PB9 (SDA)
- **Режим:** Fast Mode
- **Протокол:** Capacitive Touch Controller (FT6x36)

### USART2 — Debug Trace

- **Пины:** PA2 (TX), PA3 (RX)
- **Параметры:** 115200 бод, 8N1
- **DMA:** DMA1_Ch1 (TX)
- **Режимы отладки:**
  - `DEBUG_TRACE_UART` — через USART2
  - `DEBUG_TRACE_SWO` — через ITM (SWO)
  - `DEBUG_TRACE_NONE` — отключено

### GPIO — Кнопки и индикаторы

| Сигнал | Пин | Направление | Подтяжка |
|--------|-----|-------------|----------|
| SW2 | PE0 | Input | Pull-Up |
| SW3 | PE2 | Input | Pull-Up |
| SW4 | PE3 | Input | Pull-Up |
| SW5 | PE5 | Input | Pull-Up |
| BTN1 | PC13 | Input | Pull-Up |
| ENC1 | PE1 | Input | Pull-Up |
| ENC2 | PE4 | Input | Pull-Up |
| CHRG | PC10 | Input | — |
| LED1 | PB0 | Output | — |
| LED2 | PB7 | Output | — |
| BC, BL, B0-B2, AC, AL, A0-A2 | Various | Output | — |
| PEN (Touch) | PC11 | Output | — |

---

## Архитектура ПО

```
main()
  ├── SystemClock_Config()     → 170 МГц (HSE 8 МГц → PLL)
  ├── MX_GPIO_Init()
  ├── MX_DMA_Init()
  ├── MX_TIM1/4/5/8/15_Init()
  ├── MX_ADC1/2/3/4/5_Init()
  ├── MX_DAC1_Init()
  ├── MX_I2C1_Init()
  ├── MX_USART2_Init()
  │
  ├── mainInitialize()
  │   ├── LCD_Init()
  │   ├── KEYS_init()
  │   ├── ADC_start()
  │   ├── GEN_setParams()
  │   ├── DAC_startSin()
  │   ├── EEPROM_Test()
  │   └── QSPI_Init() + QPI/MemoryMapped
  │
  └── while(1)
      └── mainCycle()
          ├── getPoint() (touch)
          ├── KEYS_scan()
          ├── ENC1_Get() / ENC2_Get()
          ├── drawScreen() / drawFrame()
          └── display metrics (LCD_ShowxNum)
```

### Ключевые модули

| Модуль | Описание |
|--------|----------|
| `_main.c` | Точка входа, инициализация, главный цикл, debug trace |
| `adc.c` | Запуск ADC, чтение данных, тайминги измерений |
| `dac.c` | Настройка DAC, запуск генерации синуса через DMA |
| `generator.c` | Параметры генератора (частота, амплитуда) |
| `lcd_fmc.c` | Инициализация и управление LCD через FMC |
| `lcd.c` | Низкоуровневые функции отрисовки (точки, линии, текст) |
| `draw.c` | Высокоуровневая отрисовка (фрейм, экран осциллографа) |
| `keys.c` | Сканирование кнопок, чтение энкодеров |
| `ft6x36.c` | Тачскрин: получение координат касаний |
| `qspi.c` | Управление внешней Flash (чтение/запись/QPI) |
| `eeprom.c` | Эмуляция EEPROM / работа с Flash |
| `delay.c` | Задержки на базе DWT cycle counter |
| `graph.c` | Графические примитивы |

---

## Тактирование

```
HSE 8 МГц
  └── PLL
       ├── SYSCLK = 170 МГц (PLLN=85, PLLR=÷2)
       ├── ADC_CLK = 113.3 МГц (PLLP=÷3)
       └── USB_CLK = 48 МГц (HSI48)

AHB = 170 МГц (÷1)
APB1 = 170 МГц (÷1)
APB2 = 170 МГц (÷1)
```

---

## Сборка

### Требования

- **Toolchain:** ARM GCC (arm-none-eabi-gcc)
- **CMake:** ≥ 4.3
- **Стандарты:** C11, C++17

### Команды

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Выходные файлы

| Файл | Описание |
|------|----------|
| `g474vc.elf` | ELF-бинарник |
| `g474vc.hex` | Intel HEX |
| `g474vc.bin` | Raw binary |
| `g474vc.map` | Карта памяти (с использованием секций) |

### Оптимизация

- **Release:** `-Ofast`
- **Debug:** `-Og -g`
- **FPU:** Hardware (fpv4-sp-d16, hard float)
- **Кэш:** Instruction/Data cache disabled

---

## Память

| Область | Адрес | Размер |
|---------|-------|--------|
| Flash (код) | 0x08000000 | 256 КБ (внутр.) + 16 МБ (QSPI) |
| SRAM | 0x20000000 | 320 КБ |
| Stack | — | 0x400 (1 КБ) |
| Heap | — | 0x200 (512 Б) |

---

## Назначение пинов (ключевые)

```
PA0  → ADC1_IN1 / ADC2_IN1     (аналоговый вход)
PA4  → DAC1_OUT1               (генератор)
PA2  → USART2_TX               (debug)
PA3  → USART2_RX
PA15 → I2C1_SCL
PB9  → I2C1_SDA
PB6  → TIM4_CH1
PB0  → LED1
PB7  → LED2
PC0  → TIM1_CH1
PC6  → TIM8_CH1 (ENC2)
PC7  → TIM8_CH2 (ENC2)
PB2  → TIM5_CH1 (ENC1)
PC12 → TIM5_CH2 (ENC1)
PF10 → QSPI_CLK
PD3  → QSPI_NCS
PC1-4→ QSPI_IO0-IO3
PD0-15, PE7-15 → LCD Data (FMC)
PD7  → LCD CS (FMC_NE1)
PD5  → LCD WR (FMC_NWE)
PD4  → LCD RD (FMC_NOE)
PD13 → LCD RS (FMC_A18)
```

---

## Особенности реализации

1. **Dual Interleaved ADC** — ADC1/2 и ADC3/4 работают в интерлированном режиме для увеличения частоты дискретизации
2. **DMA для ADC/DAC** — всё перемещение данных происходит без участия CPU
3. **QPI Mode** — внешняя Flash работает в quad-PIN режиме для максимальной скорости
4. **Memory-Mapped QSPI** — выполнение кода напрямую из внешней Flash
5. **Encoder Interface** — TIM5 и TIM8 работают в режиме X4 для точного считывания энкодеров
6. **DWT Delay** — точные задержки на базе cycle counter Cortex-M4
7. **Multi-mode Debug** — поддержка UART, SWO (ITM) и silent режимов через define

---

## Файловая структура

```
g474vc/
├── Core/
│   ├── Src/main.c              → Точка входа, периферийная инициализация
│   └── Startup/startup_stm32g474vctx.s
├── usr/
│   ├── src/
│   │   ├── _main.c             → mainInitialize/mainCycle, логика
│   │   ├── adc.c/dac.c         → Аналоговая подсистема
│   │   ├── generator.c         → Генератор сигналов
│   │   ├── lcd.c/lcd_fmc.c     → LCD дисплей
│   │   ├── draw.c/graph.c      → Отрисовка UI
│   │   ├── keys.c              → Кнопки и энкодеры
│   │   ├── ft6x36.c            → Тачскрин
│   │   ├── qspi.c              → Внешняя Flash
│   │   ├── eeprom.c            → EEPROM
│   │   └── delay.c/dwt.c       → Задержки
│   └── inc/                    → Заголовочные файлы
├── Drivers/
│   ├── CMSIS/
│   └── STM32G4xx_HAL_Driver/
├── CMakeLists.txt
├── STM32G474VCTX_FLASH.ld      → Linker script
└── g474vc.ioc                  → STM32CubeMX config
```
