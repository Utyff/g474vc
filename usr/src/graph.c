#include <graph.h>
#include <dwt.h>
#include <DataBuffer.h>
#include <lcd.h>
#include <draw.h>


/**
 * Make and draw oscillogram
 */

uint8_t graph0[MAX_X];
uint8_t graph1[MAX_X];
float scaleX = 1; // no more than 1
float scaleY = 0.94f;
u8 trgLvl = 128;

/**
 * Looking for trigger event position in 1 channel samples array
 * @return if trigger found - index of start element. Other case - 0
 */
int triggerStart1ch(u8 const *samples) {
    int i;
    u8 trgRdy = 0;

    for (i = 0; i < BUF_SIZE; i++) {
        if (trgRdy == 0) {
            if (samples[i] < trgLvl)
                trgRdy = 1;
            continue;
        }

        if (samples[i] > trgLvl)
            return i;
    }
    return 0;
}


// start position in buffer
// number of samples to display
uint32_t BuildGraphTick;

/**
 * Build graph for 1 channels samples array
 */
static void buildGraph(uint8_t ch) {
    uint32_t t0 = DWT_Get_Current_Tick();
    int i, j;
    float x;

    u8 *samples = ch == 0 ? samplesBuffer0 : samplesBuffer1;
    u8 *graph = ch == 0 ? graph0 : graph1;

    x = 0;
    j = -1;
    i = triggerStart1ch(samples);
    for (; i < BUF_SIZE; i++) {
        register uint8_t val = (uint8_t) (samples[i] * scaleY);
        if ((int) x != j) {
            j = (int) x;
            if (j >= MAX_X) break;
            graph[j] = val / 2;
        } else {
            graph[j] = ((graph[j] + val) >> 1) / 2; // arithmetical mean
        }
        x += scaleX;
    }
    BuildGraphTick = DWT_Elapsed_Tick(t0);
}

void buildAllGraph() {
    buildGraph(0);
    buildGraph(1);
}

uint32_t DrawGraphTick;

static void drawGraph(uint8_t *graph) {
    u8 prev;
    u16 yShift = TOP_LINE_HEIGHT;
    if (graph == graph1) {
        yShift += MAX_Y / 2;
    }

    uint32_t t0 = DWT_Get_Current_Tick();

    prev = graph[0];
    u16 color = graph == graph0 ? CLR_CH0 : CLR_CH1;

    for (u16 i = 1; i < MAX_X; i++) {
        //LCD_DrawLine(i - (u16) 1, prev, i, graph[i]);
        LCD_Fill(i, prev + yShift, i, graph[i] + yShift, color);
        prev = graph[i];
    }
    LCD_Set_Window(0, 0, MAX_X - 1, MAX_Y - 1);

    DrawGraphTick = DWT_Elapsed_Tick(t0);
}

void drawAllGraph() {
    buildAllGraph();
    drawGraph(graph0);
    drawGraph(graph1);
}

static void eraseGraph(uint8_t *graph) {
    u8 prev;
    u16 yShift = TOP_LINE_HEIGHT;
    if (graph == graph1) {
        yShift += MAX_Y / 2;
    }

    POINT_COLOR = BLACK;
    prev = graph[0];
    for (u16 i = 1; i < MAX_X; i++) {
        //LCD_DrawLine(i - (u16) 1, prev, i, graph[i]);
        LCD_Fill(i, prev + yShift, i, graph[i] + yShift, POINT_COLOR);
        prev = graph[i];
    }
    LCD_Set_Window(0, 0, MAX_X - 1, MAX_Y - 1);
}

void eraseAllGraph() {
    eraseGraph(graph0);
    eraseGraph(graph1);
}
