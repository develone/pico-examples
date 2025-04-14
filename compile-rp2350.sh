#!/bin/bash
rm -rf build-rp2350 
mkdir build-rp2350
cd build-rp2350
cmake .. -DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350 -DWIFI_SSID="Pico_w-Net3" -DWIFI_PASSWORD="abdcda5860"
make

