#include "featureextractor.h"

FeatureExtractor::Maximator::Maximator(int limit) : limit(limit) {
    assert(SIZES::COMPETITIVE_WINNERS > 0);
    items = new Item[limit];
    counter = 0;
}

FeatureExtractor::Maximator::~Maximator() {
    delete[] items;
}

void FeatureExtractor::Maximator::push(Item &first) {
    int i = 1;
    while (first.val > items[i].val && i < counter) {
        items[i - 1] = items[i];
        ++i;
    }
    items[i - 1] = first;
}

void FeatureExtractor::Maximator::add(int x, int y, double val) {
    if (counter < limit) {
        for (int i = counter; i >= 1; --i) {
            items[i] = items[i - 1];
        }
        ++counter;
        Item first(x, y, val);
        push(first);
    } else if (items[0].val < val) {
        Item first(x, y, val);
        push(first);
    }
}

int FeatureExtractor::Maximator::size() {
    return counter;
}

void FeatureExtractor::Maximator::getXY(int index, int &x, int &y) {
    assert(0 <= index && index <= SIZES::COMPETITIVE_WINNERS);
    x = items[index].x;
    y = items[index].y;
}

void FeatureExtractor::Maximator::clear() {
    counter = 0;
}