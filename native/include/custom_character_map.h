#ifndef CUSTOM_CHARACTER_MAP_H
#define CUSTOM_CHARACTER_MAP_H

#include <stdbool.h>
#include <stdint.h>

#define CUSTOM_CHARACTER_FIRST 31u
#define CUSTOM_CHARACTER_COLUMNS 8u
#define CUSTOM_SPRITE_FRAME_COUNT 6u
#define CUSTOM_SPRITE_WIDTH 7u

typedef struct {
    uint8_t advance;
    uint8_t columns[CUSTOM_CHARACTER_COLUMNS];
} CustomCharacterGlyph;

bool custom_character_map_lookup(uint8_t code, CustomCharacterGlyph *glyph);
const uint8_t *custom_character_map_sprite_frame(uint8_t frame_index);

#endif
