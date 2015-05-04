#include <string>

class ImageMiner {
private:
    static const __int64 START_ID = 1001;
    static const __int64 END_ID = 100000;
    static const int IDS_PER_REQUEST = 300;
    static const int TIME_BEETWEEN_REQUESTS = 334;

    std::string int64ToString(__int64 val);
public:
    void mine();
};