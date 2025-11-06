##
# Graphics
#
# @file
# @version 0.1

all: examples

examples: bin/font_rendering \
		bin/animation \
		bin/character_control \
		bin/sprites \
		bin/map \
		bin/map_editor

objs: obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler

bin/font_rendering: examples/font_rendering.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/animation: examples/animation.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/character_control: examples/character_control.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/sprites: examples/sprites.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/map: examples/map.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

bin/map_editor: examples/map_editor.c obj/timespec obj/sx obj/tile obj/vec obj/color obj/png_handler
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel -lpng

obj/timespec: src/timespec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/sx: src/sx.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude -lsixel

obj/tile: src/tile.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/vec: src/vec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/color: src/color.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/png_handler: src/png_handler.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude -lpng

clean:
	rm -f bin/* obj/*
# end
