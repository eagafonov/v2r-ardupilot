# Setup for seamless sketch upload

Magic makefiles is trying to upload fresh ELF to remote host name `v2r` using scp. 
To let this magic works you need setup you virt2real and build host a bit.

First of all make sure virt2real is connected to same netwrok as build host 
and you are able to connect to virt2real w/o password promt.

See wiki how to do it (Vir2real networking)<http://wiki.virt2real.ru/wiki/%D0%9F%D0%BE%D0%B4%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D0%B5_%D0%BA_%D0%BB%D0%BE%D0%BA%D0%B0%D0%BB%D1%8C%D0%BD%D0%BE%D0%B9_%D1%81%D0%B5%D1%82%D0%B8>

Secondly, update `/etc/hosts` to point `v2r`to virt2real's IP address.

Double check with `ssh v2r`: you must see virt2real shell promt w/o any password requests 


# Building

- Checkout source code.
- Copy config-v2r-template.mk to config.mk
- Edit config.mk to set V2R_SDK_ROOT variable (at last line) to point to Virt2real SDK root
- Build copter with `make copter`

ELF appers in `build/ArduCopter.build` and automatically uploade to remote host name `v2r` (if available)
 
# TODO

There is some dirty hacks and TODO need to be polished/implemented.
Here is (an incomplete) list of that boring things:

- Use ioctl to mass-update PWM states rather than writing text strings to /dev/v2r_pins.
- Implement ioctl to mass-update PWM (sen prev item)
- Custom thread names is impemented but does not work with latest kernel (worked with ancestor image)
- Read GPS UART baudrate from params
- Remove conditional compilatio for MPU6050 axis direction hack