#!/bin/bash

echo "Preparation"
sudo rmmod ./hello_world.ko
echo "Preparation"

sudo dmesg --clear
clear
make

echo "Lets load module"
sudo insmod ./hello_world.ko
echo "Module loaded"
modinfo hello_world.ko

echo "Lets try get smth"
sudo cat /sys/kernel/kobject-example/foo
echo "^ that is what we get"

echo "Lets try write \"1\" to baz"
echo "127001" > /sys/kernel/kobject_example/baz
sudo cat /sys/kernel/kobject_example/baz

echo "Lets try write \"2\" to bar"
echo "2" > /sys/kernel/kobject_example/bar
sudo cat /sys/kernel/kobject_example/bar

echo "Lets try write \"3\" to foo"
echo "3" > /sys/kernel/kobject_example/foo
sudo cat /sys/kernel/kobject_example/foo

echo "Lets unload module"
sudo rmmod ./hello_world.ko
echo "Module unloaded"
sudo dmesg
