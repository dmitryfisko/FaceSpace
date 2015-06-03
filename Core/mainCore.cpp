#include "imageminer.h"
#include <common/featureextractor.h>
#include <common/classifier.h>
#include <common/tests.h>
#include <iostream>
#include <ctime>

int main() {
    //ImageMiner miner;
    //miner.mine();

    size_t backpropagationStartTime = clock();
    FeatureExtractor extractor;
    extractor.train(FeatureExtractor::TrainMode::Continue, 0);
    cout << "Algorithm worked for " << clock() - backpropagationStartTime << "ms" << endl;

    //Mat durov = imread("d:\\X\\FaceSpace\\Datasets\\imageminer\\faces_hq\\1\\face_1.jpg", -1);
    //extractor.getVector(durov, true);
    Classifier classifier;
    classifier.addProfile("d:\\X\\FaceSpace\\Datasets\\profiles\\fisko.jpg",
                          "Дмитрий Фисько");
    classifier.addProfile("d:\\X\\FaceSpace\\Datasets\\profiles\\laschenko.jpg",
                          "Антон Лащенко");
    //classifier.getUID(  );

    //FILE *stream;
    //freopen_s(&stream, "out.txt", "w", stdout);
    //how does name this test?
    //size_t testStartTime = clock();
    //Tests::outputResultPairsLFW();
    //cout << "Test worked for " << clock() - testStartTime << " ms" << endl;

    return 0;
}