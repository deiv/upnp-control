

firmware:
	cd esp-firmware && make test ESPPORT=/dev/ttyUSB0 &&  ../../utils/filteroutput.py --port /dev/ttyUSB0 --baud 115200 --elf ./build/upnp_test.out

tests:
	cd tests && mvn test

.PHONY: firmware tests
