#!/bin/bash

sudo rmmod ./hello_world.ko
sudo rm /dev/chardev
clear
make
sudo dmesg --clear
sudo dmesg
echo "Lets load module"
sudo insmod ./hello_world.ko
sudo mknod /dev/chardev c 511 0
echo "Module loaded"
modinfo hello_world.ko
echo "Node have been created"
sudo ./ioctl
