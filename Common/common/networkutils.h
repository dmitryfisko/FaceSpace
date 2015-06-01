#include <string>
#include <vector>
#include <map>

using namespace std;

class NetworkUtils {
    private:
        static const string MINE_LQ_FACES_FOLDER_PATH;
        static const string MINE_HQ_FACES_FOLDER_PATH;
        static const string MINE_LQ_IMAGES_FOLDER_PATH;
        static const string MINE_HQ_IMAGES_FOLDER_PATH;
        static const string VISUALIZER_FOLDER_PATH;
        static const string TRAIN_MAINER_FOLDER_PATH;
        static const string TEST_LFW_FOLDER_PATH;
        static const string WEIGHTS_FOLDER_PATH;
        static const int TIME_BEETWEEN_REQUESTS = 334;

        struct ResponseHolder;
        static size_t prevRequestTime;

        static size_t writeFuncMem(void *ptr, size_t size, size_t nmemb, struct ResponseHolder *s);
        static size_t writeFuncFile(void *ptr, size_t size, size_t nmemb, FILE *stream);
        static void initString(struct ResponseHolder *s);
        static string loadUrl(string &url);
    public:
        static void removeAllWeights();
        static vector< vector<string> > loadTrainSetMiner();
        static map<string, vector<string>> loadPairsTestSetLFW();
        static string requestProfilesData(string ids);
        static string requestUserPhotos(string id);
        static void downloadImageAndCache(string imageUrl, string imagePath);
        static bool isFileExist(string fileName);
        static string getLQFacePath(string fileName);
        static string getHQFacePath(string userID, string photoNum);
        static string getHQImagePath(string fileName);
        static string getLQImagePath(string fileName);
        static string getVisualizerImagePath(string fileName);
};