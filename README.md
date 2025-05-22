# W25QXX Flash Memory Library for Arduino

![W25QXX Chip](https://www.winbond.com/resource-files/w25q32jv%20revj%2003272018%20plus.pdf)

A comprehensive Arduino library for interfacing with Winbond W25QXX series SPI flash memory chips. This library is based on code from [f4lcrum/W25Q32-Arduino-STM32](https://github.com/f4lcrum/W25Q32-Arduino-STM32) with significant improvements and additions.

## Features

- Supports W25Q series chips (8Mb to 512Mb)
- Automatic chip size detection
- Page read/write operations (256 bytes)
- Sector erase (4KB)
- Block operations
- Automatic handling of 24-bit vs 32-bit addressing
- Robust error handling

## Installation

1. Download the latest release from GitHub
2. Extract the ZIP file
3. Copy the `W25QXX` folder to your Arduino libraries folder
4. Restart Arduino IDE

## Usage

### Basic Example

```cpp
#include <SPI.h>
#include <W25QXX.h>

#define CS_PIN PA4  // Chip select pin

W25QXX flash(CS_PIN);

void setup() {
  Serial.begin(115200);
  flash.begin();
  
  // Detect chip size automatically
  flash.detectSize();
  
  // Example write/read operations
  uint8_t data[256];
  uint8_t readBuf[256];
  
  // Fill buffer with test data
  for(int i=0; i<256; i++) {
    data[i] = i;
  }
  
  // Write data to page 0
  flash.write(0, 0, 256, data);
  
  // Read back
  flash.read(0, 0, 256, readBuf);
  
  // Verify
  for(int i=0; i<256; i++) {
    if(data[i] != readBuf[i]) {
      Serial.println("Verification failed!");
      break;
    }
  }
  Serial.println("Operation completed successfully");
}

void loop() {
  // Your code here
}
```

## API Reference

### Core Functions

- `begin()` - Initialize the SPI interface
- `reset()` - Reset the flash chip
- `readJEDECID()` - Read manufacturer and device ID
- `readManufacturerDeviceID()` - Read additional device info
- `read(page, offset, length, buffer)` - Read data
- `write(page, offset, length, data)` - Write data
- `writeClean(page, offset, length, data)` - Write with automatic sector erase
- `eraseSector(sectorNum)` - Erase 4KB sector
- `detectSize()` - Auto-detect chip capacity

## Disclaimer

**Important Notice Regarding Use of This Software**

THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This library is based on code originally developed by f4lcrum and available at [https://github.com/f4lcrum/W25Q32-Arduino-STM32](https://github.com/f4lcrum/W25Q32-Arduino-STM32). While significant modifications and improvements have been made, the core functionality owes credit to this original work.

## Safety Warning

- Flash memory has limited write/erase cycles (typically 100,000 cycles)
- Always verify critical writes by reading back data
- Improper use can permanently damage your flash chip
- Ensure proper voltage levels for your specific chip
- Double-check connections before powering on

## Contributing

Contributions are welcome! Please open an issue or pull request on GitHub.

## License

This library is released under the MIT License. See LICENSE file for details.

## Acknowledgments

- Original code by f4lcrum ([W25Q32-Arduino-STM32](https://github.com/f4lcrum/W25Q32-Arduino-STM32))
- Winbond for their excellent flash memory products
- Arduino community for SPI library and support
