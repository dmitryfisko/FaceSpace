#include <iostream>

#include "imageminer.h"
#include "featureextractor.h"
#include "tests.h"

int main() {
    //ImageMiner miner;
    //miner.mine();

    FeatureExtractor extractor;
    //extractor.train(FeatureExtractor::TrainMode::Continue);
    //Mat durov = imread("d:\\X\\FaceSpace\\Datasets\\faces_hq\\1_face_hq_1.jpg", -1);
    //extractor.getVector(durov, true);
    
    FILE *stream;
    freopen_s(&stream, "out.txt", "w", stdout);
    cout << "LFW: " << Tests::getResLFW();

    return 0;
}