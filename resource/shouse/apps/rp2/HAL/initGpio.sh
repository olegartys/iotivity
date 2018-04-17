#!/bin/bash

echo '17' > /sys/class/gpio/export
echo '27' > /sys/class/gpio/export

sleep 1

echo 'out' > /sys/class/gpio/gpio17/direction
