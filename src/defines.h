#ifndef DEFINES_H
#define DEFINES_H

#ifdef NESMODE
#define CITYWIDTH 76
#define CITYHEIGHT 76
#else
#define CITYWIDTH 120
#define CITYHEIGHT 100
#endif

#define CITYAREA (CITYWIDTH * CITYHEIGHT)

#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x

#endif
