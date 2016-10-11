The PNG files in this folder contain city maps based on real terrains. They are based on OpenStreetMap (http://osm.org)'s map data and Mapbox (https://mapbox.com/about/maps)'s custom styles (namely, a style that only displays rivers and parks/forests).

After that, the map is resized to 120x100 and edited in order to make rivers/water areas contiguous.

To import these maps properly, use the following command:

    snescityeditor -ix -I1 sram_file.srm map_file.png