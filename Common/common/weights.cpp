#include "stdafx.h"
#include "featureextractor.h"

#include <fstream>
#include <time.h>
#include <common/tests.h>

FeatureExtractor::Weights::Weights() {
    int convNum = 0;
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        if (LAYER_TYPE[layer] == LAYER_CONV ||
                LAYER_TYPE[layer] == LAYER_FULL_CONNECTED) {
            if (LAYER_TYPE[layer] == LAYER_CONV) {
                CONV_LAYER_EDGE[convNum] = SIZES::CONV_EDGE[convNum];
            } else if (LAYER_TYPE[layer] == LAYER_FULL_CONNECTED) {
                int prevLayerMapsNum = SIZES::LAYER_MAPS[layer - 1];
                CONV_LAYER_EDGE[convNum] = prevLayerMapsNum;
            }
            CONV_LAYER_MAPS[convNum] = SIZES::LAYER_MAPS[layer];
            CONV_LAYER_TYPE[convNum] = LAYER_TYPE[layer];
            ++convNum;
        }
    }

    srand(time(NULL));
    trainEpoch = 0;
    for (convNum = 0; convNum < SIZES::CONV_LAYERS; ++convNum) {
        if (CONV_LAYER_TYPE[convNum] == LAYER_CONV ||
                CONV_LAYER_TYPE[convNum] == LAYER_FULL_CONNECTED) {
            int layerMaps = CONV_LAYER_MAPS[convNum];
            int convEdge = CONV_LAYER_EDGE[convNum];
            for (int mapNum = 0; mapNum < layerMaps; ++mapNum) {
                if (CONV_LAYER_TYPE[convNum] == LAYER_CONV) {
                    weights[convNum].push_back( Array2D(convEdge, convEdge, true) );
                } else if (CONV_LAYER_TYPE[convNum] == LAYER_FULL_CONNECTED) {
                    weights[convNum].push_back( Array2D(convEdge, 1, true) );
                }

                //??? how does push or assign objet to vector without copy constructor
            }
        }
    }

    ifstream in("../common/weights.dat");
    if (!in) {
        return;
    }

    in >> trainEpoch;
    for (int layer = 0; layer < SIZES::CONV_LAYERS; ++layer) {
        int convEdge = CONV_LAYER_EDGE[layer];
        int layerMaps;
        in >> layerMaps;
        assert(layerMaps == CONV_LAYER_MAPS[layer]);
        if (layerMaps != CONV_LAYER_MAPS[layer]) {
            return;
        }
            
        for (int map = 0; map < layerMaps; ++map) {
            int width;
            int height;
            double bias;
            in >> width >> height >> bias;
            assert((width == convEdge && height == convEdge) || 
                   ((width == convEdge && height == 1)));

            Array2D &weight = weights[layer][map];
            weight.setBias(bias);
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    double cell;
                    in >> cell;
                    weight.set(i, j, cell);
                }
            }
        }
    }
}

void FeatureExtractor::Weights::incTrainEpoch() {
    ++trainEpoch;
}

int FeatureExtractor::Weights::getTrainEpoch() {
    return trainEpoch;
}

void FeatureExtractor::Weights::save() {
    save(-1);
}

void FeatureExtractor::Weights::save(int num) {
    string s = "../common/weights";
    if (num >= 0) {
        save();
        s += " " + to_string(num) + " " 
            + to_string(Tests::outputResultPairsLFW());
    }
    s += ".dat";

    ofstream out(s);
    if (!out) {
        return;
    }

    out << trainEpoch << endl;
    for (int convNum = 0; convNum < SIZES::CONV_LAYERS; ++convNum) {
        int layerMaps = CONV_LAYER_MAPS[convNum];
        int convEdge = CONV_LAYER_EDGE[convNum];

        out << layerMaps << endl;
        for (int map = 0; map < layerMaps; ++map) {
            Array2D &weight = weights[convNum][map];
            out << weight.getWidth() << ' ' << weight.getHeight() << ' '
                << weight.getBias() << endl;
            for (int i = 0; i < weight.getHeight(); ++i) {
                for (int j = 0; j < weight.getWidth(); ++j) {
                    out << weight.at(i, j) << ' ';
                }
                out << endl;
            }
        }
    }
    out.flush();
}