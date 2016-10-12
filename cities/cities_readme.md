The PNG files in this folder contain city maps based on real terrains. They are based on [OpenStreetMap](http://osm.org)'s map data and [Mapbox](https://mapbox.com/about/maps)'s custom styles (namely, a style that only displays rivers and parks/forests).

After that, the map is resized to 120x100 and edited in order to make rivers/water areas contiguous and occasionally moving/resizing islands/peninsulas so that they can fit a 3x3 building.

To use these maps properly, use the following command:

    snescityeditor -cx -I1 sram_file.srm map_file.png

Notes on specific cities:

* Kyiv's Dnieper River is much more complex than in most cities. Not only does it divide the city into two parts, it also creates a series of islands in the middle. I attempted to replicate said islands in the map, hence why it looks so weird compared to other cities. It also means that this map is less suited for the "-I3" option, as it will result in them being "flooded" beyond recognition.

* Moscow\_1.png and Moscow\_2.png represent two different zoom levels. The first one attempts to fit more of the city, while the second one aims to create a more playable central area (e.g. Balchug Island now can actually fit 3x3 buildings in it).
