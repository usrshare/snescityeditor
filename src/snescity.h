#ifndef SNESCITY_H
#define SNESCITY_H

extern char* city_lasterror;

int newcity(const char* sfname, const char* mfname, const char* cityname, int improve, int improve_flags);
int png2city (const char* sfname, const char* mfname, int citynum, int improve, int improve_flags);

int city2png (const char* sfname, const char* mfname, int citynum);
int fixsram (const char* sfname);

int describe_cities (const char* sfname, char* city1, char* city2); 

#endif
