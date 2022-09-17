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

echo "Lets try get smth"
sudo cat /dev/chardev
echo " "
echo "^ that is what we get(emptiness)"

echo "Lets try write \"ABABA\""
echo "ABABA" > /dev/chardev
sudo cat /dev/chardev

echo "Lets try write \"ABAB\""
echo "ABAB" > /dev/chardev
sudo cat /dev/chardev

echo "Lets try write \"ABA\""
echo "ABA" > /dev/chardev
sudo cat /dev/chardev

sudo rmmod ./hello_world.ko
sudo rm /dev/chardev
sudo dmesg
