#include <common/stdafx.h>
#include "networkutils.h"
#include <dirent.h>
#include <direct.h>
#include <assert.h>
#include <string>
#include <curl/curl.h>
#include <io.h>

const string NetworkUtils::MINE_LQ_FACES_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\imageminer\\faces_all\\";
const string NetworkUtils::MINE_HQ_FACES_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\imageminer\\faces_hq\\";
const string NetworkUtils::MINE_LQ_IMAGES_FOLDER_PATH = MINE_LQ_FACES_FOLDER_PATH;
const string NetworkUtils::MINE_HQ_IMAGES_FOLDER_PATH = MINE_LQ_FACES_FOLDER_PATH;
const string NetworkUtils::VISUALIZER_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\visualizer\\";
const string NetworkUtils::TRAIN_MAINER_FOLDER_PATH = MINE_HQ_FACES_FOLDER_PATH;
const string NetworkUtils::TEST_LFW_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\lfw\\";
const string NetworkUtils::WEIGHTS_FOLDER_PATH = "d:\\X\\FaceSpace\\Common\\";
size_t NetworkUtils::prevRequestTime = 0;

void NetworkUtils::removeAllWeights() {
    struct dirent *ent;
    DIR *dir = opendir(WEIGHTS_FOLDER_PATH.c_str());
    assert(dir != NULL);

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0 ||
            ent->d_type == DT_DIR) {
            continue;
        }

        string fileName = ent->d_name;
        if (fileName.find("weights") != string::npos &&
            fileName.find(".dat") != string::npos) {
            remove((WEIGHTS_FOLDER_PATH + fileName).c_str());
        }
    }

    closedir(dir);
}

vector< vector<string> > NetworkUtils::loadTrainSetMiner() {
    vector< vector<string> > trainSet;

    struct dirent *ent;
    DIR *dir = opendir(TRAIN_MAINER_FOLDER_PATH.c_str());
    assert(dir != NULL);

    int userNum = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0 ||
            ent->d_type != DT_DIR) {
            continue;
        }

        string folderName = ent->d_name;
        string humanFolder = TRAIN_MAINER_FOLDER_PATH + folderName + '\\';

        DIR *innerDir = opendir(humanFolder.c_str());
        struct dirent *innerEnt;

        int photoNum = -1;
        while ((innerEnt = readdir(innerDir)) != NULL) {
            if (strcmp(innerEnt->d_name, ".") == 0 ||
                    strcmp(innerEnt->d_name, "..") == 0 ||
                innerEnt->d_type == DT_DIR) {
                continue;
            }

            string imagePath = humanFolder + innerEnt->d_name;
            if (++photoNum == 0) {
                trainSet.push_back(vector<string>());
            }
            trainSet[userNum].push_back(imagePath);
        }
        if (photoNum != -1) {
            ++userNum;
        }

        closedir(innerDir);
    }

    closedir(dir);

    return trainSet;
}

map<string, vector<string>> NetworkUtils::loadPairsTestSetLFW() {
    map<string, vector<string>> trainSet;
    struct dirent *ent;
    DIR *dir = opendir(TEST_LFW_FOLDER_PATH.c_str());
    assert(dir != NULL);

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0 ||
                ent->d_type != DT_DIR) {
            continue;
        }

        string humanName = ent->d_name;
        string humanFolder = TEST_LFW_FOLDER_PATH + humanName + '\\';

        DIR *innerDir = opendir(humanFolder.c_str());
        struct dirent *innerEnt;

        while ((innerEnt = readdir(innerDir)) != NULL) {
            if (strcmp(innerEnt->d_name, ".") == 0 ||
                    strcmp(innerEnt->d_name, "..") == 0 ||
                    innerEnt->d_type == DT_DIR) {
                continue;
            }

            string imagePath = humanFolder + innerEnt->d_name;
            trainSet[humanName].push_back(imagePath);
        }

        closedir(innerDir);
    }

    closedir(dir);

    return trainSet;
}

struct NetworkUtils::ResponseHolder {
    char *ptr;
    size_t len;
};

void NetworkUtils::initString(struct ResponseHolder *s) {
    s->len = 0;
    s->ptr = (char*)malloc(s->len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t NetworkUtils::writeFuncMem(void *ptr, size_t size, size_t nmemb, struct ResponseHolder *s) {
    size_t new_len = s->len + size*nmemb;
    s->ptr = (char*) realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

size_t NetworkUtils::writeFuncFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

string NetworkUtils::loadUrl(string &url) {
    size_t timeDif = clock() - prevRequestTime;
    if (timeDif < TIME_BEETWEEN_REQUESTS) {
        Sleep(TIME_BEETWEEN_REQUESTS - timeDif);
    }
    prevRequestTime = clock();

    CURL *curl = curl_easy_init();
    string data = "";

    if (curl) {
        CURLcode res;

        struct ResponseHolder response;
        initString(&response);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFuncMem);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        data = response.ptr;
        free(response.ptr);

        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return data;
}

string NetworkUtils::requestProfilesData(string ids) {
    string url = "http://api.vk.com/method/users.get?user_ids=" + ids + "&fields=photo_max,photo_max_orig";
    return loadUrl(url);
}

string NetworkUtils::requestUserPhotos(string id) {
    string url = "http://api.vk.com/method/photos.get?owner_id=" + id + "&album_id=profile&rev=1&extended=0&count=40&version=4.30";
    return loadUrl(url);
}

string NetworkUtils::getLQFacePath(string fileName) {
    return MINE_LQ_FACES_FOLDER_PATH + fileName + ".jpg";
}

string NetworkUtils::getHQFacePath(string userID, string faceNum) {
    const string USER_FACES_FOLDER = MINE_HQ_FACES_FOLDER_PATH + userID + '\\';
    _mkdir(USER_FACES_FOLDER.c_str());
    return USER_FACES_FOLDER + "face_" + faceNum + ".jpg";
}

string NetworkUtils::getLQImagePath(string fileName) {
    return MINE_LQ_IMAGES_FOLDER_PATH + fileName + ".jpg";
}

string NetworkUtils::getHQImagePath(string fileName) {
    return MINE_HQ_IMAGES_FOLDER_PATH + fileName + ".jpg";
}

string NetworkUtils::getVisualizerImagePath(string fileName) {
    return VISUALIZER_FOLDER_PATH + fileName + ".jpg";
}

void NetworkUtils::downloadImageAndCache(string imageUrl, string imagePath) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    const char *url = imageUrl.c_str();
    char outfilename[FILENAME_MAX];

    strncpy_s(outfilename, imagePath.c_str(), sizeof(outfilename));
    outfilename[FILENAME_MAX - 1] = 0;

    curl = curl_easy_init();
    if (curl) {
        fopen_s(&fp, outfilename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFuncFile);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

bool NetworkUtils::isFileExist(string filePath) {
    return (_access(filePath.c_str(), 0) != -1);
}