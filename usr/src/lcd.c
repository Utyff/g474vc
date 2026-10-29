#include <stdio.h>
#include <lcd_fmc.h>
#include <lcd.h>
#include "font.h"
#include "delay.h"

/**
 * @file lcd.c
 * @brief Драйвер TFT-дисплеев 2.4" / 2.8" / 3.5" / 4.3" (контроллер ST7796S,
 * совместим с ILI9488), подключённого к параллельной шине FMC.
 */

u16 POINT_COLOR = 0x0000; // текущий цвет пера
u16 BACK_COLOR = 0xFFFF;  // фоновый цвет

// Параметры дисплея: размеры, ID контроллера, ориентация экрана
_lcd_dev lcddev;


// Вперёд, полное описание — в определении
void LCD_Init_sequence();

/**
 * @brief Начать запись в GRAM (буфер кадров).
 * После отправки команды «запись данных дисплея» последующие записи
 * данных поступают в пиксели в пределах ранее заданного окна
 * (см. LCD_Set_Window).
 */
__STATIC_INLINE void LCD_WriteRAM_Prepare(void) {
    LCD_WR_REG(LCD_WR_RAM_CMD);
}

/**
 * @brief Установить позицию курсора записи (адрес пикселя).
 * @param x координата X (столбец)
 * @param y координата Y (строка)
 */
void LCD_SetCursor(u16 x, u16 y) {
    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA8(x >> 8);
    LCD_WR_DATA8(x & (u16) 0XFF);
    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA8(y >> 8);
    LCD_WR_DATA8(y & (u16) 0XFF);
}

/**
 * @brief Задать окно рисования (прямоугольник пикселей).
 * Последующие записи пикселей заполняют окно построчно,
 * начиная с левого верхнего угла (sx, sy) и заканчивая (ex, ey).
 * @param sx координата X левого верхнего угла
 * @param sy координата Y левого верхнего угла
 * @param ex координата X правого нижнего угла
 * @param ey координата Y правого нижнего угла
 */
void LCD_Set_Window(u16 sx, u16 sy, u16 ex, u16 ey) {
    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA8(sx >> 8);
    LCD_WR_DATA8(sx & (u16) 0XFF);
    LCD_WR_DATA8(ex >> 8);
    LCD_WR_DATA8(ex & (u16) 0XFF);

    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA8(sy >> 8);
    LCD_WR_DATA8(sy & (u16) 0XFF);
    LCD_WR_DATA8(ey >> 8);
    LCD_WR_DATA8(ey & (u16) 0XFF);
}

#define LCD_W  SIZE_Y
#define LCD_H  SIZE_X

/**
 * @brief Установить направление сканирования экрана.
 * Также меняет логические размеры (lcddev.width / lcddev.height).
 * @param direction направление развёртки (0..3), значения соответствуют
 *        константам lcd.h: L2R_U2D, L2R_D2U, R2L_U2D, R2L_D2U
 */
void LCD_Scan_Dir(u8 direction)
{
    lcddev.dir = direction%4;
    switch(lcddev.dir){
        case 0:
            lcddev.width=LCD_W;
            lcddev.height=LCD_H;
            LCD_WriteReg(0x36,(1<<3)|(1<<6));
            break;
        case 1:
            lcddev.width=LCD_H;
            lcddev.height=LCD_W;
            LCD_WriteReg(0x36,(1<<3)|(1<<5));
            break;
        case 2:
            lcddev.width=LCD_W;
            lcddev.height=LCD_H;
            LCD_WriteReg(0x36,(1<<3)|(1<<7));
            break;
        case 3:
            lcddev.width=LCD_H;
            lcddev.height=LCD_W;
            LCD_WriteReg(0x36,(1<<3)|(1<<7)|(1<<6)|(1<<5));
            break;
        default:break;
    }
}


/**
 * @brief Нарисовать один пиксель текущим цветом пера (POINT_COLOR).
 * Точки вне экрана не рисуются.
 * @param x координата X
 * @param y координата Y
 */
void LCD_DrawPoint(u16 x, u16 y) {
    if (x >= SIZE_X || y >= SIZE_Y)
        return;

    LCD_SetCursor(x, y);        // установить позицию курсора
    LCD_WriteRAM_Prepare();     // начать запись в GRAM
    LCD_WR_DATA(POINT_COLOR);
}

