-!/bin/bash

echo "Preparation"
sudo rmmod ./main.ko
sudo dmesg --clear
echo "Preparation"

make

echo "Lets load module"
sudo insmod ./main.ko
echo "Module loaded"
modinfo main.ko

echo "Lets try write to ping"
echo | ifconfig | tail | grep "inet " | cut -c14-30 | cut -d " " -f 1 > /sys/kernel/kobject_example/ping
sudo echo "Read writed cache"; sudo cat /sys/kernel/kobject_example/ping

#echo "Lets try ping \"127.0.0.1\""
#ping 127.0.0.1 -c 5 -w 1 -W 1

echo "Lets unload module"
sudo rmmod ./main.ko
echo "Module unloaded"
sudo dmesg
