#master makefile to make some or all deliverables (no 64bit 'min' ones yet, and no 'min' built by default)

all: txall

txall: release debug m64r m64d

txmin: minr mind min64r min64d

release:
	make -C _txall_/macr

debug:
	make -C _txall_/macd

m64r:
	make -C _txall_/m64r

m64d:
	make -C _txall_/m64d

minr:
	make -C _txmin_/macr

mind:
	make -C _txmin_/macd

min64r:
	make -C _txmin_/m64r

min64d:
	make -C _txmin_/m64d

clean:
	make -C _txall_/macr clean
	make -C _txall_/macd clean
	make -C _txall_/m64r clean
	make -C _txall_/m64d clean
	make -C _txmin_/macr clean
	make -C _txmin_/macd clean
	make -C _txmin_/m64r clean
	make -C _txmin_/m64d clean

