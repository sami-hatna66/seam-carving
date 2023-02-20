# Seam Carving

My implementation of [seam carving for content-aware image resizing](https://perso.crans.org/frenoy/matlab2012/seamcarving.pdf). I have endeavoured to make as little use of library functions as possible in this project, hence OpenCV is only used to load the input image and save the resulting output, all pixel manipulation functions are implemented by myself.

## Demo

https://user-images.githubusercontent.com/88731772/220096658-8ae9cbaf-e52c-4a21-906c-98a2f7967245.mp4

## Usage

The program is operated from the command line
```
$ ./seamcarving /path/to/image [width] [height] /save/output/path
```
