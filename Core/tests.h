#include <hash_set>
//How does unrequarement inludes influence on comlpiler and binary (perfomance, size)?

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

class Tests {

private:
    static void normalizeImage(Mat &mat);
    static void displayNearestImages(Mat &mat, string &nearestImagePath);
public:
    static double outputResultPairsLFW();
};