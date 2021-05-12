// © Kay Sievers <kay@vrfy.org>, 2020-2021
// SPDX-License-Identifier: Apache-2.0

#include "V2Memory.h"

extern "C" char *sbrk(int incr);
uint32_t V2Memory::RAM::getFree() {
  uint8_t top;
  return &top - reinterpret_cast<uint8_t *>(sbrk(0));
}
