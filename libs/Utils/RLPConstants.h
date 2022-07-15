#pragma once

namespace Utils::RLP
{
static constexpr uint8_t SINGLE_BYTE_PREFIX = 128;      // 128 dec == 0x80 hex
static constexpr uint8_t SHORT_STRING_PREFIX = 128;     // 128 dec == 0x80 hex
static constexpr uint8_t LONG_STRING_PREFIX = 183;      // 183 dec == 0xb7

static constexpr uint8_t SHORT_LIST_PREFIX = 192;       // 192 dec == 0xc0
static constexpr uint8_t LONG_LIST_PREFIX = 247;       // 247 dec == 0xf7

static constexpr uint8_t SHORT_STRING_MAX_WIDTH = 55;
static constexpr uint8_t SHORT_LIST_MAX_SIZE = 55;
}