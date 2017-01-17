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

    snescityeditor -<ceif> [-2] [-x] sramfile.srm mapfile.png

The first argument sets the mode in which the program operates:

* **-c** will import a map from the PNG file and create an SRAM file from scratch based on said map. (WARNING: if an SRAM file under the specified name already exists, it will be overwritten!)

* **-e** will extract a map from the specified SRAM file and save it into the specified PNG file, overwriting it.

* **-i** will import a map from the PNG file and use it to replace the city map in the SRAM file.

* **-f** will update the checksums in an SRAM file to ensure it opens in the game. (The program also does this when using the -i switch. This switch is useful in case the user wants to manually modify the SRAM file.)

Optional switches are as follows:

* **-2** will make the program operate on the second city in the SRAM file. This switch is unnecessary when using -f, as it updates all checksums. This switch doesn't work with the -c command due to being unnecessary.

* **-n** allows to specify a city name (max. 8 characters) that will be used when creating a new SRAM file. If not specified, the city name will be based on the PNG map file name.

* **-x** will tell the game that the map needs to be "improved" before being imported into the game. This involves redrawing the roads, bridges, the coastline and the forests to make sure they look smooth. Given that, most of the time, the map is drawn with solid colors, this is practically necessary to achieve a good-looking map.

* **-I** allows to specify binary flags that can change how the improvement process works. To select the flags, add up the numbers listed below:

  1: draw a coastline from scratch instead of fixing the already existing one.

  2: draw a thicker, better-looking coastline (requires 1). Might result in small islands being shrunk or removed.
  
  4: (experimental algorithm) try to draw a thick coastline w/o overwriting sea tiles (requires 1). Maps imported this way may need manual editing.

  8: when drawing a thick coastline, check if the shore tiles fit each other. When using flags 1 and 2, results in less island shrinkage, but weirder looking maps.

## Graphical interface

When compiled with "SDL\_UI=1", the program opens with a menu providing different operations. This menu is still a work in progress, and there might be bugs, but it should work for the most basic needs.

You can select different options by clicking on them. After selecting an option, the program will ask for the SRAM and PNG files to be dropped into its window. The program may also ask which one of the two cities in the SRAM file should be loaded or to apply one of the few improvements/modifications to the map file. Be warned that it doesn't prevent the user from choosing an empty city - the program will still report success, but the imported city won't load anyway.

After all the parameters are specified, the program will either report "success" or "error".

### Screenshots

![Title Screen](https://i.imgur.com/cI4JMfP.png)

![Map Options Screen](https://i.imgur.com/bQQkXNB.png)

![Editor Screen](https://i.imgur.com/n4wvZpG.png)


# Bugs

The program only performs a simplified compression routine on maps when importing them into the game. This means that a complex map, including lots of buildings or complex river/forest patterns, might be impossible to import back.

The program doesn't report every single error when importing a map. If the map is malformed, it might result in a glitched city in the game.

It only modifies the city map and none of the other variables, so it might cause glitches when replacing a city map that already had buildings/citizens/etc. 

# Disclaimer

The program probably won't cause your computer to melt, but there's no definitive evidence it can't. Use it at your own risk.
