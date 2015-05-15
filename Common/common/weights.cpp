#include "stdafx.h"
#include "featureextractor.h"

#include <fstream>
#include <time.h>

FeatureExtractor::Weights::Weights() {
    int convNum = 0;
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        if (LAYER_TYPE[layer] == LAYER_CONV ||
                LAYER_TYPE[layer] == LAYER_NEIRON) {
            CONV_LAYER_MAPS[convNum] = SIZES::LAYER_MAPS[layer];
            CONV_LAYER_EDGE[convNum] = SIZES::CONV_EDGE[convNum];
            ++convNum;
        }
    }

    srand(time(NULL));
    for (int layer = 0; layer < SIZES::CONV_LAYERS; ++layer) {
        int layerMaps = CONV_LAYER_MAPS[layer];
        int convEdge = CONV_LAYER_EDGE[layer];
        for (int map = 0; map < layerMaps; ++map) {
            Array2D item(convEdge, convEdge, true);
            weights[layer].push_back(item);
            //??? how does push or assign objet to vector without copy constructor
        }
    }

    ifstream in("weights.dat");
    if (!in) {
        return;
    }

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
            assert(width == convEdge && height == convEdge);

            Array2D &weight = weights[layer][map];
            weight.setBias(bias);
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    double cell;
                    in >> cell;
                    weight.set(i, j, cell);
                }
            }
        }
    }
}

FeatureExtractor::Weights::~Weights() {
    ofstream out("weights.dat");
    if (!out) {
        return;
    }

    for (int layer = 0; layer < SIZES::CONV_LAYERS; ++layer) {
        int layerMaps = CONV_LAYER_MAPS[layer];
        int convEdge = CONV_LAYER_EDGE[layer];

        out << layerMaps << endl;
        for (int map = 0; map < layerMaps; ++map) {
            Array2D &weight = weights[layer][map];
            out << convEdge << ' ' << convEdge << ' ' 
                << weight.getBias() << endl;
            for (int i = 0; i < convEdge; ++i) {
                for (int j = 0; j < convEdge; ++j) {
                    out << weight.at(i, j) << ' ';
                }
                out << endl;
            }
        }

    }
}