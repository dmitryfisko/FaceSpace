#pragma once

#include <vector>
#include <cmath>
#include <stdint.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

class FeatureExtractor {
private:
    struct SIZES {
        static const int LAYERS_COUNT = 8;
        static const int LAYER_EDGE[LAYERS_COUNT];
        static const int LAYER_MAPS[LAYERS_COUNT];
        static const int CONV_LAYERS = 4;
        static const int CONV_MAPS[CONV_LAYERS];
        static const int CONV_EDGE[CONV_LAYERS];
        static const int LAST_LAYER_NUM = LAYERS_COUNT - 1;
    };


    static const int LAYER_INPUT = 0;
    static const int LAYER_CONV = 1;
    static const int LAYER_POOL = 2;
    static const int LAYER_FULL_CONNECTED = 3;
    static const int LAYER_L2_NORM = 4;
    static const int LAYER_TYPE[SIZES::LAYERS_COUNT];
    static const int AUTO = 0;

    const enum Distance { Minimize, Maximize };

    class Array2D {
    private:
        double *data;
        double bias;
        int width;
        int height;

        void randn();
    public:
        Array2D(const Array2D &prev);
        Array2D(Mat &mat);
        Array2D(int width, int height);
        Array2D(int width, int height, bool randomize);
        ~Array2D();

        void normalize();
        void clear();
        int getWidth();
        int getHeight();
        double getBias();
        void fill(double val);
        void setBias(double val);
        void addBias(double delta);
        double at(int x, int y);
        void set(int x, int y, double val);
        void add(int x, int y, double val);
    };

    class Weights {
    private:
        int CONV_LAYER_MAPS[SIZES::CONV_LAYERS];
        int CONV_LAYER_EDGE[SIZES::CONV_LAYERS];
        int CONV_LAYER_TYPE[SIZES::CONV_LAYERS];

        vector< Array2D > weights[SIZES::CONV_LAYERS];
        int trainEpoch;
    public:
        Weights();

        vector< Array2D > operator [](int i) const { return weights[i]; }
        vector< Array2D > &operator [](int i) { return weights[i]; }

        void incTrainEpoch();
        int getTrainEpoch();
        void save();
        void save(int num);
    };

    class Visializer {
    private:
        string toString(int val);
    public:
        void show(vector<Array2D> mapsLayers[]);
    };

    //How does underlay global variable?
    bool isExtractorTrain = false;
    bool isVisualize = false;
    double learningRate;

    vector<Array2D> layersMaps[SIZES::LAYERS_COUNT];
    vector<Array2D> layersMapsDeriv[SIZES::LAYERS_COUNT];
    vector<Array2D> layersMapsError[SIZES::LAYERS_COUNT];
    Weights weights;

    inline double convActiv(double sum);
    inline double convActivDeriv(double sum);
    inline double normL2Activ(double val);
    inline double normL2ActivDeriv(double val);
    inline double poolActiv(double sum);
    inline double poolActivDeriv(double sum);
    inline Rect getPointLocalFields(int i, int j,
                                    int layerEdge, int convEdge);
    void conv(Array2D &prevMap, int layerNum, int &mapNum, int convNum);
    void fullConnected(int layerNum, int convNum);
    void pool(Array2D &prevMap, int layerNum, int mapNum);
    void normL2(int layerNum);
    void backpropagation(vector<double> &goal, vector<double> &v, Distance mode);
public:
    FeatureExtractor();

    const enum TrainMode { Continue, New };

    vector<double> getVector(Mat &mat);
    vector<double> getVector(Mat &mat, bool visualize);
    void train(TrainMode MODE, int trainInterations);

};