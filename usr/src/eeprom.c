#include "main.h"
#include "eeprom.h"
#include "stm32g4xx_ll_i2c.h"
#include "stm32g4xx_ll_utils.h" // Для LL_mDelay

#define EEPROM_I2C_ADDRESS  0xA0  // 7-bit address (0x50) << 1
#define EEPROM_TIMEOUT         10000 // Условный таймаут для циклов ожидания
#define I2Cx I2C1


uint8_t  rdBuf[100];
uint8_t  rdBuf2[100];
uint8_t  wrBuf[100];


void EEPROM_Test() {
    for (int i = 0; i<100; i++) {
        wrBuf[i] = i;
    }
    EEPROM_ReadBytes(0, rdBuf, 24);
    EEPROM_WriteBytes(0, wrBuf, 24);
    EEPROM_ReadBytes(0, rdBuf2, 24);
}

/**
  * @brief  Запись страницы/массива в EEPROM (в пределах одной страницы)
  * @param  mem_addr: 16-битный внутренний адрес в EEPROM
  * @param  pData: Указатель на буфер с данными
  * @param  len: Количество байт для записи
  */
uint8_t EEPROM_WriteBytes(uint16_t mem_addr, uint8_t *pData, uint16_t len) {
    uint32_t timeout = EEPROM_TIMEOUT;

    // Ждем, пока шина освободится
    while(LL_I2C_IsActiveFlag_BUSY(I2Cx)) {
        if(--timeout == 0) return 0;
    }

    // Настраиваем передачу: адрес устройства, количество байт (len + 2 байта адреса)
    // Включаем AUTOEND (автоматический STOP после отправки всех байт)
    LL_I2C_HandleTransfer(I2Cx, EEPROM_I2C_ADDRESS, LL_I2C_ADDRESSING_MODE_7BIT,
        (uint8_t)(len + 2), LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

    // Отправляем Старший байт (MSB) адреса памяти
    while(!LL_I2C_IsActiveFlag_TXIS(I2Cx)){}
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(mem_addr >> 8));

    // Отправляем Младший байт (LSB) адреса памяти
    while(!LL_I2C_IsActiveFlag_TXIS(I2Cx)){}
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(mem_addr & 0xFF));

    // Передаем массив данных
    for(uint16_t i = 0; i < len; i++) {
        timeout = EEPROM_TIMEOUT;
        while(!LL_I2C_IsActiveFlag_TXIS(I2Cx)) {
            if(--timeout == 0) return 0;
        }
        LL_I2C_TransmitData8(I2Cx, pData[i]);
    }

    // Ждем окончания генерации STOP-условия
    timeout = EEPROM_TIMEOUT;
    while(!LL_I2C_IsActiveFlag_STOP(I2Cx)) {
        if(--timeout == 0) return 0;
    }

    // Очищаем флаг STOP
    LL_I2C_ClearFlag_STOP(I2Cx);

    // ВАЖНО: Физическая запись в EEPROM занимает время. Ждем завершения цикла записи.
    LL_mDelay(5);

    return 1;
}

/**
  * @brief  Последовательное чтение массива данных из EEPROM
  * @param  mem_addr: 16-битный внутренний адрес в EEPROM
  * @param  pData: Указатель на буфер для сохранения данных
  * @param  len: Количество байт для чтения
  */
uint8_t EEPROM_ReadBytes(uint16_t mem_addr, uint8_t *pData, uint16_t len) {
    uint32_t timeout = EEPROM_TIMEOUT;

    while(LL_I2C_IsActiveFlag_BUSY(I2Cx)) {
        if(--timeout == 0) return 0;
    }

    // Запись адреса чтения (2 байта адреса памяти)
    // Используем SOFTEND, чтобы не генерировать STOP, а сделать Repeated START
    LL_I2C_HandleTransfer(I2Cx, EEPROM_I2C_ADDRESS, LL_I2C_ADDRESSING_MODE_7BIT, 2, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

    // Отправляем MSB адреса
    while(!LL_I2C_IsActiveFlag_TXIS(I2Cx)){}
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(mem_addr >> 8));

    // Отправляем LSB адреса
    while(!LL_I2C_IsActiveFlag_TXIS(I2Cx)){}
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(mem_addr & 0xFF));

    // Ждем окончания передачи адреса (флаг TC — Transfer Complete для SOFTEND)
    timeout = EEPROM_TIMEOUT;
    while(!LL_I2C_IsActiveFlag_TC(I2Cx)) {
        if(--timeout == 0) return 0;
    }

    // Перезапуск шины (Repeated START) в режиме ЧТЕНИЯ с AUTOEND
    LL_I2C_HandleTransfer(I2Cx, EEPROM_I2C_ADDRESS, LL_I2C_ADDRESSING_MODE_7BIT, (uint8_t)len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

    // Читаем поток байт
    for(uint16_t i = 0; i < len; i++)
    {
        timeout = EEPROM_TIMEOUT;
        while(!LL_I2C_IsActiveFlag_RXNE(I2Cx)) {
            if(--timeout == 0) return 0;
        }
        pData[i] = LL_I2C_ReceiveData8(I2Cx);
    }

    // Ждем флаг STOP от AUTOEND
    timeout = EEPROM_TIMEOUT;
    while(!LL_I2C_IsActiveFlag_STOP(I2Cx)) {
        if(--timeout == 0) return 0;
    }

    LL_I2C_ClearFlag_STOP(I2Cx);

    return 1;
}
