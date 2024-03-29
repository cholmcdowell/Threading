#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    // 3. Then push the vector at the end of the queue
    // 4. Wake up threads that were waiting for push

    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    vector<char> data(msg, msg + size);
    unique_lock<mutex> l(mtx);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    slot_available.wait(l, [this]{return (int)q.size() < cap;});
    // 3. Then push the vector at the end of the queue
    q.push(data);
    // 4. Wake up threads that were waiting for push
    l.unlock();
    data_available.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {
    // 1. Wait until the queue has at least 1 item
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    // 4. Wake up threads that were waiting for pop
    // 5. Return the vector's length to the caller so that they know how many bytes were popped

    // 1. Wait until the queue has at least 1 item
    unique_lock<mutex> l(mtx);
    data_available.wait(l, [this]{return (int)q.size() > 0;});
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> data;
    data = q.front();
    q.pop();
    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    assert((int)data.size() <= size);
    char* buff = data.data();
    memcpy(msg, buff, (int)data.size());
    // 4. Wake up threads that were waiting for pop
    l.unlock();
    slot_available.notify_one();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return data.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}