#include "main.h"
#include "ft6x36.h"
#include "ft6x36_regs.h"

// Адрес чипа FT6x36 на шине I2C.
// Ожидается, что адрес уже сдвинут на 1 бит влево (7-битный адрес << 1).
#define FT6x36_ADDR (0x38 << 1)
#define FT_I2C I2C1

// Константы для конфигурации I2C-передачи
#define  I2C_NO_STARTSTOP               (0x00000000U)
#define  I2C_GENERATE_STOP              (uint32_t)(0x80000000U | I2C_CR2_STOP)
#define  I2C_GENERATE_START_READ        (uint32_t)(0x80000000U | I2C_CR2_START | I2C_CR2_RD_WRN)
#define  I2C_GENERATE_START_WRITE       (uint32_t)(0x80000000U | I2C_CR2_START)


/**
 * @brief Настройка конфигурационного регистра I2C (CR2) для передачи.
 * Устанавливает адрес устройства, количество байт, режим авто-завершения
 * и генерацию START/STOP условий.
 * @param Size — количество байт для передачи.
 * @param Request — флаги генерации START/STOP (I2C_GENERATE_START_READ,
 *                  I2C_GENERATE_START_WRITE, I2C_NO_STARTSTOP).
 */
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


/**
 * @brief Передача массива данных на устройство по I2C.
 * Использует прямую запись в регистры LL для максимальной скорости.
 * После передачи сбрасывает поле CR2 для подготовки к следующей операции.
 * @param data — указатель на передаваемый буфер.
 * @param length — количество байт для передачи.
 */
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

    // Сброс поля CR2 (I2C_RESET_CR2)
    FT_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);
}


/**
 * @brief Приём одного байта от устройства по I2C.
 * Ожидает готовности шины, генерирует START-условие для чтения,
 * принимает байт и генерирует STOP-условие.
 * @param data — указатель на переменную для сохранения принятого байта.
 */
void I2C_Receive(uint8_t * const data) {

    while (LL_I2C_IsActiveFlag_BUSY(FT_I2C)) {}

    I2C_TransferConfig(1, I2C_GENERATE_START_READ);

    while (!LL_I2C_IsActiveFlag_RXNE(FT_I2C)) {}

    *data = LL_I2C_ReceiveData8(FT_I2C);

    while (!LL_I2C_IsActiveFlag_STOP(FT_I2C)) {}
    LL_I2C_ClearFlag_STOP(FT_I2C);

    // Сброс поля CR2 (I2C_RESET_CR2)
    FT_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);
}


/**
 * @brief Чтение одного регистра чипа FT6x36.
 * Выполняет запись адреса регистра, затем чтение одного байта данных.
 * @param _reg_addr — адрес читаемого регистра.
 * @return значение регистра.
 */
uint8_t readRegister(const uint8_t _reg_addr) {
    uint8_t regval = 0;

    I2C_Transmit(&_reg_addr, 1);
    I2C_Receive(&regval);
    return regval;
}


/**
 * @brief Запись значения в регистр чипа FT6x36.
 * Отправляет два байта: адрес регистра и значение.
 * @param _reg_addr — адрес записываемого регистра.
 * @param _val — записываемое значение.
 */
void writeRegister(uint8_t _reg_addr, uint8_t _val) {
    uint8_t data[] = {
        _reg_addr, _val
    };
    I2C_Transmit(data, 2);
}


/**
 * @brief Чтение идентификатора чипа FT6x36.
 * Возвращает значение из регистра FOCALTECH_ID (0xA3),
 * которое содержит идентификатор производителя/модели.
 * @return идентификатор чипа. Для FT6236 обычно 0x11 или 0x64.
 */
uint8_t readID(void) {
    return readRegister(FT_REG_FOCALTECH_ID);
}


/**
 * @brief Получение количества активных касаний.
 * Считывает регистр TD_STATUS и извлекает биты количества точек.
 * При некорректном значении (>2) возвращает 0 — чип поддерживает
 * максимум 2 одновременных касания.
 * @return количество активных касаний (0, 1 или 2).
 */
