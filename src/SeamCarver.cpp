#include "SeamCarver.hpp"

// Flatten 2D index into 1D
int IX(int x, int y, int width, int grouping = 1) { return (x + (y * width)) * grouping; }

SeamCarver::SeamCarver(std::string imagePath, int pTargetWidth, int pTargetHeight,
                       std::string outPath) {
    cv::Mat readImg = cv::imread(imagePath, 1);

    currentWidth = readImg.cols;
    currentHeight = readImg.rows;

    targetWidth = pTargetWidth;
    targetHeight = pTargetHeight;

    // Load image BGR values into array
    cv::Vec3b color;
    for (int i = 0; i < currentHeight; i++) {
        for (int j = 0; j < currentWidth; j++) {
            color = readImg.at<cv::Vec3b>(i, j);
            image.push_back(color[0]);
            image.push_back(color[1]);
            image.push_back(color[2]);
        }
    }

    adjustWidth();
    adjustHeight();

    saveResult(outPath);
}

int SeamCarver::energy(int x, int y, int w, int h) {
    int leftPixel = IX(x - 1, y, w, 3);
    int rightPixel = IX(x + 1, y, w, 3);
    int abovePixel = IX(x, y - 1, w, 3);
    int belowPixel = IX(x, y + 1, w, 3);

    if (x == 0)
        leftPixel = IX(w - 1, y, w, 3);
    if (x == w - 1)
        rightPixel = IX(0, y, w, 3);
    if (y == 0)
        abovePixel = IX(x, h - 1, w, 3);
    if (y == h - 1)
        belowPixel = IX(x, 0, w, 3);

    int Bx = std::abs(image[rightPixel] - image[leftPixel]);
    int Gx = std::abs(image[rightPixel + 1] - image[leftPixel + 1]);
    int Rx = std::abs(image[rightPixel + 2] - image[leftPixel + 2]);
    int deltaX = (Bx * Bx) + (Gx * Gx) + (Rx * Rx);

    int By = std::abs(image[belowPixel] - image[abovePixel]);
    int Gy = std::abs(image[belowPixel + 1] - image[abovePixel + 1]);
    int Ry = std::abs(image[belowPixel + 2] - image[abovePixel + 2]);
    int deltaY = (By * By) + (Gy * Gy) + (Ry * Ry);

    return deltaX + deltaY;
}

void SeamCarver::updateEnergies(int w, int h) {
    energies.clear();
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            energies.push_back(energy(j, i, w, h));
        }
    }
    // Set border pixel energies high so that seam finding avoids edges
    for (int i = 0; i < h; i++) {
        energies[IX(0, i, w, 1)] = 2147483647;
        energies[IX(w - 1, i, w, 1)] = 2147483647;
    }
    for (int j = 0; j < w; j++) {
        energies[IX(j, 0, w, 1)] = 2147483647;
        energies[IX(j, h - 1, w, 1)] = 2147483647;
    }
}

// Use dynamic programming to find seam with lowest energy
std::pair<int, int> SeamCarver::findSeam(int w, int h) {
    updateEnergies(w, h);

    optimums.clear();
    seamPath.clear();

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            optimums.push_back(0);
            seamPath.push_back(0);
        }
    }

    for (int i = 0; i < w; i++) {
        optimums[IX(i, 0, w, 1)] = energies[IX(i, 0, w, 1)];
        seamPath[IX(i, 0, w, 1)] = 0;
    }

    for (int i = 1; i < h; i++) {
        for (int j = 0; j < w; j++) {
            optimums[IX(j, i, w, 1)] = optimums[IX(j, i - 1, w, 1)];
            seamPath[IX(j, i, w, 1)] = 0;

            auto leftPixel = optimums[IX(j - 1, i - 1, w, 1)];
            auto rightPixel = optimums[IX(j + 1, i - 1, w, 1)];

            if (leftPixel < optimums[IX(j, i, w, 1)]) {
                optimums[IX(j, i, w, 1)] = leftPixel;
                seamPath[IX(j, i, w, 1)] = -1;
            }
            if (rightPixel < optimums[IX(j, i, w, 1)]) {
                optimums[IX(j, i, w, 1)] = rightPixel;
                seamPath[IX(j, i, w, 1)] = 1;
            }

            optimums[IX(j, i, w, 1)] += energies[IX(j, i, w, 1)];
        }
    }

    int startCol = 1;
    for (int j = 0; j < w; j++) {
        if (optimums[IX(j, h - 1, w, 1)] < optimums[IX(startCol, h - 1, w, 1)]) {
            startCol = j;
        }
    }

    // Return the final pixel in the lowest energy seam
    return std::pair<int, int>(startCol, h);
}

void SeamCarver::deleteSeam(std::pair<int, int> start, int w, int h) {
    auto colIndex = start.first;
    // iterate through seam from bottom to top, deleting pixels in seam
    for (int i = start.second - 1; i >= 0; i--) {
        image.erase(image.begin() + IX(colIndex, i, w, 3) + 2);
        image.erase(image.begin() + IX(colIndex, i, w, 3) + 1);
        image.erase(image.begin() + IX(colIndex, i, w, 3));
        colIndex += seamPath[IX(colIndex, i, w, 1)];
        if (colIndex < 0) {
            colIndex = w - 1;
        } else if (colIndex >= w) {
            colIndex = 0;
        }
    }
}

