##
# Graphics
#
# @file
# @version 0.1

all: examples

examples: bin/font_rendering \
		bin/animation \
		bin/character_control

bin/font_rendering: examples/font_rendering.c obj/timespec obj/sx obj/tile obj/vec
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel

bin/animation: examples/animation.c obj/timespec obj/sx obj/tile obj/vec
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel

bin/character_control: examples/character_control.c obj/timespec obj/sx obj/tile obj/vec
	$(CC) $(CFLAGS) $^ -o $@ -lm -Iinclude -lsixel

obj/timespec: src/timespec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/sx: src/sx.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude -lsixel

obj/tile: src/tile.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

obj/vec: src/vec.c
	$(CC) $(CFLAGS) $^ -c -o $@ -Iinclude

clean:
	rm -f bin/* obj/*
# end
