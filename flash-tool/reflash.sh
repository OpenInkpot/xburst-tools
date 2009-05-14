#!/bin/bash

sudo ./inflash -c "boot"
sudo ./inflash -c "nprog 0 u-boot-nand.bin 0 0 -n"
# sudo ./inflash -c "nprog 2048 uImage 0 0 -n"

