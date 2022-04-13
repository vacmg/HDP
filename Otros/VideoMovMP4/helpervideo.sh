rm Camera.zip
echo sleeping for 15 seconds
sleep 15s
python3 video.py
rm Camera/*.h264
zip -9 -r Camera.zip Camera
rm -r Camera
