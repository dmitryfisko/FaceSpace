#include <vector>
#include <cmath>
#include <iostream>

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
    public:
        __int64 getUID(vector<double> &point);
        double getDif(vector<double> &v1, vector<double> &v2);
        bool isSame(vector<double> &v1, vector<double> &v2);
};