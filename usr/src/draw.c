#include <dwt.h>
#include "draw.h"
#include "graph.h"


void drawFrame() {
    u16 x, y;

//    LCD_Clear(BLACK);
    eraseGraph();
//    POINT_COLOR = GRAY;  // Drawing pen color
    BACK_COLOR = CLR_BACKGROUND;

    u32 t0 = DWT_Get_Current_Tick();

    for (y = FRAME_DIV_PIXELS; y < SIZE_Y; y += FRAME_DIV_PIXELS) {
        if (y == 128) POINT_COLOR = GRAY;  // coordinate axis
        else POINT_COLOR = DARKGRAY;       // division line
        LCD_Fill(0, y, SIZE_X - 1, y, POINT_COLOR);
    }

    for (x = FRAME_DIV_PIXELS; x < SIZE_X; x += FRAME_DIV_PIXELS) {
        if (x == 160) POINT_COLOR = GRAY;  // coordinate axis
        else POINT_COLOR = DARKGRAY;       // division line
        LCD_Fill(x, 0, x, SIZE_Y - 1, POINT_COLOR);
    }

    LCD_Set_Window(0, 0, SIZE_X - 1, SIZE_Y - 1);

    // count time for one circle
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(110, LINE1_Y, ticks / DWT_IN_MICROSEC, 8, 12, 0);
}

void drawScreen() {
    drawFrame();

    u32 t0 = DWT_Get_Current_Tick();

    drawGraph();

    // count time for one circle
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(170, LINE1_Y, ticks / DWT_IN_MICROSEC, 8, 12, 0);
}
