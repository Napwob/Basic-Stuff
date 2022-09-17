#!/bin/bash

sudo rmmod ./hello_world.ko
sudo rm /dev/chardev
sudo dmesg
