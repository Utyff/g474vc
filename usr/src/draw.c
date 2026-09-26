#include <dwt.h>
#include <lcd.h>
#include <draw.h>
#include <graph.h>


static void drawTopLine();


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
    drawTopLine();

    u32 t0 = DWT_Get_Current_Tick();

    drawAllGraph();

    // count time for one circle
    u32 ticks = DWT_Elapsed_Tick(t0);
    POINT_COLOR = YELLOW;
    LCD_ShowxNum(170, LINE1_Y, ticks / DWT_IN_MICROSEC, 8, 12, 0);
}

static void drawTopLine() {
    POINT_COLOR = WHITE; // Drawing pen color
    BACK_COLOR = activeChannel == 0 ? BROWN : DARKGRAY;
    LCD_ShowString(TOP_LINE_POSITION1, TOP_LINE_Y, 30, 60, 12, "CH1", 0);
    BACK_COLOR = activeChannel == 0 ? DARKGRAY : BROWN;
    LCD_ShowString(TOP_LINE_POSITION2, TOP_LINE_Y, 30, 60, 12, "CH2", 0);
    BACK_COLOR = DARKGRAY;
    LCD_ShowxNum(TOP_LINE_POSITION3, TOP_LINE_Y, ch1Shift, 5, 12, 0);
    LCD_ShowxNum(TOP_LINE_POSITION4, TOP_LINE_Y, ch2Shift, 5, 12, 0);
}