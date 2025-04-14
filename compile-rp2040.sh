#!/bin/bash
rm -rf build-rp2040 
mkdir build-rp2040
cd build-rp2040
cmake .. -DPICO_BOARD=pico_w -DPICO_PLATFORM=rp2040 -DWIFI_SSID="Pico_w-Net3" -DWIFI_PASSWORD="abdcda5860"
make

