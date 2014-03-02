
# Select 'mega' for the 1280 APM1, 'mega2560' otherwise
BOARD = V2R

# HAL_BOARD determines default HAL target.
HAL_BOARD ?= HAL_BOARD_V2R

# The communication port used to communicate with the APM.
PORT = /dev/ttyACM0

# PX4Firmware tree: fill in the path to PX4Firmware repository from github.com/diydrones:
PX4_ROOT=../PX4Firmware

# PX4NuttX tree: fill in the path to PX4NuttX repository from github.com/diydrones:
NUTTX_SRC=../PX4NuttX/nuttx

V2R_SDK_ROOT ?= /path/to/virt2real-sdk
