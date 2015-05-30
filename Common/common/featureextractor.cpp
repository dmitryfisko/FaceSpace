#include "stdafx.h"
#include "featureextractor.h"
#include <common/classifier.h>

#include "networkutils.h"

#include <fstream>
#include <iostream>
#include <ctime>

#define sqr(x) (x)*(x)

const int FeatureExtractor::LAYER_TYPE[SIZES::LAYERS_COUNT] = { LAYER_INPUT,
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_SOFTMAX };
const int FeatureExtractor::SIZES::LAYER_EDGE[SIZES::LAYERS_COUNT] = { 68, 64, 32, 28, 14, 10, 5, 1 };
const int FeatureExtractor::SIZES::LAYER_MAPS[SIZES::LAYERS_COUNT] = { 1, 8, 8, 32, 32, 128, 128, 128 };
const int FeatureExtractor::SIZES::CONV_MAPS[SIZES::CONV_LAYERS] = { 8, 4, 4, 1};
const int FeatureExtractor::SIZES::CONV_EDGE[SIZES::CONV_LAYERS] = { 5, 5, 5, 5};

FeatureExtractor::FeatureExtractor() {
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        int layerEdge = SIZES::LAYER_EDGE[layer];
        int layerMaps = SIZES::LAYER_MAPS[layer];
        Array2D item(layerEdge, layerEdge);
        for (int map = 0; map < layerMaps; ++map) {
            layersMaps[layer].push_back(item);
            layersMapsError[layer].push_back(item);
            layersMapsDeriv[layer].push_back(item);
        }
    }
}

double FeatureExtractor::convActiv(double sum) {
    return tanh(sum);
}

double FeatureExtractor::convActivDeriv(double sum) {
    double _tanh = tanh(sum);
    return 1 - sqr(_tanh);
}

double FeatureExtractor::poolActiv(double sum) {
    return sum / 4;
}

double FeatureExtractor::poolActivDeriv(double sum) {
    return (double)1 / 4;
}

void FeatureExtractor::conv(Array2D &prevMap, int layerNum, int &mapNum, int convNum) {
    vector<Array2D> &layerMaps = layersMaps[layerNum];
    vector<Array2D> &layerMapsDeriv = layersMapsDeriv[layerNum];
    int convEdge = SIZES::CONV_EDGE[convNum];
    int convMaps = SIZES::CONV_MAPS[convNum];
    int layerEdge = SIZES::LAYER_EDGE[layerNum];

    for (int map = 0; map < convMaps; ++map) {
        Array2D &weight = weights[convNum][mapNum];
        Array2D &convMap = layerMaps[mapNum];
        Array2D &derivMap = layerMapsDeriv[mapNum];
        for (int i = 0; i < layerEdge; ++i) {
            for (int j = 0; j < layerEdge; ++j) {
                double sum = 0;
                for (int k = 0; k < convEdge; ++k) {
                    for (int t = 0; t < convEdge; ++t) {
                        sum += prevMap.at(i + k, j + t) * weight.at(k, t);
                    }
                }
                sum += weight.getBias();
                if (LAYER_TYPE[layerNum] == LAYER_CONV) {
                    convMap.set(i, j, convActiv(sum));
                } else {
                    convMap.set(i, j, exp(sum));
                }
                
                if (isExtractorTrain) {
                    if (LAYER_TYPE[layerNum] == LAYER_CONV) {
                        derivMap.set(i, j, convActivDeriv(sum));
                    }
                }
            }
        }

        /*if (LAYER_TYPE[layerNum] == LAYER_SOFTMAX) {
            double convAllSum = 0;
            for (int i = 0; i < layerEdge; ++i) {
                for (int j = 0; j < layerEdge; ++j) {
                    convAllSum += convMap.at(i, j);
                }
            }

            for (int i = 0; i < layerEdge; ++i) {
                for (int j = 0; j < layerEdge; ++j) {
                    convMap.set(i, j, convMap.at(i, j) / convAllSum);
                }
            }

            if (isExtractorTrain) {
                for (int i = 0; i < layerEdge; ++i) {
                    for (int j = 0; j < layerEdge; ++j) {
                        double y = convMap.at(i, j);
                        derivMap.set(i, j, y * (1 - y));
                    }
                }
            }
        }*/
        
        ++mapNum;
    }
}

