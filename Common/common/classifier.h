#include <vector>
#include <cmath>
#include <iostream>
#include <assert.h>

#define sqr(x) ((x)*(x)) 

using namespace std;

class Classifier {
    private:
        static const double MAX_DIST;
        static const double THRESHOLD;

        struct Point {
            vector<double> coords;
            __int64 uid;
            __int64 number;
            Point();
            Point(vector<double> &coords, __int64 uid, __int64 number);
        };

        __int64 prevUniqueUID = 0;
        __int64 number = 0;
        vector<Point> points;

        double distance(vector<double> &v1, vector<double> &v2);
    public:
        __int64 getUID(vector<double> &point);
        bool isSame(vector<double> &v1, vector<double> &v2);
};