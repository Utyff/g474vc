#include <stdio.h>
#include <lcd_fmc.h>
#include <lcd.h>
#include "font.h"
#include "delay.h"

/**
 * 2.4 Inch /2.8 inch/3.5 inch/4.3 inch TFT LCD driver
 * Support driver IC models: ILI9341
 */

u16 POINT_COLOR = 0x0000; // Drawing pen color
u16 BACK_COLOR = 0xFFFF;  // background color

// Management LCD important parameters
_lcd_dev lcddev;


void LCD_Init_sequence();

// Start writing GRAM
__STATIC_INLINE void LCD_WriteRAM_Prepare(void) {
    LCD_WR_REG(LCD_WR_RAM_CMD);
}

// Set the cursor position
//Xpos: abscissa
//Ypos: ordinate
void LCD_SetCursor(u16 x, u16 y) {
    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA8(x >> 8);
    LCD_WR_DATA8(x & (u16) 0XFF);
    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA8(y >> 8);
    LCD_WR_DATA8(y & (u16) 0XFF);
}

// Set the window, and automatically sets the upper left corner of the window to draw point coordinates (sx,sy).
//sx,sy: window start coordinate (upper left corner)
//width,height: width and height of the window, must be greater than 0!!
// Form size:width*height.
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

#define LCD_W  320
#define LCD_H  480

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


// Draw points
//x,y: coordinates
//POINT_COLOR: the color of this point
void LCD_DrawPoint(u16 x, u16 y) {
    if (x >= MAX_X || y >= MAX_Y)
        return;

    LCD_SetCursor(x, y);       // Set the cursor position
    LCD_WriteRAM_Prepare();    // Start writing GRAM
    LCD_WR_DATA(POINT_COLOR);
}

// Draw the point fast
//x,y: coordinates
//color: color
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color) {
    if (x >= MAX_X || y >= MAX_Y)
        return;

    LCD_WR_REG(LCD_SET_X);
    LCD_WR_DATA8(x >> 8);
    LCD_WR_DATA8(x & (u16) 0XFF);
    LCD_WR_REG(LCD_SET_Y);
    LCD_WR_DATA8(y >> 8);
    LCD_WR_DATA8(y & (u16) 0XFF);

    LCD_WriteRAM_Prepare();    // Start writing GRAM
    LCD_WR_DATA(color);
}


// Set the LCD display direction
//dir:0, vertical screen; 1, horizontal screen
void LCD_Display_Dir(u8 dir) {
    if (dir == 0) {         // Vertical screen
        lcddev.dir = 0;
        lcddev.width = 320;
        lcddev.height = 480;
    } else {                 // Horizontal screen
        lcddev.dir = 1;
        lcddev.width = 480;
        lcddev.height = 320;
    }

    LCD_Scan_Dir(DFT_SCAN_DIR);    // Default scan direction
}

vu16 lid=0x1234;

// Initialize lcd
// This initialization function can initialize the various ILI93XX LCD, but the other function is based ILI9320!!!
// Not been tested on other types of driver chip!
void LCD_Init(void) {

    delay_ms(10);
    LCD_WR_REG(0X01); // SWRESET
    delay_ms(10);
    // Try to read the 7796 ID
    LCD_WR_REG(0XD3);
    lcddev.id1 = LCD_RD_DATA();   // dummy read
    lcddev.id1 = LCD_RD_DATA();   // dummy read
    lcddev.id2 = LCD_RD_DATA();   // Read 0X00
    lcddev.id3 = LCD_RD_DATA();   // Read 77
    lcddev.id4 = LCD_RD_DATA();   // Read 96
    lid = (lcddev.id3 << 8 | lcddev.id4);
//        if (lid != 0X7796) {
//            Error_Handler();
//        }

    char buf[250];
    sprintf(buf, "\nLCD ID: %x\n", lid);
    DBG_Trace((uint8_t *) buf);

    LCD_Init_sequence();

    LCD_Display_Dir(1);  // default to portrait
    LCD_Clear(GREEN);
}

void LCD_Init_sequence() {
//************* ST7796S 340x480 **********//
    LCD_WR_REG(0x11);

    delay_ms(120);              //Delay 120ms

    LCD_WR_REG(0x36);     // Memory Data Access Control MY,MX~~
    LCD_WR_DATA(0x48);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);     //LCD_WR_DATA(0x66);

    LCD_WR_REG(0xF0);     // Command Set Control
    LCD_WR_DATA(0xC3);

    LCD_WR_REG(0xF0);
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

