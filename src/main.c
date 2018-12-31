// vim: sw=4
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "defines.h"
#include "sramcity.h"
#include "cityedit.h"
#include "pngmap.h"

#ifdef USE_SDL_UI
#include "sdl_ui.h"
#include "ui_menu.h"
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef __CYGWIN__
// fixing a cygwin error
extern char *optarg;
extern int optind, opterr, optopt;
#endif

enum prgmode {
    MODE_NONE,
    MODE_FIX, //do not replace map, only fix cksum
    MODE_EXPORT, //save map from city into PNG
    MODE_IMPORT, //replace map from PNG into city
    MODE_CREATE, //create new SRAM file from PNG map
};

void exit_usage_error(char** argv) {
    printf("Usage: %s -<ceif> [-2] [-x #] [-n cityname] snescity.srm [citymap.png]\n"
	    " -c: create an SRAM file based on PNG map\n"
	    " -e: export map from SRAM into PNG\n"
	    " -i: import map from PNG into SRAM\n"
	    " -f: fix SRAM file's checksum\n"
	    " -2: operate on the second city\n"
	    " -n cityname: specify the name for the newly-created city.\n"
	    " -x: fix shores, forests and roads when importing.\n"
	    " -I #: set additional flags for usage with -x\n"
	    "\n",argv[0]); exit(1);}

    int main (int argc, char** argv) {

#ifdef _WIN32
	setvbuf(stdout,NULL,_IONBF,0); //disable buffering for windows
	//timeBeginPeriod(5); //set timer frequency to slightly higher for smoother framerates
#endif

#ifdef USE_SDL_UI
	if (argc == 1) return sdl_ui_main(ui_initfunc, ui_updatefunc);
#endif

	enum prgmode mode = MODE_NONE;
	int citynum = 0;
	int improve = 0;

	int improve_flags = 0;
	char cityname[CITYNAME_MAX + 1];
	memset(cityname, 0, sizeof cityname);

	int c = -1;
	while ( (c = getopt(argc,argv,"ceiI:f2xn:")) != -1) {
	    switch(c) {

		case 'c':
		    mode = MODE_CREATE;
		    break;
		case 'e':
		    mode = MODE_EXPORT;
		    break;
		case 'i':
		    mode = MODE_IMPORT;
		    break;
		case 'I':
		    improve_flags = atoi(optarg);
		    break;
		case 'n':
		    strncpy(cityname,optarg,8);
		    for (int i=0; i < 8; i++) cityname[i] = toupper(cityname[i]);
		    break;
		case 'f':
		    mode = MODE_FIX;
		    break;
		case 'x':
		    improve = 1;
		    break;
		case '2':
		    citynum = 1;
		    break;
	    }
	}

	if (mode == MODE_NONE) exit_usage_error(argv);

	const char* sfname = argv[optind];
	const char* mfname = argv[optind+1];

	if ((mfname) && (strlen(cityname) == 0)) {
	    find_png_filename(mfname,cityname);
	    for (int i=0; i < 8; i++) cityname[i] = toupper(cityname[i]);
	}

	switch(mode) {
	    case MODE_CREATE: {
		if ((!sfname) || (!mfname)) exit_usage_error(argv);

		uint16_t citydata[CITYWIDTH * CITYHEIGHT];
		int r = read_png_map(mfname, citydata);	

		if (r != 0) {
		    fprintf(stderr,"Failed to read the PNG city map.\n");
		    return 1;
		}

		if (improve) city_improve(citydata, improve_flags);

		return write_new_city(sfname,citydata,cityname,0);
		break; }
	    case MODE_EXPORT: {
		if ((!sfname) || (!mfname)) exit_usage_error(argv);

		uint16_t citydata[CITYWIDTH * CITYHEIGHT];

		int r = loadsramcity(sfname, citydata, citynum, cityname);
		if (r != 0) {
		    fprintf(stderr,"Failed to read the SRAM file.\n");
		    return 1;
		}

		return write_png_map(mfname, citydata);
		break; }
	    case MODE_IMPORT: {
		if ((!sfname) || (!mfname)) exit_usage_error(argv);

		uint16_t citydata[CITYWIDTH * CITYHEIGHT];

		int r = read_png_map(mfname, citydata);	
		if (r != 0) {
		    fprintf(stderr,"Failed to read the PNG city map.\n");
		    city_lasterror = "Can not read the PNG.\n";
		    return 1;
		}

		if (improve) city_improve(citydata, improve_flags);

		return replace_city(sfname,citydata,0);
		//png2city(sfname,mfname,citynum,improve,improve_flags);
		break; }
	    case MODE_FIX:
		if ((!sfname)) exit_usage_error(argv);
		fixsram(sfname);
	    default:
		break;
	}
	return 0;
    }
