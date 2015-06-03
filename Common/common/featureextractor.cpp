#include "stdafx.h"
#include <common/featureextractor.h>

#include "networkutils.h"

#include <fstream>
#include <iostream>
#include <ctime>

// x=[0; 5000] -> y=[0.15; 0.08]
#define LEARNING_RATE_FUNCT_DECREASE_SPEED 1000
// 16 is equivalent of function first value 0.1
#define LEARNING_RATE_FUNCT_COEFFICENT_FUNPARAM 16 
#define sqr(x) (x)*(x)

const int FeatureExtractor::LAYER_TYPE[SIZES::LAYERS_COUNT] = { LAYER_INPUT,
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV, LAYER_POOL, 
                                               LAYER_CONV };
const int FeatureExtractor::SIZES::LAYER_EDGE[SIZES::LAYERS_COUNT] = { 68, 64, 32, 28, 14, 10, 5, 1};
const int FeatureExtractor::SIZES::LAYER_MAPS[SIZES::LAYERS_COUNT] = { 1, 8, 8, 32, 32, 128, 128, 128 };
const int FeatureExtractor::SIZES::CONV_MAPS[SIZES::CONV_LAYERS] = { 8, 4, 4, 1 };
const int FeatureExtractor::SIZES::CONV_EDGE[SIZES::CONV_LAYERS] = { 5, 5, 5, 5 };

FeatureExtractor::FeatureExtractor() {
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        int layerEdge = SIZES::LAYER_EDGE[layer];
        int layerMaps = SIZES::LAYER_MAPS[layer];
        Array2D item(layerEdge, layerEdge);
        for (int map = 0; map < layerMaps; ++map) {
            layersMaps[layer].push_back(item);
            layersMapsDeriv[layer].push_back(item);
            layersMapsError[layer].push_back(item);
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

double FeatureExtractor::normL2Activ(double val) {
    return val * val;
}

double FeatureExtractor::normL2ActivDeriv(double val) {
    return 2 * val;
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
                convMap.set(i, j, convActiv(sum));
                
                if (isExtractorTrain) {
                    derivMap.set(i, j, convActivDeriv(sum));
                }
            }
        }
        
        ++mapNum;
    }
}

