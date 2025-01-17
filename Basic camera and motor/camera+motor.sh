#!/bin/bash

#setting up camera to ensure working
gio mount -s gphoto2

# inputting steps for rotation
read -p "Enter the number of steps you would like to rotate (160*degrees): " steps
echo ""

# rotating motor - call c code
./moves -s $steps -p 17

#taking picture while spinning turntable
gphoto2 --capture-image
