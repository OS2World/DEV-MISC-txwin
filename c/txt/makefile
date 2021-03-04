#master makefile to make some or all deliverables (64bit debug built by default)

one: m64d

all: m64r m64d

m64r:
	make -C _build_/m64r

m64d:
	make -C _build_/m64d

clean:
	make -C _build_/m64r clean
	make -C _build_/m64d clean
