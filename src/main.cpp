#include <stdio.h>

#include "SeamCarver.hpp"

int main(int argc, char *argv[]) {
    // Command line arguments: image path, new width, new height, path to save output image
    const std::string path = argv[1];
    const int newWidth = std::stoi(argv[2]);
    const int newHeight = std::stoi(argv[3]);
    const std::string outPath = argv[4];

    SeamCarver handler = SeamCarver(path, newWidth, newHeight, outPath);

    return 0;
}
