#include <string>
#include <common/facedetector.h>
#include "rapidjson/document.h"

using namespace std;
using namespace rapidjson;

class ImageMiner {
private:
    static const __int64 START_ID = 1;
    static const __int64 END_ID = 100000;
    static const int IDS_PER_REQUEST = 300;

    string int64ToString(__int64 val);
    int loadUserPhotos(string userID, 
                       Document::AllocatorType &allocator,
                       FaceDetector &faceDetector);
public:
    void mine();
};