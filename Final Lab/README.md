# Real-Time-Embedded-Systems
## Final Lab : Covid Bluetooth Trace Simulator.
## Dimosthenis Iliadis-Apostolidis

### A ready-to-run file named ./8811 is include. It has been compiled and cross-compiled and tested on rpi zero.
### To recompile the file the steps are:

1. Compile with gcc on PC

<pre>
gcc -O3 -lm -pthread -o 8811 8811_embedded.c
</pre>

2. Donwload tools-master. It can be found [here](https://github.com/raspberrypi/tools). In tools-master :

<pre>

cd /PATH/TO/tools-master

cd arm-bcm2708

cd arm-rpi-4.9.3-linux-gnueabihf

cd bin

export PATH=$PATH:/home/USER/Desktop/tools-master/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin

arm-linux-gnueabihf-gcc ~/PATH/TO/8811_embedded.c -O3 -lm -pthread -o ~/PATH/TO/GCC-COMPILED/8811 -std=gnu99

</pre>

3. Copy the cross-compiled 8811 file to rpi.

<pre>
scp 8811 USER@RASPBERRY.PI.IP.ADDRESS:~
</pre>

4. Use ssh to connect to pi and run on pi side.

<pre>
ssh USER@RASPBERRY.PI.IP.ADDRESS
</pre>

5. Copy trace.txt file with all the timestamps for statistical analysis, from rpi to PC. Run on PC side :

<pre>
scp USER@RASPBERRY.PI.IP.ADDRESS:~/trace.txt .
</pre>

### The R script is also included for the statistical analysis of the executable.
