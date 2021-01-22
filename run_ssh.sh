#!/bin/bash

sudo ./lidar &
sudo ./as5048a &
mavproxy.py --master=/dev/serial0 --baudrate 921600
