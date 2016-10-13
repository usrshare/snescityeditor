#ifndef PNGMAP_H
#define PNGMAP_H
#include <stdint.h>
#include <stdlib.h>

extern uint32_t pngcolors[];
extern const size_t pngcolor_c;

int write_png_map (const char* pngname, const uint16_t* citydata);
int read_png_map (const char* pngname, uint16_t* citydata);

#endif

