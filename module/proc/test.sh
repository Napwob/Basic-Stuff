#!/bin/bash

make
echo "Lets load module"
sudo insmod ./hello_world.ko irq=1 mode=1
echo "Lets try get smth"
sudo cat /proc/proc_file_name
sudo dmesg | tail
echo "Lets try echo"
echo "32 6" > /proc/proc_file_name
sudo dmesg | tail
echo "Lets try get smth"
sudo cat /proc/proc_file_name
sudo rmmod ./hello_world.ko
sudo dmesg | tail
