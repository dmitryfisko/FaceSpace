#include "stdafx.h"
#include <stdio.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <vector>
#include <fstream>

#include "imageminer.h"
#include "networkutils.h"

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
        s += (char) ('0' + val / div);
        val %= div;
        div /= 10;
    }

    return s;
}

int ImageMiner::loadUserPhotos(string userID, 
                                Document::AllocatorType &allocator,
                                FaceDetector &faceDetector) {
    string response = NetworkUtils::requestUserPhotos(userID);
    Document document;
    document.Parse(response.c_str());
    Value &photos = document["response"];

    int len = photos.Size();
    int recognized = 0;
    for (int i = 0; i < len; ++i) {
        Value photo(kObjectType);
        photo.CopyFrom(photos[i], allocator);

        if (!photo.HasMember("src_big")) {
            continue;
        }

        string hqImageUrl = photo["src_big"].GetString();
        string hqImagePath = NetworkUtils::getHQImagePath(userID + "_" + int64ToString(i + 1) + "_hq");

        if (!NetworkUtils::isFileExist(hqImagePath)) {
            NetworkUtils::downloadImageAndCache(hqImageUrl, hqImagePath);
        }
        Mat hqImage = imread(hqImagePath, CV_LOAD_IMAGE_UNCHANGED);
        remove(hqImagePath.c_str());
        if (hqImage.empty()) {
            continue;
        }

        vector<Rect> faces = faceDetector.
            detectEyes(hqImage, FaceDetector::DetectMode::FindAllFaces);
        if (faces.size() != 1) {
            continue;
        }

        Mat faceMat = hqImage(faces[0]);
        imwrite(NetworkUtils::getHQFacePath(userID, int64ToString(i + 1)).c_str(),
                faceMat);
        ++recognized;
    }

    return recognized;
}

void ImageMiner::mine() {
    Document data;
    data.SetArray();
    Document::AllocatorType &allocator = data.GetAllocator();
    int curID = START_ID;
    FaceDetector faceDetector;

    size_t startTime = clock();
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
        
        if (idsInRequest != 0) {
            string response = NetworkUtils::requestProfilesData(userIDs);
            cout << response.length() << endl;

            Document document;

            document.Parse(response.c_str());
            Value &profiles = document["response"];

            int len = profiles.Size();
            for (int i = 0; i < len; ++i) {
                Value profile(kObjectType);
                profile.CopyFrom(profiles[i], allocator);

                string userID = int64ToString(profile["uid"].GetInt64());
                string lqImageUrl = profile["photo_max"].GetString();
                string lqImagePath = NetworkUtils::getLQFacePath(userID + "_lq");
                NetworkUtils::downloadImageAndCache(lqImageUrl, lqImagePath);

                Mat lqImage = imread(lqImagePath, CV_LOAD_IMAGE_UNCHANGED);
                if (!lqImage.empty()) {
                    vector<Rect> lqFaces = faceDetector.
                        detectEyes(lqImage, FaceDetector::DetectMode::CheckHasFace);
                    if (lqFaces.size() > 0) {
                        int recognizedUserPhotos =
                            loadUserPhotos(userID, allocator, faceDetector);
                        if (recognizedUserPhotos > 0) {
                            cout << "   " << userID << endl;
                            ++bigImagesDownloaded;
                        }
                    }
                }
                remove(lqImagePath.c_str());

                data.PushBack(profile, allocator);
            }
        }
    }
    cout << bigImagesDownloaded << " " << clock() - startTime << endl;

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