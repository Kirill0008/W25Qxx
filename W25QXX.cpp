#include "W25QXX.h"

W25QXX::W25QXX(uint8_t csPin, uint32_t flashSizeMB) : _csPin(csPin) {
    _capacity = flashSizeMB * 1024 * 1024;
    _numBlocks = flashSizeMB * 8; // For 16Mb: 16*8=128 blocks (adjust as needed)
    _use32bitAddressing = (flashSizeMB >= 32);
}

void W25QXX::begin() {
    pinMode(_csPin, OUTPUT);
    csHigh();
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    reset();
}

void W25QXX::csHigh() {
    digitalWrite(_csPin, HIGH);
}

void W25QXX::csLow() {
    digitalWrite(_csPin, LOW);
}

void W25QXX::reset() {
    uint8_t tData[2] = {0x66, 0x99};
    csLow();
    SPI.transfer(tData, 2);
    csHigh();
    delay(100);
}

uint32_t W25QXX::readJEDECID() {
    uint8_t tData = 0x9F;
    uint8_t rData[3];
    
    csLow();
    SPI.transfer(tData);
    rData[0] = SPI.transfer(0);
    rData[1] = SPI.transfer(0);
    rData[2] = SPI.transfer(0);
    csHigh();
    
    return ((rData[0] << 16) | (rData[1] << 8) | rData[2]);
}

uint32_t W25QXX::readManufacturerDeviceID() {
    uint8_t tData = 0x90;
    uint8_t rData[2];
    
    csLow();
    SPI.transfer(tData);
    SPI.transfer(0);
    SPI.transfer(0);
    SPI.transfer(0);
    rData[0] = SPI.transfer(0);
    rData[1] = SPI.transfer(0);
    csHigh();
    
    return (rData[0] << 8 | rData[1]);
}

void W25QXX::read(uint32_t startPage, uint8_t offset, uint32_t length, uint8_t *data) {
    uint8_t tData[5];
    uint32_t memAddr = (startPage * 256) + offset;

    if (!_use32bitAddressing) {
        tData[0] = 0x03;
        tData[1] = (memAddr >> 16) & 0xFF;
        tData[2] = (memAddr >> 8) & 0xFF;
        tData[3] = memAddr & 0xFF;
    } else {
        tData[0] = 0x13;
        tData[1] = (memAddr >> 24) & 0xFF;
        tData[2] = (memAddr >> 16) & 0xFF;
        tData[3] = (memAddr >> 8) & 0xFF;
        tData[4] = memAddr & 0xFF;
    }

    csLow();
    if (!_use32bitAddressing) {
        SPI.transfer(tData, 4);
    } else {
        SPI.transfer(tData, 5);
    }

    for (uint32_t i = 0; i < length; i++) {
        data[i] = SPI.transfer(0);
    }
    csHigh();
}

void W25QXX::writeEnable() {
    uint8_t tData = 0x06;
    csLow();
    SPI.transfer(tData);
    csHigh();
    delay(5);
}

void W25QXX::writeDisable() {
    uint8_t tData = 0x04;
    csLow();
    SPI.transfer(tData);
    csHigh();
    delay(5);
}

void W25QXX::write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data) {
    uint8_t tData[266];
    uint32_t memAddr = (page * 256) + offset;
    
    writeEnable();
    
    if (!_use32bitAddressing) {
        tData[0] = 0x02;
        tData[1] = (memAddr >> 16) & 0xFF;
        tData[2] = (memAddr >> 8) & 0xFF;
        tData[3] = memAddr & 0xFF;
        
        for (uint32_t i = 0; i < size; i++) {
            tData[4 + i] = data[i];
        }
        
        csLow();
        SPI.transfer(tData, 4 + size);
        csHigh();
    } else {
        tData[0] = 0x12;
        tData[1] = (memAddr >> 24) & 0xFF;
        tData[2] = (memAddr >> 16) & 0xFF;
        tData[3] = (memAddr >> 8) & 0xFF;
        tData[4] = memAddr & 0xFF;
        
        for (uint32_t i = 0; i < size; i++) {
            tData[5 + i] = data[i];
        }
        
        csLow();
        SPI.transfer(tData, 5 + size);
        csHigh();
    }
    
    delay(5);
    writeDisable();
}

