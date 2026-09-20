#ifndef G474VC_EEPROM_H
#define G474VC_EEPROM_H

void EEPROM_Test();
uint8_t EEPROM_WriteBytes(uint16_t addr, uint8_t *buf, uint16_t bytes_count);
uint8_t EEPROM_ReadBytes(uint16_t addr, uint8_t *buf, uint16_t bytes_count);

#endif //G474VC_EEPROM_H
