# Welcome to Wireless-BoneScripts

Feel free to look at the attached projects that involves mostly socket programming in c++.
All the projects you will find involves a TCP server that runs as a daemon process. Different probes have been implemented in the source-codes, which can be read via a syslog. The syslog can be found at /var/log/messages if you are curious about its location. The TCP server is used to monitor a LM35DZ temperature sensor, meaning that a connected client can gain access to the reading operation performed by the server. 

The attached code is designed to work with a Beaglebone Black Wireless.

The essential specifications to know about the Beaglebone I am using are:
- Operating system: Debian 9.2
- Kernel: 4.4.117-bone22

This can be important, since the filesystems are organized differently based on which kernel you are using.
P.S. Newer kernels doesn't need overlays anymore, look for 'config-pin'

The Operating system can be found at:
https://beagleboard.org/latest-images
