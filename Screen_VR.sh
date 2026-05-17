#!/bin/bash
PC_Name="MyPC.local"
PC_IP=$(getent hosts "$PC_Name"| awk '{print $1}')
if [ -z "$PC_IP" ]; then
echo "The IP adress has not been found"
echo "Make sure that everything is in order"
exit 1
fi
echo "Success! Everything is in order!"
RES="800x400" 
FPS="60"
BITRATE="8000"
echo "Starting VR headset"
moonlight stream $PC_IP "Desktop"\
--resolution $RES\
--fps $FPS\
--bitrate $BITRATE\
--unsupported\
--packet-size 1392
if [ $? -ne 0 ]; then
echo "Error! The VR headset has failed to start streaming"
fi

