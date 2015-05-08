#include "stdafx.h"
#include "tests.h"
#include "featureextractor.h"
#include "classifier.h"
#include "networkutils.h"

#include <fstream>

void Tests::normalizeImage(Mat &mat) {
    resize(mat, mat, Size(174, 174));
    if (mat.channels() > 1) {
        cvtColor(mat, mat, COLOR_BGR2GRAY);
        //equalizeHist(mat, mat);
    }
}

void Tests::displayNearestImages(Mat &mat, string &nearestImagePath) {
    Mat nearestMat = imread(nearestImagePath.c_str(), CV_LOAD_IMAGE_UNCHANGED);
    normalizeImage(mat);
    normalizeImage(nearestMat);
    imshow("image", mat);
    imshow("nearest image", nearestMat);
    cvWaitKey(5000);
}

double Tests::getResLFW() {
    ifstream in("pairsDevTest.txt");
    map<string, vector<string>> humans = NetworkUtils::loadTestSetLFW();
    FeatureExtractor extractor;
    Classifier classifier;
    
    int testCases;
    int correct = 0;
    int incorrect = 0;
    const int maxTestsPerSet = 50;
    int processedTests = 0;

    in >> testCases;
    for (int i = 0; i < testCases; ++i) {
        string humanName;
        int imageNum1;
        int imageNum2;
        in >> humanName >> imageNum1 >> imageNum2;
        
        if (++processedTests >= maxTestsPerSet) {

            Mat image1 = imread(humans[humanName][imageNum1 - 1], CV_LOAD_IMAGE_UNCHANGED); 
            vector<double> v1 = extractor.getVector(image1);
            Mat image2 = imread(humans[humanName][imageNum2 - 1], CV_LOAD_IMAGE_UNCHANGED);
            vector<double> v2 = extractor.getVector(image2);

            if (classifier.isSame(v1, v2)) {
                ++correct;
            } else {
                ++incorrect;
            }
        }
    }
    processedTests = 0;
    for (int i = 0; i < testCases; ++i) {
        string humanName1;
        string humanName2;
        int imageNum1;
        int imageNum2;
        in >> humanName1 >> imageNum1 >> humanName2 >> imageNum2;

        if (processedTests++ < maxTestsPerSet) {
            Mat image1 = imread(humans[humanName1][imageNum1 - 1], CV_LOAD_IMAGE_UNCHANGED);
            vector<double> v1 = extractor.getVector(image1);
            Mat image2 = imread(humans[humanName2][imageNum2 - 1], CV_LOAD_IMAGE_UNCHANGED);
            vector<double> v2 = extractor.getVector(image2);

            if (classifier.isSame(v1, v2)) {
                ++incorrect;
            } else {
                ++correct;
            }
        }
    }
    cout << "LFW result: " << correct / (correct + incorrect) * 100 << '%';

    return correct / (correct + incorrect) * 100;
}