void FeatureExtractor::pool(Array2D &prevMap, int layerNum, int mapNum) {
    Array2D &poolMap = layersMaps[layerNum][mapNum];
    Array2D &derivMap = layersMapsDeriv[layerNum][mapNum];
    int layerEdge = SIZES::LAYER_EDGE[layerNum];
    for (int i = 0; i < layerEdge; ++i) {
        for (int j = 0; j < layerEdge; ++j) {
            double sum = 0;
            for (int k = 0; k < 2; ++k) {
                for (int t = 0; t < 2; ++t) {
                    sum += prevMap.at(i * 2 + k, j * 2 + t);
                }
            }
            poolMap.set(i, j, poolActiv(sum));
            derivMap.set(i, j, poolActivDeriv(sum));
        }
    }
}

void FeatureExtractor::normalizeVector(vector<double> &v) {
    int len = v.size();
    double sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += v[i] * v[i];
    }
    double scale = sqrt(sum);
    if (scale == 0) {
        return;
    }

    for (int i = 0; i < len; ++i) {
        v[i] /= scale;
    }
}

vector<double> FeatureExtractor::getVector(Mat &mat, bool visualize) {
    isVisualize = visualize;
    vector<double> &ans = getVector(mat);
    isVisualize = false;
    return ans;
}

vector<double> FeatureExtractor::getVector(Mat &image) {
    Mat mat;
    image.copyTo(mat);
    resize(mat, mat, Size(SIZES::LAYER_EDGE[0], SIZES::LAYER_EDGE[0]));

    Array2D inputArr(mat);
    //layersMaps[0][0] = inputArr;
    layersMaps[0].clear();
    layersMaps[0].push_back(inputArr);

    int convNum = 0;
    for (int layer = 1; layer < SIZES::LAYERS_COUNT; ++layer) {
        int prevLayerMaps = SIZES::LAYER_MAPS[layer - 1];
        
        if (LAYER_TYPE[layer] == LAYER_CONV ||
                LAYER_TYPE[layer] == LAYER_SOFTMAX) {
            int mapNum = 0;
            for (int map = 0; map < prevLayerMaps; ++map) {
                conv(layersMaps[layer - 1][map], layer, mapNum, convNum);
            }
            ++convNum;
        } else if (LAYER_TYPE[layer] == LAYER_POOL) {
            for (int map = 0; map < prevLayerMaps; ++map) {
                pool(layersMaps[layer - 1][map], layer, map);
            }
        }
    }
    
    if (isVisualize) {
        Visializer visualizer;
        visualizer.show(layersMaps);
    }

    int lastLayerNum = SIZES::LAST_LAYER_NUM;
    if (LAYER_TYPE[lastLayerNum] == LAYER_SOFTMAX) {
        int layerMaps = SIZES::LAYER_MAPS[SIZES::LAST_LAYER_NUM];
        double softmaxAllSum = 0;
        for (int map = 0; map < layerMaps; ++map) {
            Array2D &convMap = layersMaps[lastLayerNum][map];
            softmaxAllSum += convMap.at(0, 0);
        }
        for (int map = 0; map < layerMaps; ++map) {
            Array2D &convMap = layersMaps[lastLayerNum][map];
            convMap.set(0, 0, convMap.at(0, 0) / softmaxAllSum);
        }
        for (int map = 0; map < layerMaps; ++map) {
            Array2D &convMap = layersMaps[lastLayerNum][map];
            Array2D &derivMap = layersMapsDeriv[lastLayerNum][map];
            double y = convMap.at(0, 0);
            derivMap.set(0, 0, y * (1 - y));
        }
    }
    // Does layers[lastLayerNum].size() cashing by compiler?
    vector<double> ans;
    for (unsigned int i = 0; i < layersMaps[lastLayerNum].size(); ++i) {
        ans.push_back(layersMaps[lastLayerNum][i].at(0, 0));
    }
    //normalizeVector(ans);

    return ans;
}

