#include <string>
#include <vector>
#include <map>

using namespace std;

class NetworkUtils {
    private:
        static string MINE_LQ_FACES_FOLDER_PATH;
        static string MINE_HQ_FACES_FOLDER_PATH;
        static string MINE_LQ_IMAGES_FOLDER_PATH;
        static string MINE_HQ_IMAGES_FOLDER_PATH;
        static string VISUALIZER_FOLDER_PATH;
        static string TRAIN_FOLDER_PATH;
        static string TEST_LFW_FOLDER_PATH;

        struct ResponseHolder;

        static size_t writeFuncMem(void *ptr, size_t size, size_t nmemb, struct ResponseHolder *s);
        static size_t writeFuncFile(void *ptr, size_t size, size_t nmemb, FILE *stream);
        static void initString(struct ResponseHolder *s);
    public:
        static vector<string> loadTrainSet();
        static map<string,  vector<string>> loadTestSetLFW();
        static string requestUsersData(string ids);
        static void downloadImageAndCache(string imageUrl, string imagePath);
        static bool isFileExist(string fileName);
        static string getLQFacePath(string fileName);
        static string getHQFacePath(string fileName);
        static string getHQImagePath(string fileName);
        static string getLQImagePath(string fileName);
        static string getVisualizerImagePath(string fileName);

};