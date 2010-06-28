gcc -o dfsound `pkg-config --cflags --libs glib-2.0` `xml2-config --cflags --libs` `sdl-config --cflags --libs` -lSDL_mixer -g src/main.c