/**
 * @brief Быстро нарисовать одну точку заданным цветом.
 * Точки вне экрана не рисуются.
 * @param x координата X
 * @param y координата Y
 * @param color цвет точки (16-битный RGB565)
 */
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color) {
    if (x >= SIZE_X || y >= SIZE_Y)
        return;

    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA8(x >> 8);
    LCD_WR_DATA8(x & (u16) 0XFF);
    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA8(y >> 8);
    LCD_WR_DATA8(y & (u16) 0XFF);

    LCD_WriteRAM_Prepare();    // начать запись в GRAM
    LCD_WR_DATA(color);
}


/**
 * @brief Установить ориентацию экрана.
 * Меняет логические размеры (lcddev.width / lcddev.height).
 * @param dir 0 — портретная; 1 — альбомная.
 * Физическое направление развёртки настраивается LCD_Scan_Dir().
 */
void LCD_Display_Dir(u8 dir) {
    if (dir == 0) {         // портретный экран
        lcddev.dir = 0;
        lcddev.width = LCD_W;
        lcddev.height = LCD_H;
    } else {                 // альбомный экран
        lcddev.dir = 1;
        lcddev.width = LCD_H;
        lcddev.height = LCD_W;
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);    // развёртка по умолчанию
}

vu32 lid = 0x1234; // ID контроллера, прочитанный из регистра 0xD3 (младшие 3 байта)

/**
 * @brief Инициализация дисплея: настройка шины FMC, программный сброс
 * контроллера, чтение ID микросхемы (вывод через DBG_Trace), выдача
 * инициализирующей последовательности, установка ориентации и
 * очистка экрана в чёрный цвет.
 * Последовательность рассчитана на контроллер ST7796S; на других
 * чипах (например, ILI93xx) не проверена.
 */
void LCD_Init(void) {
    FMC_Init();

    delay_ms(10);
    LCD_WR_REG(0X01); // SWRESET — программный сброс
    delay_ms(10);
    // попытка прочитать ID чипа (для ST7796S ожидается 0x007796)
    LCD_WR_REG(0XD3);
    lcddev.id1 = LCD_RD_DATA();   // Dummy-чтение
    lcddev.id1 = LCD_RD_DATA();   // Dummy-чтение
    lcddev.id2 = LCD_RD_DATA();   // старший байт ID (0x00)
    lcddev.id3 = LCD_RD_DATA();   // средний байт ID (0x77)
    lcddev.id4 = LCD_RD_DATA();   // младший байт ID (0x96)
    lid = lcddev.id2 << 16 | lcddev.id3 << 8 | lcddev.id4;
//        if (lid != 0X7796) {
//            Error_Handler();
//        }

    char buf[250];
    sprintf(buf, "\nLCD ID: %x\n", lid);
    DBG_Trace((uint8_t *) buf);

    LCD_Init_sequence();

    LCD_Display_Dir(1);  // по умолчанию альбомная ориентация
    LCD_Clear(BLACK);
}

/**
 * @brief Инициализирующая последовательность контроллера ST7796S (320x480):
 * выход из спящего режима, настройка управления доступом к памяти,
 * формата данных пикселя (16 бит RGB565), гамма-коррекции
 * и включение нормального режима отображения.
 * Регистр 0xF0 переключает набор команд (пользовательский / заводской).
 */
void LCD_Init_sequence() {
//************* ST7796S 320x480 **********//
    LCD_WR_REG(0x11);

    delay_ms(120);              // ожидание 120 мс на выход из спящего режима

    LCD_WR_REG(0x36);           // управление доступом к памяти (зеркальность MY/MX)
    LCD_WR_DATA(0x48);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);          // 16 бит на пиксель (RGB565)

    LCD_WR_REG(0xF0);           // заводской набор команд
    LCD_WR_DATA(0xC3);

    LCD_WR_REG(0xF0);           // пользовательский набор команд
    LCD_WR_DATA(0x96);

    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA(0xC6);

    //LCD_WR_REG(0xB9);
    //LCD_WR_DATA(0x02);
    //LCD_WR_DATA(0xE0);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x80);
    LCD_WR_DATA(0x45);

    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0x13);   //18  //00

    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0xA7);

    LCD_WR_REG(0xC5);
    LCD_WR_DATA(0x0F);

    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x8A);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x19);
    LCD_WR_DATA(0xA5);
    LCD_WR_DATA(0x33);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x30);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x47);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x31);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x47);
    LCD_WR_DATA(0x38);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x32);

    LCD_WR_REG(0xF0);
    LCD_WR_DATA(0x3C);

    LCD_WR_REG(0xF0);
    LCD_WR_DATA(0x69);

    delay_ms(120);

    LCD_WR_REG(0x21);
    LCD_WR_REG(0x29);
}