Rect FeatureExtractor::getPointLocalFields(int i, int j,
                                           int layerEdge, int convEdge) {
    int localLeft;
    int localRight;
    int localTop;
    int localBottom;

    int offsetLeft = j;
    int offsetRight = layerEdge - j - 1;
    int offsetTop = i;
    int offsetBottom = layerEdge - i - 1;

    if (offsetLeft >= convEdge - 1) {
        localRight = convEdge - 1;
    } else {
        localRight = offsetLeft;
    }
    if (offsetRight >= convEdge - 1) {
        localLeft = 0;
    } else {
        localLeft = convEdge - offsetRight - 1;
    }
    if (offsetTop >= convEdge - 1) {
        localBottom = convEdge - 1;
    } else {
        localBottom = offsetTop;
    }
    if (offsetBottom >= convEdge - 1) {
        localTop = 0;
    } else {
        localTop = convEdge - offsetBottom - 1;
    }

    return Rect(localTop, localLeft,
                localRight - localLeft + 1, 
                localBottom - localTop + 1);
}

void FeatureExtractor::backpropagation(Mat &image, vector<double> goal,
                                       Distance mode) {
    vector<double> response = getVector(image);
    if (mode == Distance::Maximize) {
        for (int i = 0; i < goal.size(); ++i) {
            double randSign = (double)rand() / RAND_MAX * 2 - 1;
            //if (randSign >= 0) {
                goal[i] = 1 - goal[i];
            //} else {
            //    goal[i] = -1 - goal[i];
            //}
        }
    }
    
    int convNum = SIZES::CONV_LAYERS - 1;
    for (int layer = SIZES::LAST_LAYER_NUM; layer >= 0; --layer) {
        int layerMaps = SIZES::LAYER_MAPS[layer];
        int layerEdge = SIZES::LAYER_EDGE[layer];
       
        if (layer == SIZES::LAST_LAYER_NUM) {
            for (int map = 0; map < layerMaps; ++map) {
                layersMapsError[layer][map].set(0, 0, response[map] - goal[map]);
            }
            continue;
        }

        if (LAYER_TYPE[layer + 1] == LAYER_POOL) {
            int nextLayerEdge = SIZES::LAYER_EDGE[layer + 1];
            for (int map = 0; map < layerMaps; map++) {
                Array2D &derivMap = layersMapsDeriv[layer + 1][map];
                Array2D &errorMap = layersMapsError[layer + 1][map];

                for (int i = 0; i < nextLayerEdge; ++i) {
                    for (int j = 0; j < nextLayerEdge; ++j) {
                        double error = errorMap.at(i, j) * derivMap.at(i, j);
                        for (int k = 0; k < 2; ++k) {
                            for (int t = 0; t < 2; ++t) {
                                layersMapsError[layer][map].set(i * 2 + k, 
                                                                j * 2 + t, error);
                            }
                        }
                    }
                }
            }
        }

        if (LAYER_TYPE[layer + 1] == LAYER_CONV || 
                LAYER_TYPE[layer + 1] == LAYER_SOFTMAX) {
            int convEdge = SIZES::CONV_EDGE[convNum];
            int convMaps = SIZES::CONV_MAPS[convNum];
            int mapNum = 0;
            for (int map = 0; map < layerMaps; map++) {
                for (int i = 0; i < layerEdge; ++i) {
                    for (int j = 0; j < layerEdge; ++j) {
                        Rect &area = getPointLocalFields(i, j, layerEdge, convEdge);
                        double error = 0;
                        for (int convMapNum = 0; convMapNum < convMaps; ++convMapNum) {
                            int mapNum = map * convMaps + convMapNum;
                            Array2D &weight = weights[convNum][mapNum];
                            Array2D &errorMap = layersMapsError[layer + 1][mapNum];
                            Array2D &derivMap = layersMapsDeriv[layer + 1][mapNum];
                            for (int k = area.x; k < area.x + area.height; ++k) {
                                for (int t = area.y; t < area.y + area.width; ++t) {
                                    error += weight.at(k, t) * 
                                        errorMap.at(i - k, j - t) *
                                        derivMap.at(i - k, j - t);
                                }
                            }
                        }

                        layersMapsError[layer][map].set(i, j, error);
                    }
                }
            }
            --convNum;
        }
    }

    convNum = 0;
    for (int layer = 1; layer < SIZES::LAYERS_COUNT; ++layer) {
        if (LAYER_TYPE[layer] != LAYER_CONV &&
                LAYER_TYPE[layer] != LAYER_SOFTMAX) {
            continue;
        }
        int prevLayerMaps = SIZES::LAYER_MAPS[layer - 1];
        int mapNum = 0;
        for (int map = 0; map < prevLayerMaps; ++map) {
            int convEdge = SIZES::CONV_EDGE[convNum];
            int convMaps = SIZES::CONV_MAPS[convNum];
            int layerEdge = SIZES::LAYER_EDGE[layer];
            Array2D &prevMap = layersMaps[layer - 1][map];

            for (int convMapNum = 0; convMapNum < convMaps; ++convMapNum) {
                Array2D &weight = weights[convNum][mapNum];
                Array2D &derivMap = layersMapsDeriv[layer][mapNum];
                Array2D &errorMap = layersMapsError[layer][mapNum];
                for (int i = 0; i < layerEdge; ++i) {
                    for (int j = 0; j < layerEdge; ++j) {
                        for (int k = 0; k < convEdge; ++k) {
                            for (int t = 0; t < convEdge; ++t) {
                                double delta = (-1) * learningRate *
                                    prevMap.at(i + k, j + t) *
                                    errorMap.at(i, j) * 
                                    derivMap.at(i, j);
                                weight.add(k, t, delta);
                            }
                        }
                        double delta = (-1) * learningRate *
                            prevMap.at(i, j) *
                            errorMap.at(i, j) *
                            derivMap.at(i, j);
                        weight.addBias(delta);
                    }
                }
                ++mapNum;
            }
        }
        ++convNum;
    }
}

