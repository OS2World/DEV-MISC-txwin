#master makefile to make some or all deliverables (64bit debug built by default)

one: m64d

all: release debug m64r m64d

release:
	make -C _build_/macr

debug:
	make -C _build_/macd

m64r:
	make -C _build_/m64r

m64d:
	make -C _build_/m64d

clean:
	make -C _build_/macr clean
	make -C _build_/macd clean
	make -C _build_/m64r clean
	make -C _build_/m64d clean
