# snescityeditor

**snescityeditor** is a small program designed to create/edit maps for the Super NES version of SimCity by modifying SRAM save files.

# Compilation/Installation

**snescityeditor** only depends on one library, which is libpng. It should be easily available in any distribution's repositories.

The program compiles into a single executable file that doesn't require installation. Users that want to do so are welcome to copy the resulting file into /usr/local/bin or an equivalent directory manually.

## Compilation flags

By appending "SDL\_UI=1" to the **make** command, the user may try and compile a work-in-progress graphical version of this program. It depends on SDL2 and SDL\_image version 2.0.1 or later.

# Usage

## Command line

The program runs on a command line and takes arguments in a following format:

    snescityeditor -<eif> [-2] [-x] sramfile.srm mapfile.png

The first argument sets the mode in which the program operates:

* **-e** will extract a map from the specified SRAM file and save it into the specified PNG file, overwriting it.

* **-i** will import a map from the PNG file and use it to replace the city map in the SRAM file.

* **-f** will update the checksums in an SRAM file to ensure it opens in the game. (The program also does this when using the -i switch. This switch is useful in case the user wants to manually modify the SRAM file.)

Optional switches are as follows:

* **-2** will make the program operate on the second city in the SRAM file. This switch is unnecessary when usingg -f, as it updates all checksums.

* **-x** will set a level of improvement on maps that are imported into the game. Right now, it will redraw the roads/bridges, the coastline and the forests to make sure they look smooth (given that one is placed in the PNG map). This parameter is practically necessary when drawing maps from scratch, as different kinds of coastline/forest use slightly different shades of a similar color.

## Graphical interface

When compiled with "SDL\_UI=1", the program opens with a menu providing different operations. This menu doesn't provide the entire functionality of the command line version, and its output is very limited (it doesn't tell different errors apart or prevent a user from making bad decisions), but it should work for the most basic needs.

You can select different options by clicking on them. After selecting an option, the program will ask for the SRAM and PNG files to be dropped into its window. The program may also ask which one of the two cities in the SRAM file should be loaded. Be warned that it doesn't prevent the user from choosing an empty city - the program will still report success, but the imported city won't load anyway.

After all the parameters are specified, the program will either report "success" or "error". The only way from either of those screens is by closing the program window.

# Bugs

The program does not, as of yet, completely understand the decryption format used by the game in order to create SRAM files. It will most likely fail to extract maps from any savefiles that include buildings.

It only modifies the city map and none of the other variables, so it might cause glitches when replacing a city map that already had buildings/citizens/etc. 

# Disclaimer

This program does not generate SRAM files from scratch. These must be obtained manually. The program won't probably cause your computer to melt, but there's no definitive evidence it can't. Use it at your own risk.
