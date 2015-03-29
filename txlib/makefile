#master makefile to make some or all deliverables

all: txall txmin

txall: trace release debug

txmin: mint minr mind

trace:
	make -C _txall_/mact

release:
	make -C _txall_/macr

debug:
	make -C _txall_/macd

mint:
	make -C _txmin_/mact

minr:
	make -C _txmin_/macr

mind:
	make -C _txmin_/macd


clean:
	make -C _txall_/mact clean
	make -C _txall_/macr clean
	make -C _txall_/macd clean
	make -C _txmin_/mact clean
	make -C _txmin_/macr clean
	make -C _txmin_/macd clean

