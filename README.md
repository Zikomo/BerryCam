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
|   | A+ | Yes | No
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




