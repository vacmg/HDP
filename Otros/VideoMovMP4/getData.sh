clear

if scp pi@$1:/home/pi/Camera.zip .
then
unzip Camera.zip && rm Camera.zip && sleep 5s && clear && echo success
else
echo check ip address and password
fi