void W25QXX::eraseSector(uint16_t sectorNum) {
    uint8_t tData[5];
    uint32_t memAddr = sectorNum * 16 * 256;

    writeEnable();

    if (!_use32bitAddressing) {
        tData[0] = 0x20;
        tData[1] = (memAddr >> 16) & 0xFF;
        tData[2] = (memAddr >> 8) & 0xFF;
        tData[3] = memAddr & 0xFF;

        csLow();
        SPI.transfer(tData, 4);
        csHigh();
    } else {
        tData[0] = 0x21;
        tData[1] = (memAddr >> 24) & 0xFF;
        tData[2] = (memAddr >> 16) & 0xFF;
        tData[3] = (memAddr >> 8) & 0xFF;
        tData[4] = memAddr & 0xFF;

        csLow();
        SPI.transfer(tData, 5);
        csHigh();
    }

    delay(450);
    writeDisable();
}

uint32_t W25QXX::bytesToWrite(uint32_t size, uint16_t offset) {
    if ((size + offset) < 256) return size;
    else return 256 - offset;
}

void W25QXX::writeClean(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data) {
    uint8_t tData[266];
    
    uint32_t startPage = page;
    uint32_t endPage = startPage + ((size + offset - 1) / 256);
    uint32_t numPages = endPage - startPage + 1;

    uint16_t startSector = startPage / 16;
    uint16_t endSector = endPage / 16;
    uint16_t numSectors = endSector - startSector + 1;

    for (uint16_t i = 0; i < numSectors; i++) {
        eraseSector(startSector + i);
    }

    uint32_t dataPosition = 0;

    for (uint32_t i = 0; i < numPages; i++) {
        uint32_t memAddr = (startPage * 256) + offset;
        uint16_t bytesremaining = bytesToWrite(size, offset);
        uint8_t indx = 0;

        writeEnable();

        if (!_use32bitAddressing) {
            tData[0] = 0x02;
            tData[1] = (memAddr >> 16) & 0xFF;
            tData[2] = (memAddr >> 8) & 0xFF;
            tData[3] = memAddr & 0xFF;
            indx = 4;
        } else {
            tData[0] = 0x12;
            tData[1] = (memAddr >> 24) & 0xFF;
            tData[2] = (memAddr >> 16) & 0xFF;
            tData[3] = (memAddr >> 8) & 0xFF;
            tData[4] = memAddr & 0xFF;
            indx = 5;
        }

        for (uint16_t j = 0; j < bytesremaining; j++) {
            tData[indx + j] = data[dataPosition + j];
        }

        uint16_t bytestosend = bytesremaining + indx;

        csLow();
        if (bytestosend > 100) {
            uint16_t firstChunk = 100;
            SPI.transfer(tData, firstChunk);
            SPI.transfer(tData + firstChunk, bytestosend - firstChunk);
        } else {
            SPI.transfer(tData, bytestosend);
        }
        csHigh();

        startPage++;
        offset = 0;
        size -= bytesremaining;
        dataPosition += bytesremaining;

        delay(5);
        writeDisable();
    }
}

void W25QXX::detectSize() {
    uint32_t jedec_id = readJEDECID();
    uint8_t manufacturer = (jedec_id >> 16) & 0xFF;
    uint8_t memory_type = (jedec_id >> 8) & 0xFF;
    uint8_t capacity = jedec_id & 0xFF;

    Serial.print("Manufacturer ID: 0x");
    Serial.println(manufacturer, HEX);
    Serial.print("Memory Type: 0x");
    Serial.println(memory_type, HEX);
    Serial.print("Capacity: 0x");
    Serial.println(capacity, HEX);

    switch(capacity) {
        case 0x15: 
            Serial.println("Size: 8Mb (1MB)");
            _capacity = 1 * 1024 * 1024;
            break;
        case 0x16: 
            Serial.println("Size: 16Mb (2MB)");
            _capacity = 2 * 1024 * 1024;
            break;
        case 0x17: 
            Serial.println("Size: 32Mb (4MB)");
            _capacity = 4 * 1024 * 1024;
            break;
        case 0x18: 
            Serial.println("Size: 64Mb (8MB)");
            _capacity = 8 * 1024 * 1024;
            break;
        case 0x19: 
            Serial.println("Size: 128Mb (16MB)");
            _capacity = 16 * 1024 * 1024;
            break;
        case 0x20: 
            Serial.println("Size: 256Mb (32MB)");
            _capacity = 32 * 1024 * 1024;
            _use32bitAddressing = true;
            break;
        case 0x21: 
            Serial.println("Size: 512Mb (64MB)");
            _capacity = 64 * 1024 * 1024;
            _use32bitAddressing = true;
            break;
        default: 
            Serial.println("Unknown size");
            break;
    }
}