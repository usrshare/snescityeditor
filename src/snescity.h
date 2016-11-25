#ifndef SNESCITY_H
#define SNESCITY_H
#include <stdint.h>

extern char* city_lasterror;

int find_png_filename(const char* filename, char* o_f);

int loadsramcity (const char* sfname, uint16_t* citydata, int citynum);
int write_new_city(const char* sfname, const uint16_t* citydata, const char* cityname, int citynum);
int replace_city(const char* sfname, const uint16_t* citydata, int citynum);

int newcity(const char* sfname, const char* mfname, const char* cityname, int improve, int improve_flags);
int png2city (const char* sfname, const char* mfname, int citynum, int improve, int improve_flags);

int city_improve (uint16_t* city, int improve_flags);

int city2png (const char* sfname, const char* mfname, int citynum);
int fixsram (const char* sfname);

int describe_cities (const char* sfname, char* city1, char* city2); 

#endif
