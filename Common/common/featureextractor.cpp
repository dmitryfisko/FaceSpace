#include "stdafx.h"
#include "featureextractor.h"
#include "networkutils.h"

#include <fstream>
#include <iostream>
#include <ctime>

#define sqr(x) (x)*(x)

const int FeatureExtractor::LAYER_TYPE[SIZES::LAYERS_COUNT] = { LAYER_INPUT,
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_NEIRON };
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
        }
    }

    int convNum = 0;
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        if (LAYER_TYPE[layer] == LAYER_INPUT) {
            continue;
        }

        int layerEdge = SIZES::LAYER_EDGE[layer];
        int layerMaps = SIZES::LAYER_MAPS[layer];
        Array2D item(layerEdge, layerEdge);
        for (int map = 0; map < layerMaps; ++map) {
            layersMapsDeriv[convNum].push_back(item);
        }
        ++convNum;
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
    vector<Array2D> &res = layersMaps[layerNum];
    vector<Array2D> &resDeriv = layersMapsDeriv[convNum];
    int convEdge = SIZES::CONV_EDGE[convNum];
    int convMaps = SIZES::CONV_MAPS[convNum];
    int layerEdge = SIZES::LAYER_EDGE[layerNum];

    for (int map = 0; map < convMaps; ++map) {
        Array2D &weight = weights[convNum][mapNum];
        Array2D &convMap = res[mapNum];
        Array2D &derivMap = resDeriv[mapNum];
        for (int i = 0; i < layerEdge; ++i) {
            for (int j = 0; j < layerEdge; ++j) {
                double sum = 0;
                for (int k = 0; k < convEdge; ++k) {
                    for (int t = 0; t < convEdge; ++t) {
                        sum += prevMap.at(i + k, j + t) * weight.at(k, t);
                    }
                }
                sum += weight.getBias();
                double activation = convActiv(sum);
                double derivative = convActivDeriv(sum);

                convMap.set(i, j, activation);
                derivMap.set(i, j, derivative);
            }
        }
        
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

vector<double> FeatureExtractor::getVector(Mat &mat, bool visualize) {
    isVisualize = visualize;
    vector<double> &ans = getVector(mat);
    isVisualize = visualize;
    return ans;
}

vector<double> FeatureExtractor::getVector(Mat &image) {
    Mat mat;
    image.copyTo(mat);
    resize(mat, mat, Size(SIZES::LAYER_EDGE[0], SIZES::LAYER_EDGE[0]));

    Array2D inputArr(mat);
    int convNum = 0;
    layersMaps[0].clear();
    layersMaps[0].push_back(inputArr);

    for (int layer = 1; layer < SIZES::LAYERS_COUNT; ++layer) {
        int prevLayerMaps = SIZES::LAYER_MAPS[layer - 1];
        int layerEdge = SIZES::LAYER_EDGE[layer - 1];
        
        if (LAYER_TYPE[layer] == LAYER_CONV ||
                LAYER_TYPE[layer] == LAYER_NEIRON) {
            int mapNum = 0;
            for (int map = 0; map < prevLayerMaps; ++map) {
                conv(layersMaps[layer - 1][map], layer, mapNum, convNum);
            }
            ++convNum;
        } else if (LAYER_TYPE[layer] == LAYER_POOL) {
            for (int map = 0; map < prevLayerMaps; ++map) {
                pool(layersMaps[layer - 1][map], layer, map);
            }
            ++convNum;
        }
    }

    
    if (isVisualize) {
        Visializer visualizer;
        visualizer.show(layersMaps);
    }

    vector<double> ans;
    // Does layers[lastLayerNum].size() cashing by compiler?
    for (unsigned int i = 0; i < layersMaps[SIZES::LAST_LAYER_NUM].size(); ++i) {
        ans.push_back(layersMaps[SIZES::LAST_LAYER_NUM][i].at(0, 0));
    }

    return ans;
}

Rect FeatureExtractor::getPossibleConvPositionsRect(int i, int j, 
                                                    int layerEdge, int convEdge) {
    int localLeft;
    int localRight;
    int localTop;
    int localBottom;

    int offsetLeft = j - 1;
    int offsetRight = layerEdge - j - 1;
    int offsetTop = i - 1;
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

vector<double> FeatureExtractor::backpropagation(Mat &image, vector<double> goal) {
    vector<double> out = getVector(image, false);

    int convNum = SIZES::CONV_LAYERS - 1;
    for (int layer = SIZES::LAST_LAYER_NUM; layer >= 0; --layer) {
        int layerMaps = SIZES::LAYER_MAPS[layer];
        int layerEdge = SIZES::LAYER_EDGE[layer];
        int nextLayerEdge = SIZES::LAYER_EDGE[layer + 1];
       
        if (layer == SIZES::LAST_LAYER_NUM) {
            for (int map = 0; map < layerMaps; ++map) {
                layersMapsError[layer][map].set(0, 0, out[map] - goal[map]);
            }
            continue;
        }

        if (LAYER_TYPE[layer + 1] == LAYER_NEIRON) {
            for (int map = 0; map <= layerMaps; map++) {
                Array2D &weight = weights[layer][map];
                Array2D &derivMap = layersMapsDeriv[layer + 1][map];
                Array2D &errorMap = layersMapsDeriv[layer + 1][map];
                for (int i = 0; i < layerEdge; ++i) {
                    for (int j = 0; j < layerEdge; ++j) {
                        double erorr = errorMap.at(0, 0) * 
                            derivMap.at(0, 0) * weight.at(i, j);
                        layersMapsError[layer][map].set(i, j, erorr);
                    }
                }
            }
            --convNum;
        }

        if (LAYER_TYPE[layer + 1] == LAYER_POOL) {
            for (int map = 0; map <= layerMaps; map++) {
                Array2D &derivMap = layersMapsDeriv[layer + 1][map];
                Array2D &errorMap = layersMapsDeriv[layer + 1][map];

                for (int i = 0; i < nextLayerEdge; ++i) {
                    for (int j = 0; j < nextLayerEdge; ++j) {
                        double error = errorMap.at(i, j) * derivMap.at(i, j);
                        for (int k = 0; k < 2; ++k) {
                            for (int t = 0; t < 2; ++t) {
                                layersMapsError[layer][map].set (i * 2 + k, 
                                                                j * 2 + t, error);
                            }
                        }
                    }
                }
            }
        }

        if (LAYER_TYPE[layer + 1] == LAYER_CONV) {
            int convEdge = SIZES::CONV_EDGE[convNum];
            int convMaps = SIZES::CONV_MAPS[convNum];
            for (int map = 0; map <= layerMaps; map++) {
                Array2D &weight = weights[layer][map];
                for (int i = 0; i < layerEdge; ++i) {
                    for (int j = 0; j < layerEdge; ++j) {
                        Rect &area = getPossibleConvPositionsRect(i, j, layerEdge, convEdge);
                        double error = 0;
                        for (int k = area.x; k < area.x + area.height; ++k) {
                            for (int t = area.y; t < area.y + area.width; ++t) {
                                for (int convMap = 0; convMap < convMaps; ++convMap) {
                                    int convMapNum = map * convMaps + convMap;
                                    Array2D &errorMap = layersMapsError[layer + 1][convMapNum];
                                    Array2D &derivMap = layersMapsDeriv[layer + 1][convMapNum];
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
        int prevLayerMaps = SIZES::LAYER_MAPS[layer - 1];
        int mapNum = 0;
        for (int map = 0; map < prevLayerMaps; ++map) {
            if (LAYER_TYPE[layer] == LAYER_CONV || 
                    LAYER_TYPE[layer] == LAYER_NEIRON) {
                int convEdge = SIZES::CONV_EDGE[convNum];
                int convMaps = SIZES::CONV_MAPS[convNum];
                int layerEdge = SIZES::LAYER_EDGE[layer];
                Array2D &prevMap = layersMaps[layer - 1][map];

                for (int map = 0; map < convMaps; ++map) {
                    Array2D &weight = weights[convNum][mapNum];
                    Array2D &convMap = layersMaps[layer][mapNum];
                    Array2D &derivMap = layersMapsDeriv[layer][mapNum];
                    Array2D &errorMap = layersMapsError[layer][mapNum];
                    for (int i = 0; i < layerEdge; ++i) {
                        for (int j = 0; j < layerEdge; ++j) {
                            for (int k = 0; k < convEdge; ++k) {
                                for (int t = 0; t < convEdge; ++t) {
                                    double delta = (-1) * LEARNING_SPEED * 
                                        prevMap.at(i + k, j + t) *
                                        errorMap.at(i, j) * 
                                        derivMap.at(i, j);
                                    weight.add(k, t, delta);
                                }
                            }
                        }
                    }
                    ++mapNum;
                }
                ++convNum;
            }
        }
    }
}

void FeatureExtractor::train(TrainMode MODE) {
    if (MODE == TrainMode::New) {
        remove("weights.dat");
        //memory leak ?
        weights = *(new Weights());
    }
    isExtractorTrain = true;

    int maxTrainSetSize = 100000;
    trainEpoch = 0;

    vector< vector<string> > people = NetworkUtils::loadTrainSetMiner();

    srand(time(NULL));
    while (trainEpoch < maxTrainSetSize) {
        int humanNum = rand() % people.size();
        int photoNum1 = rand() % people[humanNum].size();
        int photoNum2 = rand() % people[humanNum].size();
        if (photoNum1 == photoNum2) {
            continue;
        }

        Mat image1 = imread(people[humanNum][photoNum1].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        Mat image2 = imread(people[humanNum][photoNum2].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        backpropagation(image1, getVector(image2));

        ++trainEpoch;
        if (trainEpoch < maxTrainSetSize) {
            break;
        }
    }

    isExtractorTrain = false;
}