void FeatureExtractor::fullConnected(int layerNum, int convNum) {
    vector<Array2D> &prevLayerMaps = layersMaps[layerNum - 1];
    vector<Array2D> &layerMaps = layersMaps[layerNum];
    vector<Array2D> &layerMapsDeriv = layersMapsDeriv[layerNum];
    int layerMapsSize = layerMaps.size();
    int prevLayerMapsSize = prevLayerMaps.size();

    for (int mapNum = 0; mapNum < layerMapsSize; ++mapNum) {
        Array2D &weight = weights[convNum][mapNum];
        Array2D &convMap = layerMaps[mapNum];
        Array2D &derivMap = layerMapsDeriv[mapNum];

        double sum = 0;
        for (int prevMapNum = 0; prevMapNum < prevLayerMapsSize; prevMapNum++) {
            Array2D &prevMap = prevLayerMaps[prevMapNum];
            sum += prevMap.at(0, 0) * weight.at(0, prevMapNum);
        }

        sum += weight.getBias();
        convMap.set(0, 0, convActiv(sum));

        if (isExtractorTrain) {
            derivMap.set(0, 0, convActivDeriv(sum));
        }
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


void FeatureExtractor::normL2(int layerNum) {
    vector<Array2D> &prevLayer = layersMaps[layerNum - 1];
    vector<Array2D> &layer = layersMaps[layerNum];
    vector<Array2D> &derivLayer = layersMapsDeriv[layerNum];
    int mapsNum = SIZES::LAYER_MAPS[layerNum];
    for (int map = 0; map < mapsNum; ++map) {
        double y = prevLayer[map].at(0, 0);
        layer[map].set(0, 0, normL2Activ(y));
        derivLayer[map].set(0, 0, normL2ActivDeriv(y));
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
        
        if (LAYER_TYPE[layer] == LAYER_CONV) {
            int mapNum = 0;
            for (int map = 0; map < prevLayerMaps; ++map) {
                conv(layersMaps[layer - 1][map], layer, mapNum, convNum);
            }
            ++convNum;
        } else if (LAYER_TYPE[layer] == LAYER_POOL) {
            for (int map = 0; map < prevLayerMaps; ++map) {
                pool(layersMaps[layer - 1][map], layer, map);
            }
        } else if (LAYER_TYPE[layer] == LAYER_L2_NORM) {
            normL2(layer);
        } else if (LAYER_TYPE[layer] == LAYER_FULL_CONNECTED) {
            fullConnected(layer, convNum);
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

void FeatureExtractor::backpropagation(vector<double> &goal, vector<double> &v, Distance mode) {   
    int convNum = SIZES::CONV_LAYERS - 1;
    for (int layer = SIZES::LAST_LAYER_NUM; layer >= 0; --layer) {
        int layerMaps = SIZES::LAYER_MAPS[layer];
        int layerEdge = SIZES::LAYER_EDGE[layer];
       
        if (layer == SIZES::LAST_LAYER_NUM) {
            if (mode == Distance::Maximize) {
                for (int i = 0; i < goal.size(); ++i) {
                    if (LAYER_TYPE[SIZES::LAST_LAYER_NUM] == LAYER_L2_NORM) {
                        goal[i] = 1 - goal[i];
                    } else {
                        double randSign = (double)rand() / RAND_MAX * 2 - 1; 
                        if (randSign >= 0) {
                            goal[i] = 1 - goal[i];
                        } else {
                            goal[i] = -1 - goal[i];
                        }
                    }
                }
            }

            for (int map = 0; map < layerMaps; ++map) {
                layersMapsError[layer][map].set(0, 0, v[map] - goal[map]);
            }
            continue;
        }

        if (LAYER_TYPE[layer + 1] == LAYER_L2_NORM) {
            int nextLayerEdge = SIZES::LAYER_EDGE[layer + 1];
            for (int map = 0; map < layerMaps; map++) {
                Array2D &derivMap = layersMapsDeriv[layer + 1][map];
                Array2D &errorMap = layersMapsError[layer + 1][map];
                double error = errorMap.at(0, 0) * derivMap.at(0, 0);
                layersMapsError[layer][map].set(0, 0, error);
            }
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

        if (LAYER_TYPE[layer + 1] == LAYER_FULL_CONNECTED) {
            vector<Array2D> &nextLayerMapsDeriv = layersMapsDeriv[layer + 1];
            vector<Array2D> &nextLayerMapsError = layersMapsError[layer + 1];
            int layerMapsSize = SIZES::LAYER_MAPS[layer];
            int nextLayerMapsSize = SIZES::LAYER_MAPS[layer + 1];

            for (int mapNum = 0; mapNum < layerMapsSize; ++mapNum) {
                double error = 0;
                for (int nextMapNum = 0; nextMapNum < nextLayerMapsSize; nextMapNum++) {
                    Array2D &weight = weights[convNum][nextMapNum];
                    error += nextLayerMapsDeriv[nextMapNum].at(0, 0) *
                        nextLayerMapsError[nextMapNum].at(0, 0) *
                        weight.at(0, mapNum);
                }
                layersMapsError[layer][mapNum].set(0, 0, error);
            }
            --convNum;
        }

        if (LAYER_TYPE[layer + 1] == LAYER_CONV) {
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
        if (LAYER_TYPE[layer] == LAYER_CONV) {
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
        if (LAYER_TYPE[layer] == LAYER_FULL_CONNECTED) {
            vector<Array2D> &prevLayerMaps = layersMaps[layer - 1];
            vector<Array2D> &layerMapsDeriv = layersMapsDeriv[layer];
            vector<Array2D> &layerMapsError = layersMapsError[layer];
            int layerMapsSize = SIZES::LAYER_MAPS[layer];
            int prevLayerMapsSize = SIZES::LAYER_MAPS[layer - 1];

            for (int mapNum = 0; mapNum < layerMapsSize; ++mapNum) {
                Array2D &weight = weights[convNum][mapNum];
                double derivative = layerMapsDeriv[mapNum].at(0, 0);
                double error = layerMapsError[mapNum].at(0, 0);

                double sum = 0;
                for (int prevMapNum = 0; prevMapNum < prevLayerMapsSize; prevMapNum++) {
                    Array2D &prevMap = prevLayerMaps[prevMapNum];
                    double delta = (-1) * learningRate *
                        prevMap.at(0, 0) *
                        error * derivative;
                    weight.add(0, prevMapNum, delta);
                }
                double delta = (-1) * learningRate *
                    error * derivative;
                weight.addBias(delta);
            }
            ++convNum;
        }

    }
}

void FeatureExtractor::train(TrainMode MODE, int trainInterations) {
    if (MODE == TrainMode::New) {
        NetworkUtils::removeAllWeights();
        remove("weights.dat");
        //memory leak ?
        weights = *(new Weights());
        weights.save();
    }
    isExtractorTrain = true;

    vector< vector<string> > people = NetworkUtils::loadTrainSetMiner();

    int finalTrainEpoch = weights.getTrainEpoch() + trainInterations;
    //int maxTrainSetSize = trainInterations;
    double pi = 4. * std::atan(1.);

    srand(time(NULL));
    while (weights.getTrainEpoch() < finalTrainEpoch) {
        int humanNum = rand() % people.size();
        int difHumanNum = rand() % people.size();
        
        int photoNum1 = rand() % people[humanNum].size();
        int photoNum2 = rand() % people[humanNum].size();
        int difPhotoNum = rand() % people[difHumanNum].size();
        if (humanNum == difHumanNum || photoNum1 == photoNum2) {
            continue;
        }

        learningRate = 
            (pi / 2 - atan(weights.getTrainEpoch() / LEARNING_RATE_FUNCT_DECREASE_SPEED)) /
            LEARNING_RATE_FUNCT_COEFFICENT_FUNPARAM;

        Mat anchor = imread(people[humanNum][photoNum1].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        Mat positive = imread(people[humanNum][photoNum2].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        Mat negative = imread(people[difHumanNum][difPhotoNum].c_str(), CV_LOAD_IMAGE_UNCHANGED);
        vector<double> anc;
        vector<double> pos;
        vector<double> neg;
       
        anc = getVector(anchor);
        neg = getVector(negative);
        //double imagePrevDifMax = Classifier::getDif(anc, neg);
        backpropagation(anc, neg, Distance::Maximize);
        //double imageDifMax = Classifier::getDif(getVector(anchor), getVector(negative));

        anc = getVector(anchor);
        pos = getVector(positive);
        //double imagePrevDifMin = Classifier::getDif(anc, pos);
        backpropagation(anc, pos, Distance::Minimize);
        //double imageDifMin = Classifier::getDif(getVector(anchor), getVector(positive));
       
        weights.incTrainEpoch();
        if (weights.getTrainEpoch() >= finalTrainEpoch) {
            break;
        }
        if (weights.getTrainEpoch() % 100 == 0) {
            weights.save(weights.getTrainEpoch() / 100);
        }
    }

    weights.save();

    isExtractorTrain = false;
}