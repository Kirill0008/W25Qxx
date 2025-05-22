#ifndef W25QXX_H
#define W25QXX_H

#include <Arduino.h>
#include <SPI.h>

class W25QXX {
public:
    W25QXX(uint8_t csPin, uint32_t flashSizeMB = 16);
    
    void begin();
    void reset();
    
    uint32_t readJEDECID();
    uint32_t readManufacturerDeviceID();
    void read(uint32_t startPage, uint8_t offset, uint32_t length, uint8_t *data);
    
    void write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
    void writeClean(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
    void eraseSector(uint16_t sectorNum);
    
    void detectSize();
    uint32_t getCapacity() { return _capacity; }

private:
    void csHigh();
    void csLow();
    void writeEnable();
    void writeDisable();
    uint32_t bytesToWrite(uint32_t size, uint16_t offset);
    
    uint8_t _csPin;
    uint32_t _capacity;
    uint32_t _numBlocks;
    bool _use32bitAddressing;
};

#endif