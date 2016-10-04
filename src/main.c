#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "snescity.h"

#ifdef USE_SDL_UI
#include "sdl_ui.h"
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
	printf("Usage: %s -<ceif> [-2] [-x #] snescity.srm [citymap.png]\n"
			" -c: create an SRAM file based on PNG map\n"
			" -e: export map from SRAM into PNG\n"
			" -i: import map from PNG into SRAM\n"
			" -f: fix SRAM file's checksum\n"
			" -2: operate on the second city\n"
			" -x: fix shores, forests and roads when importing.\n"
			"\n",argv[0]); exit(1);}

	int main (int argc, char** argv) {

#ifdef USE_SDL_UI
		if (argc == 1) return sdl_ui_main();
#endif

		enum prgmode mode = MODE_NONE;
		int citynum = 0;
		int improve = 0;




		int c = -1;
		while ( (c = getopt(argc,argv,"ceif2x")) != -1) {
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

		switch(mode) {
			case MODE_CREATE:
				fprintf(stderr,"Create mode not implemented yet.\n");
				break;
			case MODE_EXPORT:
				if ((!sfname) || (!mfname)) exit_usage_error(argv);
				city2png(sfname,mfname,citynum);
				break;
			case MODE_IMPORT:
				if ((!sfname) || (!mfname)) exit_usage_error(argv);
				png2city(sfname,mfname,citynum,improve);
				break;
			case MODE_FIX:
				if ((!sfname)) exit_usage_error(argv);
				fixsram(sfname);
			default:
				break;
		}
		return 0;
	}
