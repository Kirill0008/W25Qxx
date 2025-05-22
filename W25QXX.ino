#include <SPI.h>
#include "W25QXX.h"

// Пины SPI (зависит от вашей платы)
#define PIN_SPI_SS PA4

W25QXX flash(PIN_SPI_SS); // По умолчанию 16MB (можно указать другой размер)

uint8_t readBuffer[256];
uint8_t writeBuffer[256];

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    flash.begin();
    
    // Определяем размер чипа автоматически
    flash.detectSize();
    
    // Читаем идентификаторы
    uint32_t jedec_id = flash.readJEDECID();
    uint32_t manufacturer_id = flash.readManufacturerDeviceID();
    
    Serial.print("JEDEC ID: 0x");
    Serial.println(jedec_id, HEX);
    Serial.print("Manufacturer Device ID: 0x");
    Serial.println(manufacturer_id, HEX);
    
    // Стираем сектор 0 (первые 4KB)
    Serial.println("Erasing sector 0...");
    flash.eraseSector(0);
    
    // Заполняем буфер для записи
    for (int i = 0; i < 256; i++) {
        writeBuffer[i] = i;
    }
    
    // Записываем данные (обычная запись)
    Serial.println("Writing data...");
    flash.write(0, 0, 256, writeBuffer); // Страница 0, смещение 0, 256 байт
    
    // Читаем обратно
    Serial.println("Reading data...");
    flash.read(0, 0, 256, readBuffer);
    
    // Выводим первые 32 байта для проверки
    Serial.println("First 32 bytes:");
    for (int i = 0; i < 32; i++) {
        Serial.print(readBuffer[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0) Serial.println();
    }
    
    // Тестируем writeClean (с автоматическим стиранием)
    Serial.println("Testing writeClean...");
    for (int i = 0; i < 256; i++) {
        writeBuffer[i] = 255 - i;
    }
    flash.writeClean(1, 0, 256, writeBuffer); // Страница 1
    
    // Читаем обратно
    flash.read(1, 0, 256, readBuffer);
    Serial.println("First 32 bytes after writeClean:");
    for (int i = 0; i < 32; i++) {
        Serial.print(readBuffer[i], HEX);
        Serial.print(" ");
        if ((i + 1) % 16 == 0) Serial.println();
    }
}

void loop() {
    // Можно добавить периодические операции здесь
    delay(5000);
}