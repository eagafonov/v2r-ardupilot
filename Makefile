all: notify

TMPDIR:=$(shell pwd)/build

mpu6050:
	make -C	libraries/AP_InertialSensor/examples/MPU6000  TMPDIR=$(TMPDIR) upload

motor:
	make -C	libraries/AP_Motors/examples/AP_Motors_test TMPDIR=$(TMPDIR) upload

gcs_console:
	make -C	libraries/GCS_Console/examples/Console TMPDIR=$(TMPDIR) upload

copter:
	make -C	ArduCopter  TMPDIR=$(TMPDIR) upload
menu:
	make -C	libraries/AP_Menu/examples/menu TMPDIR=$(TMPDIR) upload

compass:
	make -C	libraries/AP_Compass/examples/AP_Compass_test TMPDIR=$(TMPDIR) upload

baro:
	make -C	libraries/AP_Baro/examples/AP_Baro_BMP085_test TMPDIR=$(TMPDIR) upload

gps_console:
	make -C libraries/AP_GPS/examples/GPS_UBLOX_passthrough TMPDIR=$(TMPDIR) upload

gps_ublox:
	make -C	libraries/AP_GPS/examples/GPS_UBLOX_test TMPDIR=$(TMPDIR) upload

gps_nmea:
	make -C libraries/AP_GPS/examples/GPS_NMEA_test TMPDIR=$(TMPDIR) upload

gps_gpsd:
	make -C libraries/AP_GPS/examples/GPS_GPSD_test TMPDIR=$(TMPDIR) upload

gpsd_client_test:
	make -C libraries/GPSDClient/examples/gpsd_client_test TMPDIR=$(TMPDIR) upload


v2r_uart_console:
	make -C libraries/AP_HAL_V2R/examples/uart_console TMPDIR=$(TMPDIR) upload

v2r_rc_input:
	make -C libraries/AP_HAL_V2R/examples/rc_input  TMPDIR=$(TMPDIR) upload

inertial_nav_test:
	make -C libraries/AP_InertialNav/examples/AP_InertialNav_test  TMPDIR=$(TMPDIR) upload

notify:
	make -C libraries/AP_Notify/examples/AP_Notify_test  TMPDIR=$(TMPDIR) upload