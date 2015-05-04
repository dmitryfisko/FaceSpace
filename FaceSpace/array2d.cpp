#include "featureextractor.h"

FeatureExtractor::Array2D::Array2D(const Array2D &prev) {
    width = prev.width;
    height = prev.height;
    bias = prev.bias;
    int len = width * height;
    data = new double[len];
    memcpy(data, prev.data, sizeof(double) * len);
}

FeatureExtractor::Array2D::Array2D(int width, 
                                   int height) : width(width), height(height) {
    int len = width*height;
    data = new double[len];

    bias = 0;
    for (int i = 0; i < len; ++i) {
            *(data + i) = 0;
    }
}

FeatureExtractor::Array2D::Array2D(int width, 
                                   int height, 
                                   bool randomize) : width(width), height(height) {
    int len = width * height;
    data = new double[len];
    //Does use srand in constructor or from called function?
    
    bias = ((double) rand() / RAND_MAX * 2 - 1) * 2;
    for (int i = 0; i < len; ++i) {
        if (randomize) {
            *(data + i) = ((double) rand() / RAND_MAX * 2 - 1);
        }
    }
    normalize();
}

FeatureExtractor::Array2D::Array2D(Mat &mat,
                                   bool isNormalize) : width(mat.cols), height(mat.rows) {
    data = new double[width * height];
    bias = 0;
    if (mat.channels() > 1) {
        cvtColor(mat, mat, COLOR_BGR2GRAY);
    }

    uint8_t *pixelPtr = (uint8_t*) mat.data;
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            set(i, j, ((double)pixelPtr[i*width + j] - 128) / 255);
        }
    }
    if (isNormalize) {
        normalize();
    }
}

void FeatureExtractor::Array2D::normalize() {
    // Does like this or width*height
    int len = getWidth() * getHeight();
    double sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += data[i] * data[i];
    }
    sum += bias * bias;
    double scale = sqrt(sum);
    if (scale == 0) {
        return;
    }
    for (int i = 0; i < len; ++i) {
        data[i] /= scale;
    }
    bias /= scale;
}

int FeatureExtractor::Array2D::getWidth() {
    return width;
}

int FeatureExtractor::Array2D::getHeight() {
    return height;
}

double FeatureExtractor::Array2D::getBias() {
    return bias;
}

void FeatureExtractor::Array2D::setBias(double val) {
    bias = val;
}

double FeatureExtractor::Array2D::at(int x, int y) {
    return *(data + height*x + y);
}

void FeatureExtractor::Array2D::set(int x, int y, double val) {
    *(data + height*x + y) = val;
}

void FeatureExtractor::Array2D::add(int x, int y, double val) {
    *(data + height*x + y) += val;
}

FeatureExtractor::Array2D::~Array2D() {
    delete[] data;
}