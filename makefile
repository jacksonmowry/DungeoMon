##
# Graphics
#
# @file
# @version 0.1
.SUFFIXES:

all: examples

examples: bin/font_rendering \
		bin/animation \
		bin/character_control \
		bin/sprites \
		bin/map \
		bin/map_editor

bin/font_rendering: examples/font_rendering.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/animation: examples/animation.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/character_control: examples/character_control.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/sprites: examples/sprites.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/map: examples/map.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/map_editor: examples/map_editor.c obj/timespec.o obj/sx.o obj/tile.o obj/vec.o obj/color.o obj/png_handler.o obj/events.o obj/map.o obj/map_layer.o obj/tile_selection_list.o obj/debug_layer.o obj/scrolling_text_layer.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng -lpthread

obj/timespec.o: src/timespec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/sx.o: src/sx.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude -lsixel -lpthread

obj/tile.o: src/tile.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/vec.o: src/vec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/color.o: src/color.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/png_handler.o: src/png_handler.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude -lpng

obj/events.o: src/events.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/map.o: src/map.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/map_layer.o: src/map_layer.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/tile_selection_list.o: src/tile_selection_list.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/debug_layer.o: src/debug_layer.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/scrolling_text_layer.o: src/scrolling_text_layer.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

clean:
	rm -f bin/* obj/*
# end
