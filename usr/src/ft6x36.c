#include "main.h"
#include "ft6x36.h"
#include "ft6x36_regs.h"

// The address of the FT6x36 chip on the bus.
// Requires the address to already be shifted by one.
#define FT6x36_ADDR (0x38 << 1)
#define FT_I2C I2C1

#define  I2C_NO_STARTSTOP               (0x00000000U)
#define  I2C_GENERATE_STOP              (uint32_t)(0x80000000U | I2C_CR2_STOP)
#define  I2C_GENERATE_START_READ        (uint32_t)(0x80000000U | I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_GENERATE_START_WRITE       (uint32_t)(0x80000000U | I2C_CR2_START)


void I2C_TransferConfig(const uint8_t Size, const uint32_t Request) {

    const uint32_t tmp = (((uint32_t)FT6x36_ADDR & I2C_CR2_SADD) | \
                      (((uint32_t)Size << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES) | \
                      I2C_CR2_AUTOEND | (uint32_t)Request) & ~0x80000000U;

    MODIFY_REG(FT_I2C->CR2,
               ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND |
                   (I2C_CR2_RD_WRN & (Request >> (31U - I2C_CR2_RD_WRN_Pos))) |
                   I2C_CR2_START | I2C_CR2_STOP)),
               tmp);
}


void I2C_Transmit(const uint8_t *data, const uint8_t length) {
    uint8_t i = 0;

    while (LL_I2C_IsActiveFlag_BUSY(FT_I2C)) {}

    LL_I2C_TransmitData8(FT_I2C, data[i++]);
    I2C_TransferConfig(length, I2C_GENERATE_START_WRITE);

    while (i < length) {
        while (!LL_I2C_IsActiveFlag_TXIS(FT_I2C)) {}
        LL_I2C_TransmitData8(FT_I2C, data[i++]);
        I2C_TransferConfig(length - i, I2C_NO_STARTSTOP);
    }

    while (!LL_I2C_IsActiveFlag_STOP(FT_I2C)) {}
    LL_I2C_ClearFlag_STOP(FT_I2C);

    // I2C_RESET_CR2
    FT_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);
}

// receive 1 byte
void I2C_Receive(uint8_t * const data) {

    while (LL_I2C_IsActiveFlag_BUSY(FT_I2C)) {}

    I2C_TransferConfig(1, I2C_GENERATE_START_READ);

    while (!LL_I2C_IsActiveFlag_RXNE(FT_I2C)) {}

    *data = LL_I2C_ReceiveData8(FT_I2C);

    while (!LL_I2C_IsActiveFlag_STOP(FT_I2C)) {}
    LL_I2C_ClearFlag_STOP(FT_I2C);

    // I2C_RESET_CR2
    FT_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);
}


uint8_t  readRegister(const uint8_t _reg_addr) {
    uint8_t regval = 0;

    I2C_Transmit(&_reg_addr, 1);
    I2C_Receive(&regval);
    return regval;
}

void  writeRegister(uint8_t _reg_addr, uint8_t _val) {
    uint8_t data[] = {
        _reg_addr, _val
    };
    I2C_Transmit(data, 2);
}

uint8_t  readID() {
    return readRegister(FT_REG_FOCALTECH_ID);
}

uint8_t  getNrTouches() {
    uint8_t count = readRegister(FT_REG_TD_STATUS) & 0b111;
    if (count > 2) count = 0;   // can only have at most 2 touches but at startup there might be ff in the reg
    return count;
}

void  getPoint(uint8_t _n, touch_point_t *_point) {
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

uint8_t  getGesture() {
    return readRegister(FT_REG_GEST_ID);
}

uint8_t  getThreshold() {
    return readRegister(FT_REG_TH_GROUP);
}

void  setThreshold(uint8_t _threshold) {
    writeRegister(FT_REG_TH_GROUP, _threshold);
}

void  setModeSwitching(uint8_t _onoff) {
    // false = 0x00 = off, true = 0x01 = on
    writeRegister(FT_REG_CTRL, (uint8_t)_onoff);
}

void  setModeSwitchDelay(uint8_t _delay) {
    writeRegister(FT_REG_TIME_ENTER_MONITOR, _delay);
}

uint8_t  getPeriodActive() {
    return readRegister(FT_REG_PERIOD_ACTIVE);
}

uint8_t  getPeriodMonitor() {
    return readRegister(FT_REG_PERIOD_MONITOR);
}

void  setPeriodActive(uint8_t _rate) {
    writeRegister(FT_REG_PERIOD_ACTIVE, _rate);
}

void  setPeriodMonitor(uint8_t _rate) {
    writeRegister(FT_REG_PERIOD_MONITOR, _rate);
}