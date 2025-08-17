all: zig-out/lib/libfence.a

zig-out/lib/libfence.a: build.zig src/root.zig src/fence.h
	zig build

clean:
	rm -rf zig-out
