#include <dwt.h>
#include <lcd.h>
#include <draw.h>
#include <graph.h>


void drawFrame() {
    POINT_COLOR = GRAY; // Drawing pen color
    BACK_COLOR = CLR_BACKGROUND;

    u32 t0 = DWT_Get_Current_Tick();

    // horizontal lines
    for (u16 y = FRAME_STEP + BOTTOM_LINE_HEIGHT; y < MAX_Y - TOP_LINE_HEIGHT; y += FRAME_STEP) {
        if (y == MAX_Y / 2) POINT_COLOR = GRAY; // Drawing pen color
        else POINT_COLOR = DARKGRAY;
        LCD_Fill(0, y, MAX_X - 1, y, POINT_COLOR);
    }

    // vertical lines
    for (u16 x = FRAME_STEP; x < MAX_X; x += FRAME_STEP) {
        if (x == MAX_X / 2 + FRAME_STEP / 2) POINT_COLOR = GRAY; // Drawing pen color
        else POINT_COLOR = DARKGRAY;
        LCD_Fill(x, TOP_LINE_HEIGHT, x, MAX_Y - BOTTOM_LINE_HEIGHT - 1, POINT_COLOR);
    }

    LCD_Set_Window(0, 0, MAX_X - 1, MAX_Y - 1);

    // count time for one circle
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(110, LINE1_Y, ticks / DWT_IN_MICROSEC, 8, 12, 0);
}

void drawScreen() {
    eraseAllGraph();
    drawFrame();

    u32 t0 = DWT_Get_Current_Tick();

    drawAllGraph();

    // count time for one circle
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(170, LINE1_Y, ticks / DWT_IN_MICROSEC, 8, 12, 0);
}