void SeamCarver::addSeam(std::vector<std::pair<int, int>> start, int w, int h) {
    auto colIndex = start[0].first;
    for (int i = h - 1; i >= 0; i--) {
        // Pixel values for new seam are derived from those of neighbouring pixels
        uchar avgB = (image[IX(colIndex, i, w, 3)] + image[IX(colIndex, i, w, 3) - 3] +
                      image[IX(colIndex, i, w, 3) + 3]) /
                     3;
        uchar avgG = (image[IX(colIndex, i, w, 3) + 1] + image[IX(colIndex, i, w, 3) - 2] +
                      image[IX(colIndex, i, w, 3) + 4]) /
                     3;
        uchar avgR = (image[IX(colIndex, i, w, 3) + 2] + image[IX(colIndex, i, w, 3) - 1] +
                      image[IX(colIndex, i, w, 3) + 5]) /
                     3;
        image.insert(image.begin() + IX(colIndex, i, w, 3) + 3, avgB);
        image.insert(image.begin() + IX(colIndex, i, w, 3) + 4, avgG);
        image.insert(image.begin() + IX(colIndex, i, w, 3) + 5, avgR);
        if (i > 0) {
            colIndex = start[i - 1].first;
        }
    }
}

void SeamCarver::seamInsertion(int &w, int &h, int targetW, int targetH) {
    std::vector<std::vector<std::pair<int, int>>> pathList;

    std::vector<uchar> cacheImage(image);
    int cacheWidth = w;

    // Remove n seams, record those seams in pathList
    auto range = std::abs(targetW - w);
    for (int i = 0; i < range; i++) {
        auto base = findSeam(w, h);
        int colIndex = base.first;
        std::vector<std::pair<int, int>> currentPath;
        for (int i = h - 1; i >= 0; i--) {
            currentPath.push_back({colIndex, i});
            colIndex += seamPath[IX(colIndex, i, w, 1)];
        }
        deleteSeam(base, w, h);
        w -= 1;
        pathList.push_back(currentPath);
    }

    // Restore original image
    image = cacheImage;
    w = cacheWidth;

    std::reverse(pathList.begin(), pathList.end());

    // insert new seams at each of the previously deleted seams recorded in pathList
    for (int i = 0; i < range; i++) {
        auto currentSeam = pathList.back();
        pathList.pop_back();
        addSeam(currentSeam, w, h);
        w += 1;
        // Update indexes stored in pathList if their coordinates are affected by the insertion of a new seam
        for (auto &remainingSeam : pathList) {
            for (int j = 0; j < h; j++) {
                if (remainingSeam[j].first >= currentSeam[j].first) {
                    remainingSeam[j].first += 1;
                }
            }
        }
    }
}

void SeamCarver::adjustWidth() {
    if (targetWidth < currentWidth) {
        auto range = std::abs(currentWidth - targetWidth);
        for (int i = 0; i < range; i++) {
            deleteSeam(findSeam(currentWidth, currentHeight), currentWidth, currentHeight);
            currentWidth--;
        }
    } else if (targetWidth > currentWidth) {
        seamInsertion(currentWidth, currentHeight, targetWidth, targetHeight);
    }
}

void SeamCarver::adjustHeight() {
    // Rotate image so horizontal seams become vertical seams
    std::vector<uchar> rotatedImg;
    for (int i = 0; i < currentWidth; i++) {
        for (int j = currentHeight - 1; j >= 0; j--) {
            rotatedImg.push_back(image[IX(i, j, currentWidth, 3)]);
            rotatedImg.push_back(image[IX(i, j, currentWidth, 3) + 1]);
            rotatedImg.push_back(image[IX(i, j, currentWidth, 3) + 2]);
        }
    }
    image = rotatedImg;

    if (targetHeight < currentHeight) {
        auto range = std::abs(currentHeight - targetHeight);
        for (int i = 0; i < range; i++) {
            // Pass in currentHeight as w and currentWidth as h because rotating image causes these dimensions to swap
            deleteSeam(findSeam(currentHeight, currentWidth), currentHeight, currentWidth);
            currentHeight--;
        }
    } else if (targetHeight > currentHeight) {
        seamInsertion(currentHeight, currentWidth, targetHeight, targetWidth);
    }

    // Rotate image back to original orientation after seams have been added/removed
    std::vector<uchar> restoredImg;
    for (int i = currentHeight - 1; i >= 0; i--) {
        for (int j = 0; j < currentWidth; j++) {
            restoredImg.push_back(image[IX(i, j, currentHeight, 3)]);
            restoredImg.push_back(image[IX(i, j, currentHeight, 3) + 1]);
            restoredImg.push_back(image[IX(i, j, currentHeight, 3) + 2]);
        }
    }
    image = restoredImg;
}

void SeamCarver::saveResult(std::string path) {
    cv::Mat exportImg = cv::Mat(currentHeight, currentWidth, CV_8UC3, image.data());
    cv::imwrite(path, exportImg);
}
