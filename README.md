# snescityeditor

**snescityeditor** is a small program designed to create/edit maps for the NES and Super NES versions of SimCity by modifying SRAM save files.

# Compilation/Installation

**snescityeditor**'s primary functionality only depends on one library, which is libpng. It should be easily available in any distribution's repositories. The graphical interface also depends on SDL2 and SDL\_image (2.0.1 or later). It's enabled by default, but can be removed by editing the **Makefile.cfg** file and setting "SDL\_UI" to 0.

To pick whether the SNES or NES game should be used, set the "NESMODE" variable inside the **Makefile.cfg** file to 0 or 1 respectively.

The program compiles into a single executable file that doesn't require installation. Users that want to do so are welcome to copy the resulting file into /usr/local/bin or an equivalent directory manually.

(Even if the program is compiled with the graphical interface, the command line parameters will still work.)

# Usage

## Graphical interface

By default, the program opens with a menu providing different operations. This menu is still a work in progress, and there might be bugs, but it should work for the most basic needs.

The graphical interface is operated with a mouse. A keyboard can also be used (but isn't necessary) on the city rename screen.

The user can either open the map editor to create a new map from scratch or load an existing map from an SRAM file or a PNG map file. To load existing SRAM or PNG files into the program, their icons have to be dropped into the window. (This was the easiest to implement cross-platform way of getting a file path that didn't involve implementing a file manager.)

In the map editor, left-clicking places tiles and right-clicking scrolls the map around.

The options screen (shown on loading a map and accessible by clicking the gear-shaped icon in the editor) allows to set the city's in-game name, as well as apply several transformations to the map to improve its appearance.

After all the parameters are specified and a "save" option is chosen, the program will report either "success" or "error", with an error description if one is available.

### Screenshots

![Title Screen](https://i.imgur.com/cI4JMfP.png)

![Map Options Screen](https://i.imgur.com/bQQkXNB.png)

![Editor Screen](https://i.imgur.com/n4wvZpG.png)

## Command line

The program can also run on a command line, in which case it takes arguments in a following format:

    snescityeditor -<ceif> [-2] [-x] sramfile.srm mapfile.png

The first argument sets the mode in which the program operates:

* **-c** will import a map from the PNG file and create an SRAM file from scratch based on said map. (WARNING: if an SRAM file under the specified name already exists, it will be overwritten!)

* **-e** will extract a map from the specified SRAM file and save it into the specified PNG file, overwriting it.

* **-i** will import a map from the PNG file and use it to replace the city map in the SRAM file.

* **-f** will update the checksums in an SRAM file to ensure it opens in the game. (The program also does this when using the -i switch. This switch is useful in case the user wants to manually modify the SRAM file.)

Optional switches are as follows:

* **-2** will make the program operate on the second city in the SRAM file. This switch is unnecessary when using -f, as it updates all checksums. This switch doesn't work with the -c command due to being unnecessary.

* **-n** allows to specify a city name (max. 8 (SNES) or 10(NES) characters) that will be used when creating a new SRAM file. If not specified, the city name will be based on the PNG map file name.

* **-x** will tell the game that the map needs to be "improved" before being imported into the game. This involves redrawing the roads, bridges, the coastline and the forests to make sure they look smooth. Given that, most of the time, the map is drawn with solid colors, this is practically necessary to achieve a good-looking map.

* **-I** allows to specify binary flags that can change how the improvement process works. To select the flags, add up the numbers listed below:

  1: draw a coastline from scratch instead of fixing the already existing one.

  2: draw a thicker, better-looking coastline (requires 1). Might result in small islands being shrunk or removed.
  
  4: (experimental algorithm) try to draw a thick coastline w/o overwriting sea tiles (requires 1). Maps imported this way may need manual editing.

  8: when drawing a thick coastline, check if the shore tiles fit each other. When using flags 1 and 2, results in less island shrinkage, but weirder looking maps.

# Bugs

**SNES**: The program only performs a simplified compression routine on maps when importing them into the game. This means that a complex map, including lots of buildings or complex river/forest patterns, might be impossible to import back.

The program doesn't report every single error when importing a map. If the map is malformed, it might result in a glitched city in the game.

It only modifies the city map and none of the other variables, so it might cause glitches when replacing a city map that already had buildings/citizens/etc. 

# Acknowledgments

**NES**: Information on how the data in the SRAM file is located is, in part, based on [Cah4e3](http://cah4e3.shedevr.org.ru/)'s disassembly of the game.

# Disclaimer

The program shouldn't cause your computer to melt, but there's no definitive evidence it can't. Use it at your own risk.
