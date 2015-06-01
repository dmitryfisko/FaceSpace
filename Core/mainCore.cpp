#include "imageminer.h"
#include <common/featureextractor.h>
#include <common/classifier.h>
#include "tests.h"
#include <iostream>
#include <ctime>

int main() {
    //ImageMiner miner;
    //miner.mine();

    size_t backpropagationStartTime = clock();
    FeatureExtractor extractor;
    extractor.train(FeatureExtractor::TrainMode::New, 5000);
    cout << "Algorithm worked for " << clock() - backpropagationStartTime << "ms" << endl;

    //Mat durov = imread("d:\\X\\FaceSpace\\Datasets\\imageminer\\faces_hq\\1\\face_1.jpg", -1);
    //extractor.getVector(durov, true);
    //Classifier classifier;
    //classifier.getUID(  );

    //FILE *stream;
    //freopen_s(&stream, "out.txt", "w", stdout);
    //how does name this test?
    size_t testStartTime = clock();
    Tests::outputResultPairsLFW();
    cout << "Test worked for " << clock() - testStartTime << " ms" << endl;
    //55.3 10000

    return 0;
}