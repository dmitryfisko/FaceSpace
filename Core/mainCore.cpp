#include "imageminer.h"
#include <common/featureextractor.h>
#include <common/classifier.h>
#include "tests.h"
#include <iostream>

int main() {
    ImageMiner miner;
    //miner.mine();

    FeatureExtractor extractor;
    extractor.train(FeatureExtractor::TrainMode::New);
    //Mat durov = imread("d:\\X\\FaceSpace\\Datasets\\faces_hq\\1_face_hq_1.jpg", -1);
    //Classifier classifier;
    //classifier.getUID( extractor.getVector(durov, true) );
 

    //FILE *stream;
    //freopen_s(&stream, "out.txt", "w", stdout);
    //std::cout << "LFW: " << Tests::getResLFW();

    return 0;
}