#include "imageminer.h"
#include <common/featureextractor.h>
#include <common/classifier.h>
#include "tests.h"
#include <iostream>

int main() {
    //ImageMiner miner;
    //miner.mine();

    FeatureExtractor extractor;
    extractor.train(FeatureExtractor::TrainMode::Continue, 1000);
    //Mat durov = imread("d:\\X\\FaceSpace\\Datasets\\imageminer\\faces_hq\\1\\face_1.jpg", -1);
    //extractor.getVector(durov, true);
    //Classifier classifier;
    //classifier.getUID(  );

    //FILE *stream;
    //freopen_s(&stream, "out.txt", "w", stdout);
    //how does name this test?
    Tests::outputResultPairsLFW();
    //57.4 
    //57.6 LEARNING_SPEED=0.01
    //57.7 LEARNING_SPEED=0.05
    //55.9 LEARNING_SPEED=0.2

    return 0;
}