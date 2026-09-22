# AGENTS.md — Правила для AI-агентов при работе с проектом G474VC

## О проекте

**G474VC** — встроенное приложение-осциллограф на базе **STM32G474VCT6** (ARM Cortex-M4 @ 170 МГц).  
2 аналоговых входа + 1 генератор сигналов на основе DAC. TFT-дисплей 480×320, тачскрин FT6x36, QSPI Flash.

---

## Ключевые архитектурные принципы

### 1. HAL/LL — без HAL для периферии

Проект использует **STM32 LL (Low Layer)** драйверы напрямую для максимальной производительности.  
HAL используется только в минимальном объёме (только HAL_DMA, HAL_GPIO, HAL_DMA — остальные HAL-модули отключены в `stm32g4xx_hal_conf.h`).

**Правила:**
- Для ADC/TIM/DAC/I2C/USART/QSPI использовать **LL_** функции, а не HAL
- Не добавлять `#include "stm32g4xx_hal_adc.h"` и подобные — они отключены в conf.h
- Конфигурация периферии через `LL_AHB2_GRP1_EnableClock()`, `LL_APB1_GRP1_EnableClock()` и т.д.
- Структуры инициализации: `LL_ADC_InitTypeDef`, `LL_TIM_InitTypeDef`, и т.п.

### 2. DMA — обязательное использование

Вся передача данных между периферией и памятью — **исключительно через DMA**.
- ADC → DMA → Memory (двойные буферы `samplesBuffer0`, `samplesBuffer1`)
- Memory → DMA → DAC (Circular mode)
- Memory → DMA → USART2 (TX)

**Правила:**
- При работе с ADC/DAC/USART всегда учитывать DMA
- Не блокировать CPU в циклах чтения/записи периферии
- Использовать `LL_DMA_EnableIT_TC/TE` для прерываний DMA

### 3. Dual Interleaved ADC

ADC1+ADC2 и ADC3+ADC4 работают в **двойном интерлированном режиме** (`LL_ADC_MULTI_DUAL_REG_INTERL`).  
Это удваивает эффективную частоту дискретизации.

**Правила:**
- При конфигурации multimode всегда устанавливать `LL_ADC_MULTI_DUAL_REG_INTERL`
- Использовать `LL_ADC_MULTI_REG_DMA_UNLMT_RES8_6B` для DMA
- Общая регистровая шина: `__LL_ADC_COMMON_INSTANCE(ADCx)`

### 4. Main loop — init + cycle

Проект разделён на две фазы:
```c
mainInitialize();  // Однократная инициализация
while (1) {
    mainCycle();   // Бесконечный цикл
}
```

**Правила:**
- Новая логика инициализации — в `mainInitialize()` в `_main.c`
- Логика основного цикла — в `mainCycle()` в `_main.c`
- Не дублировать инициализацию в `main()` (Core/Src/main.c)

### 5. Типы данных

Проект использует собственные алиасы типов:
```c
typedef int32_t  s32;   typedef int16_t  s16;   typedef int8_t  s8;
typedef uint32_t u32;   typedef uint16_t u16;   typedef uint8_t u8;
typedef __IO uint32_t vu32;  typedef __IO uint16_t vu16;  typedef __IO uint8_t vu8;
```

**Правила:**
- Использовать `u8`, `u16`, `u32`, `s8`, `s16`, `s32` вместо стандартных
- Для регистров периферии — `vu8`, `vu16`, `vu32`
- Включать `_main.h` первым для доступа к типам

### 6. Отладочный вывод

Три режима (выбирается через define):
```c
#define DEBUG_TRACE_UART   // USART2 (PA2/PA3), 115200 бод, DMA
#define DEBUG_TRACE_SWO    // ITM/SWO (SWO_Trace)
#define DEBUG_TRACE_NONE   // Отключено
```

**Правила:**
- Использовать макрос `DBG_Trace(msg)` — он автоматически выбирает режим
- Не использовать `printf` напрямую
- `UART_Transmit()` использует DMA с блокировкой (`while(DMA1_0_busy){}`)

---

## Структура проекта

```
g474vc/
├── Core/
│   ├── Src/main.c              → Точка входа, HAL-инициализация периферии
│   └── Startup/                 → Startup файл STM32G474
├── usr/
│   ├── src/_main.c             → mainInitialize(), mainCycle(), debug trace
│   ├── src/adc.c               → ADC dual interleaved, DMA буферы
│   ├── src/dac.c               → DAC генератор, DMA circular
│   ├── src/generator.c         → Параметры генератора (TIM1 PWM)
│   ├── src/lcd.c               → Базовые функции LCD (точки, линии, текст)
│   ├── src/lcd_fmc.c           → Инициализация LCD через FMC
│   ├── src/draw.c              → Отрисовка UI (сетка, экран)
│   ├── src/graph.c             → Графические примитивы
│   ├── src/keys.c              → Кнопки, энкодеры (TIM5/TIM8 encoder mode)
│   ├── src/ft6x36.c            → Тачскрин через I2C1
│   ├── src/qspi.c              → Внешняя Flash Winbond (QPI, MM mode)
│   ├── src/eeprom.c            → EEPROM эмуляция
│   ├── src/dwt.c               → DWT cycle counter для таймингов
│   ├── src/DataBuffer.c        → Буферы ADC (samplesBuffer0/1)
│   └── inc/                    → Заголовочные файлы
├── Drivers/
│   ├── CMSIS/
│   └── STM32G4xx_HAL_Driver/
├── CMakeLists.txt              → CMake build system
├── STM32G474VCTX_FLASH.ld      → Linker script
└── g474vc.ioc                  → STM32CubeMX проект
```

