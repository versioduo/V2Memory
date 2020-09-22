#include "V2Memory.h"

uint8_t V2Memory::Flash::getBank() {
  return !NVMCTRL->STATUS.bit.AFIRST;
}

void V2Memory::Flash::eraseBlock(uint32_t offset) {
  while (!NVMCTRL->STATUS.bit.READY)
    ;

  NVMCTRL->ADDR.reg  = offset;
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
}

void V2Memory::Flash::writePage(uint32_t offset, const uint32_t *data) {
  // Manual page write (default mode).
  NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

  while (!NVMCTRL->STATUS.bit.READY)
    ;

  // Clear page buffer.
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;

  while (!NVMCTRL->STATUS.bit.READY)
    ;

  // Perform 32 bit writes to the page buffer
  volatile uint32_t *target = (volatile uint32_t *)offset;
  const uint32_t *source    = (const uint32_t *)data;
  for (uint32_t i = 0; i < getPageSize() / 4; i++)
    target[i] = source[i];

  while (!NVMCTRL->STATUS.bit.READY)
    ;

  // Write the page to flash.
  NVMCTRL->ADDR.reg  = offset;
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WP;
}

void V2Memory::Flash::writeBlock(uint32_t offset, const uint32_t *data) {
  eraseBlock(offset);

  for (uint32_t i = 0; i < getBlockSize(); i += getPageSize())
    writePage(offset + i, data + (i / sizeof(uint32_t)));

  while (!NVMCTRL->STATUS.bit.READY)
    ;
}

void V2Memory::Flash::UserPage::read(uint32_t data[128]) {
  memcpy(data, (const void *)NVMCTRL_USER, 512);
}

// In case of failure, known working values (other devices might contain
// different factory-calibrated values)  are:
//   0xFE9A9239
//   0xAEECFF80
//   0xFFFFFFFF
//   0xFFFFFFFF (user)
//   0x00804010
void V2Memory::Flash::UserPage::write(uint32_t data[128]) {
  while (NVMCTRL->STATUS.bit.READY == 0)
    ;

  // Manual write
  NVMCTRL->CTRLA.bit.WMODE = NVMCTRL_CTRLA_WMODE_MAN;

  // Erase page
  NVMCTRL->ADDR.reg  = (uint32_t)NVMCTRL_USER;
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EP;
  while (NVMCTRL->STATUS.bit.READY == 0)
    ;

  // Page buffer clear
  NVMCTRL->ADDR.reg  = (uint32_t)NVMCTRL_USER;
  NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
  while (NVMCTRL->STATUS.bit.READY == 0)
    ;

  // Write page
  uint32_t *addr = (uint32_t *)NVMCTRL_USER;
  for (uint8_t i = 0; i < 128; i += 4) {
    addr[i + 0] = data[i + 0];
    addr[i + 1] = data[i + 1];
    addr[i + 2] = data[i + 2];
    addr[i + 3] = data[i + 3];

    // Write quad word (128 bits)
    NVMCTRL->ADDR.reg  = (uint32_t)(addr + i);
    NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WQW;
    while (NVMCTRL->STATUS.bit.READY == 0)
      ;
  }
}
