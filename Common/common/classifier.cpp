#include <common/stdafx.h>
#include <common/classifier.h>

#include <assert.h>
#include <fstream>

#define sqr(x) ((x)*(x))

const double Classifier::MAXIMUM = 1e9;
const double Classifier::THRESHOLD = 0.11;

Classifier::Classifier() {
    ifstream in("../common/profiles.dat");
    if (!in) {
        return;
    }

    int profilesNum;
    in >> profilesNum;
    for (int i = 0; i < profilesNum; ++i) {
        string userName;
        vector<double> uid;
        __int64 key;
        int uidLen;

        getline(in, userName);
        getline(in, userName);
        in >> key >> uidLen;
        uid.reserve(uidLen);
        for (int j = 0; j < uidLen; ++j) {
            double item;
            in >> item;
            uid.push_back(item);
        }

        Profile profile(userName, key, uid);
        profiles.push_back(profile);
    }
}

void Classifier::saveProfiles() {
    ofstream out("../common/profiles.dat");
    
    out << profiles.size() << endl;
    for (int i = 0; i < profiles.size(); ++i) {
        out << profiles[i].userName << endl
            << profiles[i].key << endl 
            << profiles[i].uid.size() << endl;
        for (int j = 0; j < profiles[i].uid.size(); ++j) {
            out << profiles[i].uid[j] << ' ';
        }
        out << endl;
    }
}

void Classifier::addProfile(string path, string userName) {
    Mat mat = imread(path.c_str());
    int height = mat.size().height;
    int width = mat.size().width;
    assert(width > 0 && width == height);

    vector<double> uid = extractor.getVector(mat);
    __int64 key = profiles.size() + 1;
    Classifier::Profile profile(userName, key, uid);
    profiles.push_back(profile);
    saveProfiles();
}

double Classifier::getDif(vector<double> &v1, vector<double> &v2) {
    assert(v1.size() == v2.size());
    double sum = 0;
    for (int i = 0; i < v1.size(); ++i) {
        sum += sqr(v1[i] - v2[i]);
    }
    return sqrt(sum);
}

double Classifier::tripletLoss(vector<double> &a, vector<double> &p, 
                               vector<double> &n) {
    assert(a.size() == p.size() && a.size() == n.size());
    double loss = 0;
    for (int i = 0; i < a.size(); ++i) {
        loss += max((double)0, sqr(a[i] - p[i]) - sqr(a[i] - n[i]) + 0.2);
    }
    return loss / a.size();
}

vector<Classifier::Profile> Classifier::getProfiles(cv::Mat &image, vector<cv::Rect> &rects) {
    vector<Classifier::Profile> ans;
    ans.reserve(rects.size());
    for (int i = 0; i < rects.size(); ++i) {
        Mat roi = image(rects[i]);
        ans.push_back(getProfile(roi));
        //imshow("smt", roi);
        //cvWaitKey(5000);
    }
    return ans;
}

Classifier::Profile Classifier::getProfile(cv::Mat &image) {
    assert(profiles.size() > 0);

    vector<double> v = extractor.getVector(image);
    double minLoss = MAXIMUM;
    int minInd = -1;
    for (int i = 0; i < profiles.size(); ++i) {
        double loss = getDif(v, profiles[i].uid);
        if (loss < minLoss) {
            minLoss = loss;
            minInd = i;
        }
    }

    return profiles[minInd];
}

bool Classifier::isSame(vector<double> &v1, vector<double> &v2) {
    cout << getDif(v1, v2) << endl;
    if (getDif(v1, v2) <= THRESHOLD) {
        return true;
    } else {
        return false;
    }
}