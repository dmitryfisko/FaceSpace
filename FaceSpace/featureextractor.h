#include <vector>
#include <cmath>
#include <stdint.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;

class FeatureExtractor {
private:
        
    static const int LAYER_INPUT = 0;
    static const int LAYER_CONV = 1;
    static const int LAYER_POOL = 2;
    static const int LAYER_NEIRON = 3;
    static const int LAYER_TYPE[10];

    struct SIZES {
        static const int LAYER_EDGE[10];
        static const int LAYER_MAPS[10];
        static const int LAYERS_COUNT = 10;
        static const int CONV_MAPS[5];
        static const int CONV_EDGE[5];
        static const int CONV_LAYERS = 5;
        static const int CONV_LAST_LAYER_NUM = CONV_LAYERS - 1;
        static const int COMPETITIVE_FIELD_EDGE[5];
        static const int COMPETITIVE_WINNERS = 5;
        static const double MIN_VAL;
    };

    class Maximator {
    private:
        struct Item {
            int x;
            int y;
            double val;
            Item() {}
            Item(int x, int y, double val) : x(x), y(y), val(val) {}
        };

        Item *items;
        int counter;

        inline void push(Item &temp);
    public:
        Maximator();
        ~Maximator();

        void add(int x, int y, double val);
        int size();
        void getXY(int index, int &x, int &y);
        void clear();
    };

    class Array2D {
    private:
        double *data;
        double bias;
        int width;
        int height;
    public:
        Array2D(const Array2D &prev);
        Array2D(Mat &mat, bool isNormalize);
        Array2D(int width, int height);
        Array2D(int width, int height, bool randomize);
        ~Array2D();

        void normalize();
        int getWidth();
        int getHeight();
        double getBias();
        void setBias(double val);
        double at(int x, int y);
        void set(int x, int y, double val);
        void add(int x, int y, double val);
    };

    class Weights {
    private:
        int CONV_LAYER_MAPS[SIZES::CONV_LAYERS];
        int CONV_LAYER_EDGE[SIZES::CONV_LAYERS];

        vector< Array2D > weights[SIZES::CONV_LAYERS];
    public:
        vector< Array2D > operator [](int i) const { return weights[i]; }
        vector< Array2D > &operator [](int i) { return weights[i]; }
                
        Weights();
        ~Weights();
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
    int trainEpoch = 1;
    double baseLearningSpeed = 0.05;

    vector<Array2D> mapsLayers[SIZES::LAYERS_COUNT];
    Weights weights;

    inline double activation(double impulse);
    void conv(vector<Array2D> &res, Array2D &prevMap, Array2D &sumMap,
              Maximator &maximator,
              int layerNum, int &mapNum, int convNum);
    void pool(Array2D &poolMap, Array2D &prevMap, int layerNum);

public:
    FeatureExtractor();

    const enum TrainMode { Continue, New };

    vector<double> getVector(Mat &mat);
    vector<double> getVector(Mat &mat, bool visualize);
    void train(TrainMode MODE);

};