/**
 * @brief Очистить экран, залей одним цветом.
 * @param color цвет заполнения (16-битный RGB565)
 */
void LCD_Clear(u16 color) {
//    clearScreen_dma(color);
//    return;

    // фиксация начального времени
//    u32 t0 = DWT_Get_Current_Tick();

    LCD_Set_Window(0, 0, SIZE_X - 1, SIZE_Y - 1);  // окно на весь экран
    LCD_WriteRAM_Prepare();                  // начать запись в GRAM

    u32 totalPoints = lcddev.width * lcddev.height;  // общее количество пикселей
    for (u32 i = 0; i < totalPoints; i++) {
        LCD_WR_DATA(color);
    }

//    u32 LCDClearTick = DWT_Elapsed_Tick(t0); // замер времени очистки
//    POINT_COLOR = YELLOW;
//    LCD_ShowxNum(100, SIZE_Y-13, LCDClearTick / DWT_IN_MICROSEC, 8, 12, 9);
}

/**
 * @brief Залить прямоугольник одним цветом.
 * Область задаётся диагонально противоположными углами (sx, sy) и (ex, ey),
 * размер (ex - sx + 1) x (ey - sy + 1). При обратной записи координат Y
 * они автоматически меняются местами.
 * @param sx координата X первого угла
 * @param sy координата Y первого угла
 * @param ex координата X второго угла
 * @param ey координата Y второго угла
 * @param color цвет заполнения (16-битный RGB565)
 */
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color) {
    u16 tmp;
    if (sy > ey) {
        tmp = sy;
        sy = ey;
        ey = tmp;
    }
    u32 totalPoints = (ex - sx + (u16) 1) * (ey - sy + (u16) 1);

    LCD_Set_Window(sx, sy, ex, ey);          // задать окно заполнения
    LCD_WriteRAM_Prepare();                  // начать запись в GRAM
    for (int j = 0; j < totalPoints; j++) {  // заполнить пиксели
        LCD_WR_DATA(color);
    }
}

/**
 * @brief Нарисовать линию между двумя точками (алгоритм Брезенхама).
 * Линия рисуется текущим цветом пера (POINT_COLOR).
 * @param x1 координата X начальной точки
 * @param y1 координата Y начальной точки
 * @param x2 координата X конечной точки
 * @param y2 координата Y конечной точки
 */
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2) {
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; // приросты координат
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)incx = 1; // единичные шаги по осям
    else if (delta_x == 0)incx = 0;// вертикальная линия
    else {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;// горизонтальная линия
    else {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x; // главная ось линии
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++)  // построение линии
    {
        LCD_DrawPoint(uRow, uCol);       // нарисовать следующую точку
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/**
 * @brief Нарисовать контур прямоугольника.
 * Область задаётся диагонально противоположными углами (x1, y1) и (x2, y2);
 * стороны проводятся линиями текущим цветом пера (POINT_COLOR).
 * @param x1 координата X первого угла
 * @param y1 координата Y первого угла
 * @param x2 координата X второго угла
 * @param y2 координата Y второго угла
 */
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2) {
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

/**
 * @brief Нарисовать контур окружности (алгоритм Брезенхама, 8-кратная симметрия).
 * Окружность рисуется текущим цветом пера (POINT_COLOR).
 * @param x0 координата X центра
 * @param y0 координата Y центра
 * @param r  радиус окружности
 */
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r) {
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);             // признак позиции следующей точки
    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 - b); //5
        LCD_DrawPoint(x0 + b, y0 - a); //0
        LCD_DrawPoint(x0 + b, y0 + a); //4
        LCD_DrawPoint(x0 + a, y0 + b); //6
        LCD_DrawPoint(x0 - a, y0 + b); //1
        LCD_DrawPoint(x0 - b, y0 + a); //3
        LCD_DrawPoint(x0 - a, y0 - b); //2
        LCD_DrawPoint(x0 - b, y0 - a); //7
        a++;
        // построение окружности алгоритмом Брезенхама
        if (di < 0)di += 4 * a + 6;
        else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/**
 * @brief Отобразить один символ в заданной позиции.
 * @param x    координата X (начальный столбец)
 * @param y    координата Y (верхняя строка)
 * @param num  печатаемый ASCII-символ в диапазоне ' '..'~' (32..126)
 * @param size высота шрифта: 12, 16 или 24; ширина глифа = size/2
 * @param mode 1 — прозрачный фон (рисуются только биты глифа);
 *             0 — фон символа заливается BACK_COLOR
 */
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode) {
    u8 temp, t1, t;
    u16 y0 = y;
    // количество байтов на глиф для данной высоты шрифта
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    // позиция в таблице шрифта
    num = num - ' ';// сдвиг от ' '
    for (t = 0; t < csize; t++) {
        if (size == 12)temp = asc2_1206[num][t];        // шрифт 12x6
        else if (size == 16)temp = asc2_1608[num][t];    // шрифт 16x8
        else if (size == 24)temp = asc2_2412[num][t];    // шрифт 24x12
        else return;                                // нет шрифта данного размера
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);
            temp <<= 1;
            y++;
            if (y >= lcddev.height)return;        // за границей экрана (по Y)
            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= lcddev.width)return;    // за границей экрана (по X)
                break;
            }
        }
    }
}

