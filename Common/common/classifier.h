#pragma once

#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <common/featureextractor.h>

class Classifier {
public:
    Classifier();

    struct Profile {
        string userName;
        int key;
        vector<double> uid;

        Profile(string name,
                __int64 key,
                vector<double> uid) : userName(name), key(key), uid(uid) {}
    };

    Profile getProfile(cv::Mat &mat);
    vector<Profile> getProfiles(cv::Mat &mat, vector<cv::Rect> &rects);
    static double getDif(vector<double> &v1, vector<double> &v2);
    static bool isSame(vector<double> &v1, vector<double> &v2);
    static double tripletLoss(vector<double> &a, vector<double> &p,
                              vector<double> &n);
    void saveProfiles();
    void addProfile(string path, string userName);
private:
    static const double MAXIMUM;
    static const double THRESHOLD;

    vector<Profile> profiles;
    FeatureExtractor extractor;
};