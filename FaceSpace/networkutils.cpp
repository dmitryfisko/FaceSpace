#include "networkutils.h"
#include <dirent.h>
#include <assert.h>
#include <string>
#include <curl/curl.h>
#include <io.h>

string NetworkUtils::MINE_LQ_FACES_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\faces_lq\\";
string NetworkUtils::MINE_HQ_FACES_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\faces_hq\\";
string NetworkUtils::MINE_LQ_IMAGES_FOLDER_PATH = MINE_LQ_FACES_FOLDER_PATH;
string NetworkUtils::MINE_HQ_IMAGES_FOLDER_PATH = MINE_LQ_FACES_FOLDER_PATH;
string NetworkUtils::VISUALIZER_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\visualizer\\";
string NetworkUtils::TRAIN_FOLDER_PATH = MINE_HQ_FACES_FOLDER_PATH;
string NetworkUtils::TEST_LFW_FOLDER_PATH = "d:\\X\\FaceSpace\\Datasets\\lfw\\";

vector<string> NetworkUtils::loadTrainSet() {
    vector<string> images;
    
    struct dirent *ent;
    DIR *dir = opendir(TRAIN_FOLDER_PATH.c_str());
    assert(dir != NULL);

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0 ||
            ent->d_type == DT_DIR) {
            continue;
        }

        string image_path = TRAIN_FOLDER_PATH + ent->d_name;
        if (image_path.find("_face_hq_") != string::npos) {
            images.push_back(image_path);
        }
    }
    
    closedir(dir);

    return images;
}

map<string, vector<string>> NetworkUtils::loadTestSetLFW() {
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
    s->ptr = (char*)realloc(s->ptr, new_len + 1);
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

string NetworkUtils::requestUsersData(string ids) {
    CURL *curl = curl_easy_init();
    string data = "";

    if (curl) {
        CURLcode res;
        string url = "http://api.vk.com/method/users.get?user_ids=" + ids + "&fields=photo_max,photo_max_orig";

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

string NetworkUtils::getLQFacePath(string fileName) {
    return MINE_LQ_FACES_FOLDER_PATH + fileName + ".jpg";
}

string NetworkUtils::getHQFacePath(string fileName) {
    return MINE_HQ_FACES_FOLDER_PATH + fileName + ".jpg";
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