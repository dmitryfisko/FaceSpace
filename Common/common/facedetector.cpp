#include "stdafx.h"
#include <iostream>
#include "facedetector.h"

using namespace std;
using namespace cv;

FaceDetector::FaceDetector() {
    bool faceLoad = face_cascade.load(FACE_CASSCADES);
    bool eyeLoad = eye_cascade.load(EYE_CASSCADES);
    assert(faceLoad && eyeLoad);
}

struct FaceDetector::EyeInfo {
    Point edge;		// его левый верхний угол описывающего его квадрата
    Point2f center; // центр глаза
    float radius;
    int width;
    bool isInvalid;
    EyeInfo() {}
    EyeInfo(int xSquareEdge, int ySquareEdge, int SquareWidth) : edge(Point(xSquareEdge, ySquareEdge)), width(SquareWidth), isInvalid(false) {
        radius = width / 2;
        center = Point2f(xSquareEdge + radius, ySquareEdge + radius);
    }
};

long double FaceDetector::getEyeRotateAngel(Point2f &p1, Point2f &p2) {
    bool swapEye = p1.x > p2.x;
    long double xDif = p2.x - p1.x;
    long double yDif = p2.y - p1.y;
    // обязательное условие упорядочить по x, иначе угол будет определяться неправильно
    if (p1.x > p2.x) {
        xDif = -xDif;
        yDif = -yDif;
    }
    return -atan2(yDif, xDif) * (180 / 3.14);
}

long double FaceDetector::sqr(long double val) {
    return (long double) val * val;
}

void FaceDetector::find_scc(int v, bool was[], vector<int> graph[], vector<int> &path) {
    if (was[v]) return;
    was[v] = true;
    path.push_back(v);

    int count = graph[v].size();
    for (int i = 0; i < count; i++) {
        int to = graph[v][i];
        if (!was[to]) {
            was[to] = true;
            path.push_back(to);
            find_scc(to, was, graph, path);
        }
    }
}

//Is make rect const?
Rect FaceDetector::scaleRectSize(Rect &rect, double scale, int limitX, int limitY) {
    assert(scale > 0);
    assert(rect.width == rect.height);

    int offset = rect.width * (scale - 1) / 2;
    offset = min(offset, rect.x);
    offset = min(offset, rect.y);
    offset = min(offset, limitX - rect.x - rect.width);
    offset = min(offset, limitY - rect.y - rect.height);
    // Did it placed in heap?
    return Rect(rect.x - offset, rect.y - offset, 
                rect.width + offset * 2, rect.height + offset * 2);
}

Rect FaceDetector::scaleRect(Rect &r, double scale) {
    return Rect(r.x * scale, r.y * scale,
                r.width * scale, r.height * scale);
}


