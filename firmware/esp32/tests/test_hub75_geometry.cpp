#include "hub75_geometry.h"

#include <array>
#include <cassert>
#include <cstdio>

int main() {
    using namespace hub75_geometry;
    static_assert(kLogicalWidth == 160 && kLogicalHeight == 64, "160x64 required");
    static_assert(kDmaWidth == 320 && kDmaHeight == 32, "320x32 required");
    std::array<bool, kDmaWidth * kDmaHeight> visited{};
    for (uint16_t x = 0; x < kLogicalWidth; ++x) {
        for (uint16_t y = 0; y < kLogicalHeight; ++y) {
            Point p{};
            assert(logicalToDma(x, y, p));
            assert(p.x < kDmaWidth && p.y < kDmaHeight);
            assert(p.x / kPanelWidth == x / kPanelHeight);
            const size_t i = p.y * kDmaWidth + p.x;
            assert(!visited[i]);
            visited[i] = true;
        }
    }
    for (const bool used : visited) {
        assert(used);
    }
    Point p{};
    assert(logicalToDma(0, 0, p) && p.x == 0 && p.y == 31);
    assert(logicalToDma(159, 63, p) && p.x == 319 && p.y == 0);
    assert(!logicalToDma(160, 0, p));
    assert(!logicalToDma(0, 64, p));
    puts("PASS: five-panel 160x64 HUB75 mapping");
}
