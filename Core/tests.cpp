#include <common/stdafx.h>
#include "tests.h"
#include <common/featureextractor.h>
#include <common/classifier.h>
#include <common/networkutils.h>

#include <fstream>
#include <algorithm>

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

void Tests::outputResultPairsLFW() {
    ifstream in("pairsDevTest.txt");
    map<string, vector<string>> people = NetworkUtils::loadPairsTestSetLFW();
    FeatureExtractor extractor;
    Classifier classifier;
    int testPairs;

    in >> testPairs;
    vector<double> simularity;
    simularity.reserve(testPairs * 2);
    for (int i = 0; i < testPairs; ++i) {
        string humanName;
        int imageNum1;
        int imageNum2;
        in >> humanName >> imageNum1 >> imageNum2;
        
        Mat image1 = imread(people[humanName][imageNum1 - 1], CV_LOAD_IMAGE_UNCHANGED);
        vector<double> v1 = extractor.getVector(image1);
        Mat image2 = imread(people[humanName][imageNum2 - 1], CV_LOAD_IMAGE_UNCHANGED);
        vector<double> v2 = extractor.getVector(image2);
        simularity.push_back(classifier.getDif(v1, v2));
    }
    for (int i = testPairs; i < testPairs * 2; ++i) {
        string humanName1;
        string humanName2;
        int imageNum1;
        int imageNum2;
        in >> humanName1 >> imageNum1 >> humanName2 >> imageNum2;

        Mat image1 = imread(people[humanName1][imageNum1 - 1], CV_LOAD_IMAGE_UNCHANGED);
        vector<double> v1 = extractor.getVector(image1);
        Mat image2 = imread(people[humanName2][imageNum2 - 1], CV_LOAD_IMAGE_UNCHANGED);
        vector<double> v2 = extractor.getVector(image2);
        simularity.push_back(classifier.getDif(v1, v2));
    }

    const double maxSimularity = *max_element(simularity.begin(), simularity.end()) + 0.1;
    const double accuracy = 1000;

    double bestResult = -1;
    double bestThreshold;
    for (double threshold = 0; threshold < maxSimularity * accuracy; ++threshold) {
        double curThreshold = threshold / accuracy;
        int correct = 0;
        int incorrect = 0;
        // simular pairs
        for (int i = 0; i < testPairs; ++i) {
            if (simularity[i] <= curThreshold) {
                ++correct;
            } else {
                ++incorrect;
            }
        }
        // different pairs
        for (int i = testPairs; i < testPairs * 2; ++i) {
            if (simularity[i] > curThreshold) {
                ++correct;
            } else {
                ++incorrect;
            }
        }
        double result = (double)correct / (correct + incorrect);
        if (result > bestResult) {
            bestResult = result;
            bestThreshold = curThreshold;
        }
    }
        
    cout << "LFW Pairs result " << bestResult * 100 << "% when threshold " << bestThreshold;
}