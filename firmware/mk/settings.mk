# build directory
BUILD_BASE	= build

# firmware directory
FW_BASE		= firmware

# name for the target project
TARGET		= app

# Base directory for the compiler
XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin

# base directory of the ESP8266 SDK package, absolute
SDK_BASE	?= c:/Espressif/ESP8266_SDK
SDK_TOOLS	?= c:/Espressif/utils/ESP8266

# Extra libs, include and ld file
EXTRA_BASE	?= c:/Espressif/extra

# esptool path and port
ESPTOOL		?= $(SDK_TOOLS)/esptool.exe
ESPPORT		?= COM3

# Baud rate for programmer
ESPBAUD		?= 256000

# Boot mode:
# Valid values are none, old, new
# none - non use bootloader
# old  - boot_v1.1
# new  - boot_v1.2+
BOOT ?= none

# Choose bin generate (0=eagle.flash.bin+eagle.irom0text.bin, 1=user1.bin, 2=user2.bin)
APP ?= 0

# Flash Frequency for ESP8266
# Clock frequency for SPI flash interactions.
# Valid values are 20, 26, 40, 80 (MHz).
SPI_SPEED ?= 40

# Flash Mode for ESP8266
# These set Quad Flash I/O or Dual Flash I/O modes.
# Valid values are  QIO, QOUT, DIO, DOUT
SPI_MODE ?= QIO

# Flash Size for ESP8266
# Size of the SPI flash, given in megabytes.
# ESP-12, ESP-12E and ESP-12F modules (and boards that use them such as NodeMCU, HUZZAH, etc.) usually
# have at least 4 megabyte / 4MB (sometimes labelled 32 megabit) flash.
# If using OTA, some additional sizes & layouts for OTA "firmware slots" are available.
# If not using OTA updates then you can ignore these extra sizes:
#
# Valid values vary by chip type: 1, 2, 3, 4, 5, 6, 7, 8, 9
#
#|SPI_SIZE_MAP|flash_size arg | Number of OTA slots | OTA Slot Size | Non-OTA Space |
#|------------|---------------|---------------------|---------------|---------------|
#|1           |256KB          | 1 (no OTA)          | 256KB         | N/A           |
#|2           |512KB          | 1 (no OTA)          | 512KB         | N/A           |
#|3           |1MB            | 2                   | 512KB         | 0KB           |
#|4           |2MB            | 2                   | 512KB         | 1024KB        |
#|5           |4MB            | 2                   | 512KB         | 3072KB        |
#|6           |2MB-c1         | 2                   | 1024KB        | 0KB           |
#|7           |4MB-c1         | 2                   | 1024KB        | 2048KB        |
#|8           |8MB [^]        | 2                   | 1024KB        | 6144KB        |
#|9           |16MB [^]       | 2                   | 1024KB        | 14336KB       |
#
# [^] Support for 8MB & 16MB flash size is not present in all ESP8266 SDKs. If your SDK doesn't support these flash sizes, use 4MB.
#
SPI_SIZE_MAP ?= 2
