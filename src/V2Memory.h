#pragma once

#include <Arduino.h>

namespace V2Memory {
class RAM {
public:
  static constexpr uint32_t getSize() {
    return HSRAM_SIZE;
  }
  static uint32_t getFree();
};

class Flash {
public:
  static constexpr uint32_t getSize() {
    return FLASH_SIZE;
  }
  static constexpr uint32_t getPageSize() {
    return NVMCTRL_PAGE_SIZE;
  }
  static constexpr uint32_t getBlockSize() {
    return NVMCTRL_BLOCK_SIZE;
  }

  // Returns the currently active flash bank. Firmware::Secondary::activate swaps them.
  static uint8_t getBank();

  // The offset needs to be at a page boundary, data needs to be a full page (e.g. 512).
  static void writePage(uint32_t offset, const uint32_t *data);

  // The offset needs to be at a block boundary, data needs to be a full block (e.g. 8k).
  static void eraseBlock(uint32_t offset);
  static void writeBlock(uint32_t offset, const uint32_t *data);

  // "The size of the NVM User Page is 512 Bytes. The first eight 32-bit words (32 Bytes)
  // of the Non Volatile Memory (NVM) User Page contain calibration data that are automatically
  // read at device power on. The remaining 480 Bytes can be used for storing custom parameters."
  class UserPage {
  public:
    static constexpr uint32_t getStart() {
      return NVMCTRL_USER;
    }
    static void read(uint32_t data[128]);
    static void write(uint32_t data[128]);
  };
};

class Firmware {
public:
  // The start of the firmware area, the end of the bootloader area.
  static uint32_t getStart();

  // The size of the current firmware area, excluding the bootloader.
  static uint32_t getSize();

  // Calculate the hash of the given memory area.
  static void calculateHash(uint32_t offset, uint32_t len, char hash[41]);

  class Secondary {
  public:
    // The secondary flash bank, mapped to the second half of the flash area.
    static constexpr uint32_t getStart() {
      return Flash::getSize() / 2;
    }

    // Write a firmware block into the secondary flash bank, the offset starts after the bootloader area.
    static void writeBlock(uint32_t offset, const uint32_t *data);

    // Copy the current bootloader to the secondary flash bank.
    static void copyBootloader();

    // Check if the bootloader is identical, and the secondary firmware matches the given hash.
    static bool verify(uint32_t firmware_len, const char *hash);

    // Swap the flash banks, reallocate the EEPROM area, reset the system.
    static void activate();
  };
};

class EEPROM {
public:
  // The base address of the EEPROM data.
  static constexpr uint32_t getStart() {
    return SEEPROM_ADDR;
  }

  // Virtual/useable size of the EEPROM area.
  static uint32_t getSize();

  // Allocated flash space to emulate the EEPROM.
  static uint32_t getSizeAllocated();

  // Enables the SEEPROM buffered mode. Touched pages are only written to flash
  // when the write crosses a page boundary. flush() needs to be called to write
  // the data to the flash.
  //
  // Waits until the SEEPROM becomes ready for writing.
  static void prepareWrite();

  // Flushes the buffered data (current page) to the flash.
  static void flush();

  static void write(uint32_t offset, const uint8_t *buffer, uint32_t size);

  // Overwrite the entire flash area with 0xff.
  static void erase();
};
}; // namespace V2Memory
