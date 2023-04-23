# nanopi_max86150_ecg_ppg

Before starting building code wiringPi must be downloaded.

> apt-get install wiringpi

Or directly built from source code:

>     git clone https://github.com/friendlyarm/WiringNP
>     cd WiringNP/
>     chmod 755 build
>     ./build

Note, that special flags must be set while calling gcc

> -lwiringPi -lpthread

By default I2C speed is around 90 KBaud, while MAX86150 support up to 400KBaud.
Unfortunately, I couldn't reach such speed. My maximum is 250KBaud, but board didn't work well on such speeds, so I stick to 210KBaud. To make it happen, device tree must be decompiled, edited, recompiled and replaced
#### Decompile:

>     dtc -I dtb -O dts /boot/sun8i-h3-nanopi-neo-air.dtb -o /tmp/sun8i-h3-nanopi-neo-air.dts

#### Edit:
Few lines must be found with text redactor:

>                 i2c0 {
>                     pins = "PA11\OPA12";
>                     function = "i2c0";
>                     linux,phandle = <0x2c>;
>                     phandle = <0x2c>;
>                 };

>             i2c@01c2ac00 {
>                 compatible = "allwinner,sun6i-a31-i2c";
>                 ...
>             };

and next parameter must be added to both places (I added it after phandle):

> clock-frequency = <400000>;

### Important note: even though we added 400000, it doesn't mean we will have 400KB. As I said, with this value I only received 210KB, while 760000 gave me 250KB.

#### Compile device-tree, replace it, reboot:

>     dtc -I dtb -O dts /tmp/sun8i-h3-nanopi-neo-air.dts -o /tmp/sun8i-h3-nanopi-neo-air.dtb
>     cp /tmp/sun8i-h3-nanopi-neo-air.dtb /boot/sun8i-h3-nanopi-neo-air.dtb

After reboot try to find device with i2cdetect:

>     root@NanoPi-NEO-Air:/home/pi/nanopi_max86150_ecg_ppg# i2cdetect -y 0 
>          0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
>     00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- 5e -- 
>     60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
>     70: -- -- -- -- -- -- -- --

Device **5e** is MAX86150.
