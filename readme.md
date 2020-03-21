# CodeTransport

This repository aims to **transport binary file** stream through light (exactly 2-dimensional code).  
 [**Sister repository in Python**](https://github.com/HaiyunPresentation/infoTrans_Py)

This belongs to Project 1 of *Computer Network and Internets* course in the Department of Software Engineering, School of Informatics, Xiamen University. 

## Introduction
We name our 2-dimensional code **NaiveCode**, with 4 anchors and 3840 valid blocks per frame, each has one of 2³=8 colors (BGR channels).   
The output rate of NaiveCode video stream is set with 20 fps.   
Checked with CRC-4/ITU, the valid transport rate is above 100 Kibps.

## How to use
### encode
Just execute:  
`encode <input_path> <output_path> <time>`

**Notes**:
- `input_path` - Path of input binary file
- `output_path` - Path of output video
- `time` - Maximum time (ms) of output video

### decode
Play the output video and capture by mobile phone (**60 fps** or higher). Then copy the video captured to the computer and execute:  
`decode <input_path> <output_path> <val_path>`

**Notes**:
- `input_path` - Path of your captured video
- `output_path` - Path of output file
- `val_path` - Path of val file (check result)

## Problem
Results vary by device and ambient light.

