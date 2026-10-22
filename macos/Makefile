# Builds universal (Intel + Apple Silicon) binaries into bin/.
# Needs the Xcode Command Line Tools: xcode-select --install

ARCHS  = -arch x86_64 -arch arm64
CFLAGS = -O2 $(ARCHS) -mmacosx-version-min=11.0
JBIG   = src/jbig/jbig.c src/jbig/jbig_ar.c

all: bin/foo2hbpl1 bin/rastertoppm

bin/foo2hbpl1: src/foo2hbpl1.c $(JBIG)
	@mkdir -p bin
	cc $(CFLAGS) -w -Isrc/jbig -o $@ src/foo2hbpl1.c $(JBIG)

bin/rastertoppm: src/rastertoppm.c
	@mkdir -p bin
	cc $(CFLAGS) -Wno-deprecated-declarations -o $@ src/rastertoppm.c -lcups

clean:
	rm -rf bin

.PHONY: all clean
