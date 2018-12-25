#ifndef SNESCITY_H
#define SNESCITY_H
#include <stdint.h>

#ifdef SCE_EXPMODE
#define NUMBER_OF_CITIES 1
#else
#define NUMBER_OF_CITIES 2
#endif

extern char* city_lasterror;

int find_png_filename(const char* filename, char* o_f);

int loadsramcity (const char* sfname, uint16_t* citydata, int citynum, char* o_cityname);
int write_new_city(const char* sfname, const uint16_t* citydata, const char* cityname, int citynum);
int replace_city(const char* sfname, const uint16_t* citydata, int citynum);

int newcity(const char* sfname, const char* mfname, const char* cityname, int improve, int improve_flags);
int png2city (const char* sfname, const char* mfname, int citynum, int improve, int improve_flags);

int city_improve (uint16_t* city, int improve_flags);

int city2png (const char* sfname, const char* mfname, int citynum);
int fixsram (const char* sfname);

int describe_cities (const char* sfname, char* city1, char* city2); 
int vtile(int x, int y);

int city_water_fix (uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags);
int city_water_spread( uint16_t* city, uint8_t ix, uint8_t iy, uint16_t v, int improve_flags);
void put_proper_road(uint16_t* city, uint8_t ix, uint8_t iy);
void put_proper_rail(uint16_t* city, uint8_t ix, uint8_t iy);
void put_proper_power(uint16_t* city, uint8_t ix, uint8_t iy);
void city_fix_forests (uint16_t* city, uint8_t ix, uint8_t iy);

#endif
