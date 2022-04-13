# Este programa sirve para cambiar de formato un fichero de vídeo de H264 a MP4.
# Para ello, hace uso del comando MP4Box, incluido en el paquete "gpac". Para
# instalarlo, hace falta ejecutar el siguiente comando en una terminal:
# sudo apt install gpac

# se puede editar /boot/config.txt y añadir disable_camera_led=1 para evitar que se encienda la luz cuando la camara esta en uso

# import logging
# logging.basicConfig(format='%(levelname): %(asctime)s: "%(message)s"',level=logging.INFO)

from gpiozero import DigitalInputDevice
from picamera import PiCamera
from time import sleep,asctime
import subprocess
import os

pir = DigitalInputDevice(17)
seconds = 5
wait = 2
rotation = 180
resolution = 1920,1080  # 640,480 # 1920,1080

filename = ''
pstatus = 0
def setFilename():
    global filename
    filename = './Camera/video_' + asctime().replace(' ', '_').replace(':','-')
    print(f'Current filename: "{os.path.abspath(filename)}"')

def remVideo():
    os.remove(filename+'.h264')
    # os.remove(filename + '.mp4')

setFilename()

if not os.path.isdir('./Camera'):
    os.mkdir('./Camera')

print("Ready!!!")

# x = 1

while 1:
    if pir.value == 1 and pstatus == 0:
    # if x == 1:
        # x = 0
        print("I got you!!!")
        with PiCamera() as camera:
            camera.resolution = (resolution[0], resolution[1])
            camera.rotation = rotation
            sleep(2)
            print(f"Recording for {seconds} seconds")
            camera.start_recording(filename+'.h264')
            camera.wait_recording(seconds)
            camera.stop_recording()
        print('Processing the video...')

        # El fichero original es filename y la salida, ya convertida a MP4, es
        # filename + ".mp4".
        comando = 'MP4Box -add '+filename+'.h264 '+ filename+'.mp4'
        subprocess.run(comando, shell=True, check=True)
        remVideo()
        setFilename()  # update filename
        pstatus = 1
        print('Done!')
        sleep(wait)
    if pir.value == 0:
        pstatus = 0
    sleep(1)
