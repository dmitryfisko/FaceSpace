#include "featureextractor.h"
#include "networkutils.h"

string FeatureExtractor::Visializer::toString(int val) {
    assert(val >= 0);
    if (val == 0) {
        return "0";
    }
    string str = "";
    while (val != 0) {
        str = (char)('0' + val % 10) + str;
        val /= 10;
    }
    return str;
}

void FeatureExtractor::Visializer::show(vector<Array2D> mapsLayers[]) {
    for (int layer = 0; layer < SIZES::LAYERS_COUNT; ++layer) {
        for (int map = 0; map < mapsLayers[layer].size(); ++map) {
            Array2D &rect = mapsLayers[layer][map];
            int width = rect.getWidth();
            int height = rect.getHeight();

            Mat grayscale(Size(width, height), CV_8U);
            uint8_t *pixelPtr = (uint8_t*) grayscale.data;
            for (int i = 0; i < width; ++i) {
                for (int j = 0; j < height; ++j) {
                    pixelPtr[i*width + j] = (uint8_t) ((rect.at(i, j) + 1) * 128);
                }
            }

            string imageName = toString(layer) + '_' + toString(map);
            imwrite(NetworkUtils::getVisualizerImagePath(imageName), grayscale);
        }
    }
}