uint8_t getNrTouches(void) {
    uint8_t count = readRegister(FT_REG_TD_STATUS) & 0b111;
    if (count > 2) count = 0;   // Максимум 2 касания, при старте может быть 0xFF
    return count;
}


/**
 * @brief Чтение информации о точке касания.
 * Считывает координаты X/Y, событие, ID касания, вес и площадь из
 * соответствующих регистров чипа (P1_XH, P1_XL, P1_YH, P1_YL и др.).
 * Не проверяет, является ли точка активной — это задача вызывающего кода.
 * @param _n — номер точки (0 или 1).
 * @param _point — указатель на структуру для сохранения данных.
 */
void getPoint(uint8_t _n, touch_point_t *_point) {
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


/**
 * @brief Чтение регистра жеста.
 * Возвращает значение из регистра GEST_ID, которое содержит
 * идентификатор распознанного жеста (если есть).
 * @return значение регистра жеста.
 */
uint8_t getGesture(void) {
    return readRegister(FT_REG_GEST_ID);
}


/**
 * @brief Чтение значения порога чувствительности.
 * Считывает регистр TH_GROUP, содержащий порог обнаружения касания.
 * @return текущее значение порога (0..255).
 */
uint8_t getThreshold(void) {
    return readRegister(FT_REG_TH_GROUP);
}


/**
 * @brief Установка порога чувствительности касания.
 * Записывает значение в регистр TH_GROUP.
 * Более высокое значение → менее чувствительный экран,
 * более низкое → более чувствительный.
 * @param _threshold — новое значение порога (0..255).
 */
void setThreshold(uint8_t _threshold) {
    writeRegister(FT_REG_TH_GROUP, _threshold);
}


/**
 * @brief Включение/выключение переключения режимов работы.
 * Когда включено (по умолчанию), чип автоматически переключается
 * в "monitor mode" (режим мониторинга) при отсутствии касаний
 * после заданного периода. В monitor mode снижается частота опроса
 * для экономии энергии. При обнаружении касания чип возвращается
 * в active mode.
 * Если переключение отключено, чип постоянно работает в active mode.
 * @param _onoff — true (1) включить, false (0) выключить.
 */
void setModeSwitching(uint8_t _onoff) {
    // false = 0x00 = выкл, true = 0x01 = вкл
    writeRegister(FT_REG_CTRL, (uint8_t)_onoff);
}


/**
 * @brief Установка задержки переключения в monitor mode.
 * Определяет время ожидания перед переключением из active mode
 * в monitor mode после завершения касания.
 * @param _delay — время задержки в произвольных единицах регистра.
 */
void setModeSwitchDelay(uint8_t _delay) {
    writeRegister(FT_REG_TIME_ENTER_MONITOR, _delay);
}


/**
 * @brief Чтение периода опроса в active mode.
 * @return период опроса в active mode (предположительно в мс,
 *         но точное значение не документировано в даташите).
 */
uint8_t getPeriodActive(void) {
    return readRegister(FT_REG_PERIOD_ACTIVE);
}


/**
 * @brief Чтение периода опроса в monitor mode.
 * @return период опроса в monitor mode (предположительно в мс,
 *         но точное значение не документировано в даташите).
 */
uint8_t getPeriodMonitor(void) {
    return readRegister(FT_REG_PERIOD_MONITOR);
}


/**
 * @brief Установка периода опроса в active mode.
 * Определяет частоту обновления данных при обнаружении касания.
 * @param _rate — значение периода в произвольных единицах
 *                (предположительно мс, но не документировано).
 */
void setPeriodActive(uint8_t _rate) {
    writeRegister(FT_REG_PERIOD_ACTIVE, _rate);
}


/**
 * @brief Установка периода опроса в monitor mode.
 * Определяет частоту обновления данных в режиме энергосбережения.
 * @param _rate — значение периода в произвольных единицах
 *                (предположительно мс, но не документировано).
 */
void setPeriodMonitor(uint8_t _rate) {
    writeRegister(FT_REG_PERIOD_MONITOR, _rate);
}
