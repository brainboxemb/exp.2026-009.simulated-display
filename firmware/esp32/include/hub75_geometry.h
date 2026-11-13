#pragma once

#include <stdint.h>

// Five physical 64x32 HUB75 panels are chained and mounted in portrait.
// On the front they form a 160x64 landscape display.
// Equivalent to CHAIN_TOP_RIGHT_DOWN + rotation 1 (Experiment 012).
namespace hub75_geometry {
constexpr uint16_t kPanelWidth = 64;
constexpr uint16_t kPanelHeight = 32;
constexpr uint16_t kPanelCount = 5;
constexpr uint16_t kLogicalWidth = kPanelHeight * kPanelCount; // 160
constexpr uint16_t kLogicalHeight = kPanelWidth;              // 64
constexpr uint16_t kDmaWidth = kPanelWidth * kPanelCount;     // 320
constexpr uint16_t kDmaHeight = kPanelHeight;                 // 32

struct Point {
    uint16_t x;
    uint16_t y;
};

inline bool logicalToDma(uint16_t x, uint16_t y, Point &out) {
    if (x >= kLogicalWidth || y >= kLogicalHeight) {
        return false;
    }
    const uint16_t virtualX = y;
    const uint16_t virtualY = kLogicalWidth - 1 - x;
    const uint16_t row = virtualY / kPanelHeight;
    const uint16_t rowY = virtualY % kPanelHeight;

    if ((row & 1U) != 0U) {
        out.x = (kPanelCount - row) * kPanelWidth - 1 - virtualX;
        out.y = kPanelHeight - 1 - rowY;
    } else {
        out.x = (kPanelCount - row - 1) * kPanelWidth + virtualX;
        out.y = rowY;
    }
    return true;
}
} // namespace hub75_geometry