void FeatureExtractor::train(TrainMode MODE, int trainInterations) {
    if (MODE == TrainMode::New) {
        remove("weights.dat");
        //memory leak ?
        weights = *(new Weights());
    }
    isExtractorTrain = true;

    vector< vector<string> > people = NetworkUtils::loadTrainSetMiner();

    Classifier classifier;
    int maxTrainSetSize = trainInterations;
    double pi = 4. * std::atan(1.);

    srand(time(NULL));
    trainEpoch = 0;
    while (trainEpoch < maxTrainSetSize) {
        int humanNum = rand() % people.size();
        int difHumanNum = rand() % people.size();
        
        int photoNum1 = rand() % people[humanNum].size();
        int photoNum2 = rand() % people[humanNum].size();
        int difPhotoNum = rand() % people[difHumanNum].size();
        if (humanNum == difHumanNum || photoNum1 == photoNum2) {
            continue;
        }

        //lerning rate at begining is 0.05
        learningRate = (pi / 2 - atan(trainEpoch / 500)) / 32; // 16 = 0.1

        Mat image1 = imread(people[humanNum][photoNum1].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        Mat image2 = imread(people[humanNum][photoNum2].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        Mat difImage = imread(people[difHumanNum][difPhotoNum].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        vector<double> goal = getVector(image1);
        //double imagePrevDifMax = classifier.getDif(getVector(image1), getVector(difImage));
        backpropagation(difImage, goal, Distance::Maximize);
        //double imageDifMax = classifier.getDif(getVector(image1), getVector(difImage));
        //double imagePrevDifMin = classifier.getDif(getVector(image1), getVector(image2));
        backpropagation(image2, goal, Distance::Minimize);
        //double imageDifMin = classifier.getDif(getVector(image1), getVector(image2));
        

        //if (imagePrevDifMin < imageDifMin || imagePrevDifMax > imageDifMax) {
        //    if ((imagePrevDifMin - imageDifMin < -0.01) || (imagePrevDifMax - imageDifMax > 0.01)) {
        //        cout << trainEpoch << endl;
        //    }
        //}

        ++trainEpoch;
        if (trainEpoch >= maxTrainSetSize) {
            break;
        }
        if (trainEpoch % 5000 == 0) {
            weights.save(trainEpoch / 5000);
        }
    }

    weights.save();

    isExtractorTrain = false;
}