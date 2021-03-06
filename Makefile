# Uses Arduino-Makefile from https://github.com/sudar/Arduino-Makefile
ARDMK_DIR = ~/src/Arduino-Makefile
ARDUINO_DIR = /Applications/Arduino.app/Contents/Java

# Installed using MacPorts
AVR_TOOLS_DIR = /opt/local
AVRDUDE = /opt/local/bin/avrdude

BOARD_TAG = uno
MONITOR_PORT = /dev/cu.usbmodem*

ASFLAGS += -I/opt/local/avr/include

COMMON_DEPS += macros.i pins.i text.S

include $(ARDMK_DIR)/Arduino.mk

text.S: tools/textgen.py
	$< >$@

clean::
	echo test
