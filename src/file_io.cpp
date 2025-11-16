#include "file_io.hpp"
#include <fstream>
using namespace std;



void load_file(GapBuffer &b, const string &path) {
    
    ifstream file(path);
    
    if (!file.is_open()) return;
    
    vector<char> temp;
    char c;
    
    while (file.get(c)) temp.push_back(c);
    
    int gap_size = 1024;
    
    b.data.clear();
    b.data.resize(gap_size + temp.size());
    b.gap_start = 0;
    b.gap_end = gap_size;
    
    for (int i = 0; i < temp.size(); i++)
        b.data[b.gap_end + i] = temp[i];
    
}


void save_file(const GapBuffer &b, const string &path) {
    ofstream file(path);
    if (!file.is_open()) return;
    for (int i = 0; i < b.gap_start; i++)
    file << b.data[i];
    for (int i = b.gap_end; i < b.data.size(); i++)
    file << b.data[i];
}