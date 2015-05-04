#include <stdio.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <vector>
#include <windows.h>
#include <fstream>

#include "imageminer.h"
#include "facedetector.h"
#include "networkutils.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define minimum(x, y) ((x) < (y)) ? (x): (y);

using namespace std;
using namespace rapidjson;

string ImageMiner::int64ToString(__int64 val) {
    string s = "";
    __int64 div = 1;
    while (val >= 10 * div) {
        div *= 10;
    }
    while (div) {
        s += (char)('0' + val / div);
        val %= div;
        div /= 10;
    }

    return s;
}
void ImageMiner::mine() {
    Document data;
    data.SetArray();
    Document::AllocatorType& allocator = data.GetAllocator();
    int curID = START_ID;
    size_t prevRequestTime = 0;
    FaceDetector face_detector;

    size_t now = clock();
    int bigImagesDownloaded = 0;
    while (curID <= END_ID) {
        string userIDs;
        int idsInRequest = minimum(IDS_PER_REQUEST, END_ID - curID + 1);
        for (int j = 0; j < idsInRequest && curID <= END_ID; ++j, ++curID) {
            userIDs += int64ToString(curID);
            if (j != idsInRequest - 1) {
                userIDs += ',';
            }
        }

        size_t timeDif = clock() - prevRequestTime;
        if (timeDif < TIME_BEETWEEN_REQUESTS) {
            Sleep(TIME_BEETWEEN_REQUESTS - timeDif);

        }
        if (idsInRequest != 0) {
            prevRequestTime = clock();
            string response = NetworkUtils::requestUsersData(userIDs);
            cout << response.length() << endl;

            Document document;

            document.Parse(response.c_str());
            Value& s = document["response"];

            int len = s.Size();
            for (int i = 0; i < len; ++i) {
                Value obj(kObjectType);
                obj.CopyFrom(s[i], allocator);

                string userID = int64ToString(obj["uid"].GetInt64());
                string lqImageUrl = obj["photo_max"].GetString();
                string lqImagePath = NetworkUtils::getLQFacePath(userID + "_lq");
                NetworkUtils::downloadImageAndCache(lqImageUrl, lqImagePath);

                Mat lqImage = imread(lqImagePath, CV_LOAD_IMAGE_UNCHANGED);
                if (!lqImage.empty()) {
                    vector<Rect> lqFaces = face_detector.
                        detectEyes(lqImage, FaceDetector::DetectMode::CheckHasFace);
                    if (lqFaces.size() > 0) {
                        string hqImageUrl = obj["photo_max_orig"].GetString();
                        string hqImagePath = NetworkUtils::getHQImagePath(userID + "_hq");

                        if (!NetworkUtils::isFileExist(hqImagePath)) {
                            NetworkUtils::downloadImageAndCache(hqImageUrl, hqImagePath);
                        }

                        Mat hqImage = imread(hqImagePath, CV_LOAD_IMAGE_UNCHANGED);
                        if (!hqImage.empty()) {
                            vector<Rect> faces = face_detector.
                                detectEyes(hqImage, FaceDetector::DetectMode::FindAllFaces);
                            for (int j = 0; j < faces.size(); ++j) {
                                Mat faceMat = hqImage(faces[j]);
                                imwrite(NetworkUtils::getHQFacePath(
                                    userID + "_face_hq_" + int64ToString(j + 1)).c_str(),
                                    faceMat);
                            }
                        }

                        ++bigImagesDownloaded;
                    }
                }
                remove(lqImagePath.c_str());

                data.PushBack(obj, allocator);
            }
        }
    }
    cout << bigImagesDownloaded << " " << clock() - now << endl;

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    data.Accept(writer);


    ofstream out("vk_users_data.json");
    out << buffer.GetString();

    //wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    //wstring convertedData = converter.from_bytes(buffer.GetString());
    //wcout << convertedData << endl;

    /* size_t size = strlen(buffer.GetString()) + 1;
    wchar_t* wa = new wchar_t[size];
    mbstowcs(wa, buffer.GetString(), size);
    wcout << wa;*/
}