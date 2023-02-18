#include <opencv2/opencv.hpp>

class SeamCarver {
private:
    std::vector<uchar> image;
    std::vector<int> energies;
    std::vector<int> optimums;
    std::vector<int> seamPath;
    int currentWidth;
    int currentHeight;
    int targetWidth;
    int targetHeight;
public:
    SeamCarver(std::string imagePath, int pTargetWidth, int pTargetHeight, std::string outPath);

    int energy(int x, int y, int w, int h);
    void updateEnergies(int w, int h);

    std::pair<int, int> findSeam(int w, int h);
    void deleteSeam(std::pair<int, int> start, int w, int h);
    void seamInsertion(int& w, int& h, int targetW, int targetH);
    void addSeam(std::vector<std::pair<int, int>> start, int w, int h);

    void adjustWidth();
    void adjustHeight();

    void saveResult(std::string path);
};