---

## Стиль кода

### Форматирование

- **Отступы:** 4 пробела (без табуляций)
- **Фигурные скобки:** K&R (открывающая на той же строке)
- **Строка:** 120 символов максимум
- **Именование функций:** `ModuleName_ActionName` (camelCase внутри: `ADC_start`, `KEYS_scan`)
- **Именование переменных:** `camelCase` (локальные), `g_` или `ADC_`/`LCD_` (глобальные с префиксом модуля)
- **Макросы:** `UPPER_CASE`

### Комментарии

- Использовать `//` для однострочных
- `/* */` для многострочных и блоков
- Не дублировать то, что очевидно из названия функции
- Комментарии к сложной логике — на **англиийском**

### Include порядок

```c
#include "_main.h"       // Сначала собственные типы
#include <stdlib.h>      // Системные заголовочные
#include "module.h"      // Заголовочные проекта
#include "stm32_ll_xyz.h" // LL заголовочные
```

### Обработка ошибок

- Использовать `Error_Handler()` для критических ошибок
- `while(1){}` в `Error_Handler()` — halt на ошибку
- Проверять возвращаемые значения критических операций

---

## Критические ограничения

### Память

- **SRAM:** 320 КБ (всего)
- **Heap:** 512 байт (`0x200`)
- **Stack:** 1 КБ (`0x400`)
- **Не использовать `malloc`/`free`** — память статическая
- Буферы ADC — статические массивы (`samplesBuffer0`, `samplesBuffer1`)

### Компилятор

- **Стандарт:** C11 (C код), C++17 (если нужен C++)
- **FPU:** Hardware single-precision (fpv4-sp-d16)
- **Float-abi:** hard
- **Кэш:** Instruction и Data кэш **отключены**
- Оптимизация: Debug `-Og`, Release `-Ofast`

### Прерывания

- NVIC Priority Group: **4** (все приоритеты уникальны)
- SysTick priority: 15 (наивысший номер = наименьший приоритет)
- DMA приоритеты: VERY_HIGH для ADC, HIGH для DAC, LOW для USART

---

## Работа с периферией

### ADC

```c
// Ключевые переменные
extern uint8_t  ADCworks;         // 0 = не работает, 1 = работает
extern uint32_t ADCElapsedTick;   // время заполнения буфера
extern float    ADC_MeasureTime;  // время измерения в секундах
extern uint8_t  ADC_param;        // индекс параметров (0..62)

// Функции
ADC_start();    // Запустить ADC
ADC_step(step); // Изменить параметры (step = +1/-1)
```

- Два канала: `chParams[0]` (ADC1+ADC2, DMA2_Ch1), `chParams[1]` (ADC3+ADC4, DMA1_Ch3)
- Буферы: `samplesBuffer0`, `samplesBuffer1` (определяются в `DataBuffer.c`)
- 63 комбинации prescaler/sample time в `ADC_Parameters[]`

### DAC

```c
DAC_startSin();   // Запустить синусоиду
DAC_step(step);   // Изменить параметры
```

- TIM4 TRGO → триггер DAC
- DMA1_Ch2 circular → DAC1_OUT1 (PA4)

### LCD (FMC)

```c
LCD_Init();
LCD_Clear(color);
LCD_DrawPoint(x, y);
LCD_Fill(x1, y1, x2, y2, color);
LCD_ShowxNum(x, y, num, len, size, mode);
```

- Цвета: `WHITE`, `BLACK`, `BLUE`, `RED`, `GREEN`, `CYAN`, `YELLOW`, `MAGENTA`, `GRAY`, `DARKGRAY`
- Размер: 480×320 (`SIZE_X`, `SIZE_Y`)
- `POINT_COLOR` — текущий цвет рисования
- `BACK_COLOR` — текущий цвет фона

### Кнопки и энкодеры

```c
KEYS_init();
KEYS_scan();        // Вызывать в mainCycle
int16_t ENC1_Get(); // Вернуть значение энкодера 1
int16_t ENC2_Get(); // Вернуть значение энкодера 2

// Состояние кнопок
extern uint16_t btns_state;   // битовая маска
extern uint8_t  button1Count; // счётчик нажатий

#define BUTTON1 0x01  // SW3 (PE2)
#define BUTTON2 0x02  // SW2 (PE0)
#define BUTTON3 0x04  // SW5 (PE5)
#define BUTTON4 0x08  // SW4 (PE3)
#define BUTTON5 0x10  // ENC1 (PE1)
#define BUTTON6 0x20  // ENC2 (PE4)
```