vector<Rect> FaceDetector::detect(Mat &image, DetectMode MODE) {
    Mat frameGray;
    vector<Rect> faces;
    vector<Rect> checkedFaces;

    assert(!image.empty());

    image.copyTo(frameGray);
    if (frameGray.channels() > 1) {
        cvtColor(frameGray, frameGray, COLOR_BGR2GRAY);
    }
    equalizeHist(frameGray, frameGray);

    //const clock_t begin_time = clock();
    face_cascade.detectMultiScale(frameGray, faces, 1.05, 1, 0 | CV_HAAR_SCALE_IMAGE, Size(10, 10));
    //cout << "   face recogn. time: " << clock() - begin_time << endl;

    for (int i = 0; i < faces.size(); ++i) 	{
        //debug
        //Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].width * 0.5 );
        //ellipse(frame, center, Size(faces[i].width * 0.5, faces[i].width * 0.5), 
        //       0, 0, 360, Scalar(0, 255, 255), 4, 8, 0);

        Mat faceROI = frameGray(faces[i]);
        vector<Rect> eyes;

        //-- In each face, detect eyes
        //eyes_cascade.detectMultiScale(faceROI, eyes, 1.025, 4, 0|CV_HAAR_SCALE_IMAGE, Size(10, 10));
        //const clock_t begin_time2 = clock();
        eye_cascade.detectMultiScale(faceROI, eyes, 1.01, 0, 0 | CV_HAAR_SCALE_IMAGE, Size(5, 5));
        //cout << "       eye recogn. time: " << clock() - begin_time2 << endl;

        //vector<EyeInfo> eyesInfo(eyes.size());
        int eyesCount = eyes.size(), validEyesCount = eyesCount;
        EyeInfo *eyesInfo = new EyeInfo[eyesCount];

        for (int j = 0; j < eyesCount; ++j) {
            eyesInfo[j] = EyeInfo(faces[i].x + eyes[j].x,
                                  faces[i].y + eyes[j].y,
                                  eyes[j].width);
            //Point center( eyesInfo[j].center.x, eyesInfo[j].center.y );
            //ellipse( originalFrame, center, Size( eyesInfo[j].width*0.5, eyesInfo[j].width*0.5), 0, 0, 360, Scalar( 255, 0, 0 ), 4, 8, 0 );
        }

        // 1) выкидываем глаза лежащие вне круга лица
        //    но не трогаем если их кол-во 2 и меньше
        //
        // 2) если глаз лежит ниже нижней 1/3 лица или 
        //    пересекает её, то выкидываем его
        long double faceR = faces[i].width / 2.0;
        long double faceCenterX = faces[i].x + faceR;
        long double faceCenterY = faces[i].y + faceR;
        long double faceBottomLimit = faceCenterY + faceR / 6;
        faceR *= 0.95;
        for (int j = 0; j < eyesCount; ++j) {
            long double dist = sqrt(sqr(faceCenterX - eyesInfo[j].center.x) +
                                    sqr(faceCenterY - eyesInfo[j].center.y));
            long double r = eyesInfo[j].radius;
            long double eyeBottomEdge = eyesInfo[j].center.y + eyesInfo[j].radius;
            //cout << "   faceBottomLimit: " << faceBottomLimit << "   eyeBottomEdge: " << eyeBottomEdge << endl;
            //cout << "   dist: " << dist << "   r: " << r << "   faceR: " << faceR << endl;

            if (((dist + r > faceR) || (faceBottomLimit < eyeBottomEdge)) && validEyesCount != 2) {
                eyesInfo[j].isInvalid = true;
                validEyesCount--;
            }
        }

        // если круг в круге, то выкидываем наибольший
        for (int j = 0; j < eyesCount - 1; ++j) {
            if (eyesInfo[j].isInvalid) continue;
            for (int k = j + 1; k < eyesCount; ++k) {
                if (eyesInfo[k].isInvalid) {
                    continue;
                }
                long double dist = sqrt(sqr(eyesInfo[j].center.x - eyesInfo[k].center.x) +
                                        sqr(eyesInfo[j].center.y - eyesInfo[k].center.y));
                long double r = min(eyesInfo[j].radius, eyesInfo[k].radius);
                long double R = max(eyesInfo[j].radius, eyesInfo[k].radius);

                if (dist + r <= R) {
                    if (eyesInfo[j].radius > eyesInfo[k].radius) {
                        eyesInfo[j].isInvalid = true;
                        --validEyesCount;
                        break;
                    } else {
                        eyesInfo[k].isInvalid = true;
                        --validEyesCount;
                        continue;
                    }
                }
            }
        }

        //debug
        //for(int j = 0; j < eyesCount; j++) {
        //	if(eyesInfo[j].isInvalid) {
        //		Point center( eyesInfo[j].center.x, eyesInfo[j].center.y );
        //		ellipse( originalFrame, center, Size( eyesInfo[j].width*0.5, eyesInfo[j].width*0.5), 0, 0, 360, Scalar( 0, 0, 255 ), 4, 8, 0 );
        //	}
        //}

        if (validEyesCount <= 1) {
            continue;
        }

        // в сильносвязных компонентах с более чем 1 вершиной
        // удаляем все кроме самого левого и правого
        bool *was = new bool[eyesCount];
        int maxLeftInd = -1, maxRightInd = -1;
        for (int j = 0; j < eyesCount; ++j) {
            was[j] = eyesInfo[j].isInvalid;
            if (was[j]) {
                continue;
            }
            if (maxLeftInd == -1) {
                maxLeftInd = j;
            }
            if (maxRightInd == -1) {
                maxRightInd = j;
            }

            long double eyeEdgeX = eyesInfo[j].edge.x;
            if (eyeEdgeX < eyesInfo[maxLeftInd].edge.x) {
                maxLeftInd = j;
            }
            long double eyeRightEdge = eyeEdgeX + eyesInfo[j].width;
            if (eyeRightEdge >= eyesInfo[maxRightInd].edge.x + eyesInfo[maxRightInd].width) {
                maxRightInd = j;
            }
        }
        vector<int> *graph = new vector<int>[eyesCount];
        for (int j = 0; j < eyesCount - 1; j++) {
            for (int k = j + 1; k < eyesCount; k++) {
                long double dist = sqrt(sqr(eyesInfo[j].center.x - eyesInfo[k].center.x) +
                                        sqr(eyesInfo[j].center.y - eyesInfo[k].center.y));
                long double r = eyesInfo[j].radius, r2 = eyesInfo[k].radius;
                if (dist > abs(r2 - r) && dist < r + r2) {
                    graph[j].push_back(k);
                    graph[k].push_back(j);
                }
            }
        }
        for (int j = 0; j < eyesCount; ++j) {
            if (was[j]) {
                continue;
            }
            vector<int> path;
            find_scc(j, was, graph, path);
            int count = path.size();
            //cout << "            ";
            //for(int k = 0; k < count; k++) cout << path[k] << " ";
            //cout << endl;
            if (count <= 1) {
                continue;
            }

            bool justMaxLeftAndMaxRight = true;
            for (int k = 0; k < count; ++k) {
                int to = path[k];
                if (to != maxLeftInd && to != maxRightInd) {
                    eyesInfo[to].isInvalid = true;
                    justMaxLeftAndMaxRight = false;
                }
            }
            if (count == 2 && justMaxLeftAndMaxRight) {
                eyesInfo[path[0]].isInvalid = true;
                eyesInfo[path[1]].isInvalid = true;
            }
        }
        delete[] was;
        delete[] graph;

        //debug
        //for(int j = 0; j < eyesCount; j++) {
        //	if(eyesInfo[j].isInvalid) {
        //		Point center( eyesInfo[j].center.x, eyesInfo[j].center.y );
        //		ellipse(frame, center, Size( eyesInfo[j].width*0.5, eyesInfo[j].width*0.5), 
        //                0, 0, 360, Scalar( 0, 0, 255 ), 4, 8, 0 );
        //	}
        //}

        int maxSizeInd = -1;
        for (int j = 0; j < eyesCount; ++j) {
            if (eyesInfo[j].isInvalid) {
                continue;
            }
            if (maxSizeInd == -1 || eyesInfo[j].width >  eyesInfo[maxSizeInd].width) {
                maxSizeInd = j;
            }
        }

        if (maxSizeInd == -1) {
            continue;
        }

        int minAngleInd = -1;
        long double minAngle = 1000;
        for (int j = 0; j < eyesCount; j++) {
            if (eyesInfo[j].isInvalid || j == maxSizeInd) {
                continue;
            }
            long double angle = abs(getEyeRotateAngel(
                eyesInfo[maxSizeInd].center, eyesInfo[j].center));
            if (angle < minAngle) {
                minAngleInd = j;
                minAngle = angle;
            }
        }

        if (minAngleInd != -1 && minAngle <= 70) {
            if (MODE == DetectMode::CheckHasFace) {
                checkedFaces.push_back(faces[i]);
                return checkedFaces;
            }
            checkedFaces.push_back( 
                scaleRectSize(faces[i], 1.5, frameGray.cols, frameGray.rows) );
        }

        delete[] eyesInfo;
    }

    return checkedFaces;
}