// Clear screen function
//color: To clear the screen fill color
void LCD_Clear(u16 color) {
//    clearScreen_dma(color);
//    return;

    // get start time
//    u32 t0 = DWT_Get_Current_Tick();

    LCD_Set_Window(0, 0, MAX_X - 1, MAX_Y - 1);  // set the cursor position
    LCD_WriteRAM_Prepare();                  // start writing GRAM

    u32 totalPoints = lcddev.width * lcddev.height;  // get the total number of points
    for (u32 i = 0; i < totalPoints; i++) {
        LCD_WR_DATA(color);
    }

//    u32 LCDClearTick = DWT_Elapsed_Tick(t0);
//    POINT_COLOR = YELLOW;
//    LCD_ShowxNum(100, MAX_Y-13, LCDClearTick / DWT_IN_MICROSEC, 8, 12, 9);
}

// Fill a single color in the designated area
//(sx,sy),(ex,ey): filled rectangle coordinates diagonal, area size:(ex-sx+1)*(ey-sy+1)
//color: To fill color
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color) {
    u16 tmp;
    if (sy > ey) {
        tmp = sy;
        sy = ey;
        ey = tmp;
    }
    u32 totalPoints = (ex - sx + (u16) 1) * (ey - sy + (u16) 1);

    LCD_Set_Window(sx, sy, ex, ey);          // set the cursor position
    LCD_WriteRAM_Prepare();                  // start writing GRAM
    for (int j = 0; j < totalPoints; j++) {  // display colors
        LCD_WR_DATA(color);
    }
}

// Draw a line
//x1,y1: starting point coordinates
//x2,y2: end coordinates
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2) {
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; // calculate the coordinates increment
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;
    if (delta_x > 0)incx = 1; // set the single-step directions
    else if (delta_x == 0)incx = 0;// vertical line
    else {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;// horizontal
    else {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x; // Select the basic incremental axis
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++)  // draw a line output
    {
        LCD_DrawPoint(uRow, uCol);       // draw points
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

// Draw a rectangle
//(x1,y1),(x2,y2): rectangle coordinates diagonal
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2) {
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

// A circle the size of the appointed position Videos
//(x,y): the center
//r    : Radius
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r) {
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);             // determine the next point position sign
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
        // Use Bresenham algorithm Circle
        if (di < 0)di += 4 * a + 6;
        else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

// Display a character in the specified location
//x,y: Start coordinates
//num:characters to be displayed:" "--->"~"
//size: Font size 12/16/24
//mode: the superposition mode (1) or non-overlapping mode (0)
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode) {
    u8 temp, t1, t;
    u16 y0 = y;
    // get a font character set corresponding to the number of bytes occupied by a dot
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    // Setup Window
    num = num - ' ';// values obtained after offset
    for (t = 0; t < csize; t++) {
        if (size == 12)temp = asc2_1206[num][t];        // call 1206 font
        else if (size == 16)temp = asc2_1608[num][t];    // call 1608 font
        else if (size == 24)temp = asc2_2412[num][t];    // call 2412 font
        else return;                                // no fonts
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);
            temp <<= 1;
            y++;
            if (y >= lcddev.height)return;        // over the region
            if ((y - y0) == size) {
                y = y0;
                x++;
                if (x >= lcddev.width)return;    // over the region
                break;
            }
        }
    }
}

// m^n function
// Return value:m^n-th power.
u32 LCD_Pow(u8 m, u8 n) {
    u32 result = 1;
    while (n--) result *= m;
    return result;
}

// Show figures, the high is 0, no display
//x,y : the starting point coordinates
//len : Digits
//size: Font Size
//color: color
//num: Numerical(0~4294967295);
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

// Show figures, the high is 0, or show
//x,y: the starting point coordinates
//num: Numerical (0~999999999);
//len: length (ie the number of digits to be displayed)
//size: Font Size
//mode:
//[7]:0, no padding;1, filled with 0.
//[6:1]: Reserved
//[0]:0, non-superimposition display;1, superimposed display.
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

// Display string
//x,y: the starting point coordinates
//width,height: size of the area
//size: Font Size
//*p: string starting address
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, const char *p, u8 mode) {
    u16 x0 = x;
    width += x;
    height += y;
    while ((*p <= '~') && (*p >= ' '))// judgment is not illegal characters!
    {
        if (x >= width) {
            x = x0;
            y += size;
        }
        if (y >= height)break;//Exit
        LCD_ShowChar(x, y, *p, size, mode);
        x += size / 2;
        p++;
    }
}
