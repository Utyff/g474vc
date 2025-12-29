
#include "ft6x36.h"
#include "ft3x36_regs.h"


// I2C handle to use for communication
I2C_HandleTypeDef *hi2c;
// the address of the FD3x36 chip on the bus (this is fixed)
uint8_t addr;  // already shifted in constructor


void FT6x36(I2C_HandleTypeDef *_hi2c) {
     hi2c = _hi2c;
     addr = 0x38 << 1; // Cube HAL requires the address to already be shifted by one
}

uint8_t  readRegister(uint8_t _reg_addr)
{
    uint8_t regval = 0;

    HAL_I2C_Master_Transmit(hi2c, addr, &_reg_addr, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(hi2c, addr, &regval, 1, HAL_MAX_DELAY);
    return regval;
}

void  writeRegister(uint8_t _reg_addr, uint8_t _val)
{
    uint8_t data[] = {
        _reg_addr, _val
    };
    HAL_I2C_Master_Transmit(hi2c, addr, data, 2, HAL_MAX_DELAY);
}

uint8_t  readID()
{
    return readRegister(FT_REG_FOCALTECH_ID);
}

uint8_t  getNrTouches()
{
    uint8_t count = readRegister(FT_REG_TD_STATUS) & 0b111;
    if (count > 2) count = 0;   // can only have at most 2 touches but at startup there might be ff in the reg
    return count;
}

void  getPoint(uint8_t _n, touch_point_t *_point)
{
    uint8_t xh = readRegister(FT_REG_P1_XH + (FT_POINT_REGS_OFFSET * _n));
    uint8_t xl = readRegister(FT_REG_P1_XL + (FT_POINT_REGS_OFFSET * _n));
    uint8_t yh = readRegister(FT_REG_P1_YH + (FT_POINT_REGS_OFFSET * _n));
    uint8_t yl = readRegister(FT_REG_P1_YL + (FT_POINT_REGS_OFFSET * _n));
    
    _point->x = (xh & 0x0F) << 8 | xl;
    _point->y = (yh & 0x0F) << 8 | yl;
    _point->event = FT_GET_EVENT_FLAG(xh);
    _point->touch_id = FT_GET_TOUCH_ID(yh);
    _point->weight = readRegister(FT_REG_P1_WEIGHT + (FT_POINT_REGS_OFFSET * _n));
    _point->area = FT_GET_TOUCH_AREA(readRegister(FT_REG_P1_MISC + (FT_POINT_REGS_OFFSET * _n)));
}

uint8_t  getGesture()
{
    return readRegister(FT_REG_GEST_ID);
}

uint8_t  getThreshold()
{
    return readRegister(FT_REG_TH_GROUP);
}

void  setThreshold(uint8_t _threshold)
{
    writeRegister(FT_REG_TH_GROUP, _threshold);
}

void  setModeSwitching(uint8_t _onoff)
{
    // false = 0x00 = off, true = 0x01 = on
    writeRegister(FT_REG_CTRL, (uint8_t)_onoff);
}

void  setModeSwitchDelay(uint8_t _delay)
{
    writeRegister(FT_REG_TIME_ENTER_MONITOR, _delay);
}

uint8_t  getPeriodActive()
{
    return readRegister(FT_REG_PERIOD_ACTIVE);
}

uint8_t  getPeriodMonitor()
{
    return readRegister(FT_REG_PERIOD_MONITOR);
}

void  setPeriodActive(uint8_t _rate)
{
    writeRegister(FT_REG_PERIOD_ACTIVE, _rate);
}

void  setPeriodMonitor(uint8_t _rate)
{
    writeRegister(FT_REG_PERIOD_MONITOR, _rate);
}