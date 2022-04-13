clear
zip -9 sendNewVersion.zip *

if scp sendNewVersion.zip pi@$1:/home/pi
then
sleep 5s && clear
else
echo check ip address and password
fi

rm sendNewVersion.zip

#if scp updateNewVersion.sh pi@$1:/home/pi
#then
#sleep 5s && clear && echo success && echo "now decompress the file using: "&& echo unzip sendNewVersion.zip
#else
#echo check ip address and password
#fi
