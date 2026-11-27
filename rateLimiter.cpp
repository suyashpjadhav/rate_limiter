#include <chrono>
#include <deque>
#include <mutex>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

class RateLimiter {
private:
    int limit;
    deque<steady_clock::time_point> timestamps;
    mutex mtx;
public:
    RateLimiter(int N) {
        if (N <= 0) {
            throw invalid_argument("Limit must be greater than 0");
        }
        limit = N;
    }

    bool allowRequest() {

        lock_guard<mutex> lock(mtx);
        

        auto now = steady_clock::now();
        auto one_second_ago = now - seconds(1);


        while (!timestamps.empty() && timestamps.front() <= one_second_ago) {
            timestamps.pop_front();
        }


        if (timestamps.size() < limit) {
            timestamps.push_back(now);
            return true;
        }

        
        return false;
    }
};