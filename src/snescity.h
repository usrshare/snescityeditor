#ifndef SNESCITY_H
#define SNESCITY_H

int city2png (const char* sfname, const char* mfname, int citynum);
int png2city (const char* sfname, const char* mfname, int citynum, int improve);
int fixsram (const char* sfname);

int describe_cities (const char* sfname, char* city1, char* city2); 

#endif
