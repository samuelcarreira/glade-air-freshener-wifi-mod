#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <stdio.h>

uint32_t calculateCRC32(const uint8_t *data, size_t length);

String macToString(const unsigned char *mac);

#endif  // UTILS_H