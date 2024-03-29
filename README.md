# BerryCam
Raspberry Pi baby monitor project. 

# What is BerryCam
Frankly I got sick of the selection of baby monitors on the market so I decided to roll my own. This codebase should 
not be limited to strictly a baby monitor use case but keep in mind my design decisions are generally driven toward 
that. 

# Setup
## Target Hardware
Currently being developed and tested on a Raspberry Pi 3b+ with the intent to support the Raspberry Pi Zero W.

|Family|Model|Supported|Tested
|:---|:---:|:---:|---:|
| Raspberry Pi 1  | B   | No | N/A
|   | A   | No | N/A |
|   | B+  | Needs third party wifi | No
|   | A+  | Needs third party wifi | No
| Raspberry Pi 2 | B | Needs third party wifi | No 
| Raspberry Pi Zero  | Zero | No | N/A
|   | W/WH | Yes | No
| Raspberry Pi 3 | B | Yes | No 
|   | A+ | Yes | Yes
|   | B+ | Yes | Yes
| Raspberry Pi 4 | B (1GB) | Yes | No
| Raspberry Pi 4 | B (2GB) | Yes | No
| Raspberry Pi 4 | B (4GB) | Yes | No
 
## Dependencies
cmake:

`sudo apt-get install cmake`

FFMPEG - libavcodec:

`sudo apt-get install libavcodec-dev`

Boost: 

`sudo apt-get install libboost-system-dev`
`sudo apt-get install libboost-program-options-dev`


## Building
Clone this repository:

`git clone https://github.com/Zikomo/BerryCam.git`
 
Run cmake for an out of source build:
```
cd BerryCam
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
```
Run make:
`make`

## Optional: Run BerryCam as a service
`sudo nano /lib/systemd/system/berrycam.service`

Then use the following as a template but note that you need to replace everything in between < > with the path to this
repository on your system:

```
[Unit]
Description=BerryCam
After=multi-user.target

[Service]
ExecStart=<Insert the path to where you cloned this repository>/BerryCam/Release/BerryCam --settings <Insert the path to where you cloned this repository>/BerryCam/Release/settings.json

[Install]
WantedBy=multi-user.target
```
The reload systemctl:

`sudo systemctl daemon-reload`

Then enable the BerryCam service:

`sudo systemctl enable berrycam.service`

And reboot your Raspberry Pi:

`sudo reboot now`

## Optional: Using nginx as a webcam server:

```
sudo apt-get update
sudo apt-get install nginx
```

Then from your BerryCam repository: 

`sudo cp html/index.html /var/www/html/`