### Тачскрин FT6x36

```c
typedef struct {
    u16 x;
    u16 y;
    u8 size;
    u8 id;
} touch_point_t;

getPoint(touch_id, &touchPoint);  // touch_id: 0 или 1
```

### QSPI

```c
QSPI_Init();
QSPI_Enable_QPI_Mode();
QSPI_ReadBlock(address, buffer, size);
QSPI_WritePage(address, data, size);
QSPI_EnableMemoryMappedMode();
```

- Flash: Winbond W25Qxx (16 МБайт)
- Команды определены в `qspi.h`
- Префиксы: `CMD_`, режимы: `QSPI_FUNCTIONAL_MODE_*`

---

## CMake Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

**Выходные файлы:**
- `build/g474vc.elf` — ELF
- `build/g474vc.hex` — Intel HEX
- `build/g474vc.bin` — Raw binary
- `build/g474vc.map` — Memory map (с секциями)

**Критические флаги:**
- `-mcpu=cortex-m4 -mthumb -mthumb-interwork`
- `-mfloat-abi=hard -mfpu=fpv4-sp-d16`
- `-ffunction-sections -fdata-sections`
- `-DSTM32G474xx -DUSE_FULL_LL_DRIVER`

---

## Чего НЕ делать

1. **НЕ включать HAL-модули** которые отключены в `stm32g4xx_hal_conf.h` (ADC, TIM, DAC, I2C, USART, QSPI)
2. **НЕ использовать `malloc`/`free`** — нет динамического выделения
3. **НЕ включать кэш** — он отключен на уровне конфигурации
4. **НЕ блокировать CPU** в циклах чтения ADC/DAC — использовать DMA
5. **НЕ изменять `stm32g4xx_hal_conf.h`** без крайней необходимости
6. **НЕ использовать `printf`** напрямую — использовать `DBG_Trace()`
7. **НЕ дублировать инициализацию** в `main()` и `mainInitialize()`
8. **НЕ изменять linker script** (`STM32G474VCTX_FLASH.ld`) без понимания последствий
9. **НЕ использовать float без FPU** — FPU доступен и настроен
10. **НЕ забывать про двойной буфер ADC** — DMA half-transfer и transfer complete

---

## Полезные ссылки (внутренние)

| Файл | Описание |
|------|----------|
| `Core/Src/main.c` | Точка входа, периферийная инициализация |
| `usr/src/_main.c` | Основная логика приложения |
| `usr/src/adc.c` | ADC dual interleaved логика |
| `usr/src/keys.c` | Кнопки и энкодеры |
| `usr/src/draw.c` | Отрисовка UI |
| `usr/src/lcd.c` | Базовые функции LCD |
| `usr/src/qspi.c` | QSPI Flash драйвер |
| `usr/inc/stm32g4xx_hal_conf.h` | Конфигурация HAL |
| `CMakeLists.txt` | Скрипт сборки |
| `STM32G474VCTX_FLASH.ld` | Linker script |
| `g474vc.ioc` | STM32CubeMX проект |

---

## Типичные задачи для агентов

### Добавить новый канал ADC
1. Настроить пин в `MX_GPIO_Init()`
2. Добавить канал в `adc.c` в `chParams[]` или создать новый channel pair
3. Настроить DMA если нужно
4. Обновить `DataBuffer.h` для нового буфера

### Изменить параметры LCD
1. `lcd.c` — базовые функции рисования
2. `draw.c` — high-level UI (сетка, фреймы)
3. `graph.c` — графические примитивы
4. `draw.h` — константы (`FRAME_STEP`, `LINE_Y`, цвета)

### Добавить новую кнопку/действие
1. `keys.c` — добавить пин в `KEYS_scan()`
2. `keys.h` — добавить `#define BUTTONx`
3. В `KEYS_scan()` добавить обработку и действие
4. Связать с `button1Count % MAX_ACTIONS`

### Изменить параметры генератора
1. `generator.c/h` — параметры TIM1
2. `dac.c/h` — параметры DAC + DMA
3. `_main.c` — `GEN_setParams()`, `DAC_startSin()`

---

## STM32G474 специфика

- **Резюме:** Cortex-M4 @ 170 МГц, FPU, 256 КБ Flash, 320 КБ SRAM
- **5 ADC** (ADC1-5), 2 группы (ADC12, ADC345)
- **1 DAC** (12-bit, 2 канала)
- **11 таймеров** (TIM1-15, включая HRTIM1)
- **FMC** для внешнего дисплея (16-bit parallel)
- **QSPI** для внешней Flash
- **USB FS**, CAN FD, I2S, SAI, UARTx, SPIx, I2Cx
- **Package:** LQFP100
- **Clock:** HSE 8 МГц → PLL → SYSCLK 170 МГц
