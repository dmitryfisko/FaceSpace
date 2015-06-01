#include "stdafx.h"
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
    
    if (randomize) {
        bias = ((double)rand() / RAND_MAX * 2 - 1) ;
        for (int i = 0; i < len; ++i) {
            if (randomize) {
                *(data + i) = ((double)rand() / RAND_MAX * 2 - 1);
            }
        }
        //randn();
    } else {
        fill(0.01);
        setBias(0);
    }
}

FeatureExtractor::Array2D::Array2D(Mat &mat) : width(mat.cols), height(mat.rows) {
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
}

void FeatureExtractor::Array2D::randn() {
    // use formula 30.3 of Statistical Distributions (3rd ed)
    // Merran Evans, Nicholas Hastings, and Brian Peacock
    int len = width * height + 2;
    double *random = new double[len];
    for (int i = 0; i < len; ++i) {
        random[i] = (double)std::rand() / RAND_MAX;
        while (random[i] == 0) {
            random[i] = (double)std::rand() / RAND_MAX;
        }
    }

    double pi = 4. * std::atan(1.);
    double square, amp, angle;
    size_t k = 0;
    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            if (k % 2 == 0) {
                square = -2. * std::log(random[k]);
                if (square < 0.) {
                    square = 0.;
                }
                amp = sqrt(square);
                angle = 2. * pi * random[k + 1];
                set(i, j, amp * std::sin(angle));
            } else {
                set(i, j, amp * std::cos(angle));
            }
            k++;
        }
    }
    if (k % 2 == 0) {
        square = -2. * std::log(random[k]);
        if (square < 0.) {
            square = 0.;
        }
        amp = sqrt(square);
        angle = 2. * pi * random[k + 1];
        setBias(amp * std::sin(angle));
    } else {
        setBias(amp * std::cos(angle));
    }
    
    delete []random;
}

void FeatureExtractor::Array2D::normalize() {
    // Does like this or getWidth() * getHeight()
    int len = width * height;
    double sum = 0;
    for (int i = 0; i < len; ++i) {
        sum += data[i] * data[i];
    }
    sum += bias * bias;
    assert(sum >= 0);
    double scale = sqrt(sum);
    if (scale == 0) {
        return;
    }

    //Normalize: sum = items_count / 50;
    double normalizeValue;
    if (bias == 0) {
        normalizeValue = len + 1;
    } else {
        normalizeValue = len;
    }
    normalizeValue /= 50;

    scale /= sqrt(normalizeValue);
    for (int i = 0; i < len; ++i) {
        data[i] /= scale;
    }
    bias /= scale;
}

void FeatureExtractor::Array2D::clear() {
    int len = width * height;
    bias = 0;
    for (int i = 0; i < len; ++i) {
        *(data + i) = 0;
    }
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

void FeatureExtractor::Array2D::fill(double val) {
    int len = width * height;
    for (int i = 0; i < len; ++i) {
        *(data + i) = val;
    }
    bias = val;
}

void FeatureExtractor::Array2D::setBias(double val) {
    bias = val;
}

double FeatureExtractor::Array2D::at(int x, int y) {
    return *(data + width * x + y);
}

void FeatureExtractor::Array2D::set(int x, int y, double val) {
    *(data + width * x + y) = val;
}

void FeatureExtractor::Array2D::add(int x, int y, double val) {
    *(data + width * x + y) += val;
}

void FeatureExtractor::Array2D::addBias(double delta) {
    bias += delta;
}

FeatureExtractor::Array2D::~Array2D() {
    delete[] data;
}