/**
 * @brief Вычислить целую степень m^n.
 * @param m основание
 * @param n показатель степени
 * @return значение m, возведённое в степень n
 */
static u32 LCD_Pow(u8 m, u8 n) {
    u32 result = 1;
    while (n--) result *= m;
    return result;
}

/**
 * @brief Отобразить число (ведущие нули не выводятся).
 * Цифры выводятся слева направо, вместо отсутствующих цифр
 * рисуются пробелы (заполняются BACK_COLOR). Фон всегда заливается.
 * @param x    координата X начальной точки
 * @param y    координата Y начальной точки
 * @param num  отображаемое число (0..4294967295)
 * @param len  количество выводимых цифр
 * @param size размер шрифта (12/16/24)
 */
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size) {
    u8 t, temp;
    u8 enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                LCD_ShowChar(x + (size / 2) * t, y, ' ', size, 0);
                continue;
            } else enshow = 1;

        }
        LCD_ShowChar(x + (size / 2) * t, y, temp + '0', size, 0);
    }
}

/**
 * @brief Отобразить число с настройкой формата вывода.
 * @param x    координата X начальной точки
 * @param y    координата Y начальной точки
 * @param num  отображаемое число (0..4294967295)
 * @param len  количество выводимых цифр
 * @param size размер шрифта (12/16/24)
 * @param mode биты формата:
 *        бит 7 — ведущие нули заполнить '0' (0 — заполнить пробелами);
 *        бит 0 — 1: прозрачный фон, 0: заливка фона.
 */
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode) {
    u8 t, temp;
    u8 enshow = 0;
    for (t = 0; t < len; t++) {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (temp == 0) {
                if (mode & 0X80)LCD_ShowChar(x + (size / (u16) 2) * t, y, '0', size, mode & (u8) 0X01);
                else LCD_ShowChar(x + (size / (u16) 2) * t, y, ' ', size, mode & (u8) 0X01);
                continue;
            } else enshow = 1;
        }
        LCD_ShowChar(x + (size / (u16) 2) * t, y, temp + (u8) '0', size, mode & (u8) 0X01);
    }
}

/**
 * @brief Отобразить строку в области указанных размеров.
 * При достижении правой границы области текст переносится на новую строку;
 * вывод завершается при нижней границе, на непечатаемом символе
 * (вне диапазона ' '..'~') или на конце строки (null-байт).
 * @param x      координата X начальной точки
 * @param y      координата Y начальной точки
 * @param width  ширина текстовой области
 * @param height высота текстовой области
 * @param size   размер шрифта (12/16/24)
 * @param p      указатель на строку, завершённую нулём
 * @param mode   режим фона: 1 — прозрачный, 0 — заливка фона
 */
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, const char *p, u8 mode) {
    const u16 x0 = x;
    width += x;
    height += y;
    while (*p <= '~' && *p >= ' ') // только печатаемые символы от ' ' до '~'
    {
        if (x >= width) {
            x = x0;
            y += size;
        }
        if (y >= height)break; // выход за область
        LCD_ShowChar(x, y, *p, size, mode);
        x += size / 2;
        p++;
    }
}
