#ifndef CITYEDIT_H
#define CITYEDIT_H
#include <stdint.h>

int city_improve (uint16_t* city, int improve_flags);

int vtile(int x, int y);

int city_water_fix (uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags);
int city_water_spread( uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags);
void put_proper_road(uint16_t* city, uint8_t ix, uint8_t iy);
void put_proper_rail(uint16_t* city, uint8_t ix, uint8_t iy);
void put_proper_power(uint16_t* city, uint8_t ix, uint8_t iy);
void city_fix_forests (uint16_t* city, uint8_t ix, uint8_t iy);
#endif
