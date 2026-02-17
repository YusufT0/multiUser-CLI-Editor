#include "file_io.hpp"
#include "buffer_service.hpp"
#include <fstream>
using namespace std;

GapBuffer load_file(const std::string& path) {
    GapBuffer b = BufferService::create_gap_buffer();

    std::ifstream file(path);
    if (!file.is_open()) {
        std::ofstream create(path);   
        return b;                     
    }

    std::vector<char> temp;
    char c;
    while (file.get(c)) temp.push_back(c);

    int gap_size = 1024;
    b.data.clear();
    b.data.resize(gap_size + temp.size());
    b.gap_start = 0;
    b.gap_end = gap_size;

    for (size_t i = 0; i < temp.size(); i++)
        b.data[b.gap_end + i] = temp[i];

    return b;
}


void save_file(const GapBuffer &b, const string &path) {
    ofstream file(path);
    if (!file.is_open()) return;
    for (size_t i = 0; i < b.gap_start; i++)
    file << b.data[i];
    for (size_t i = b.gap_end; i < b.data.size(); i++)
    file << b.data[i];
}