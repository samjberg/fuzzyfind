#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = filesystem;
size_t AVG_PATH_LEN=110;


bool is_lowercase(char c) {
    return (c >= 'a') && (c <= 'z');
}

bool is_lowercase(string &str) {
    for (char c : str) {
        if (!is_lowercase(c)) {
            return false;
        }
    }
    return true;
}

bool is_uppercase(char c) {
    return (c >= 'A') && (c <= 'Z');
}

bool is_uppercase(string &str) {
    for (char c : str) {
        if (!is_uppercase(c)) {
            return false;
        }
    }
    return true;
}

bool is_alpha(char c) {
    return is_lowercase(c) || is_uppercase(c);
}

bool is_numer(char c) {
    return (c >= '0') && (c <= '9');
}

void lowercase(string &str) {
    char case_offset = 'a' - 'A';
    for (int i=0; i<str.size(); ++i) {
        if (is_uppercase(str[i])) {
            str[i] += case_offset;
        }
    }
}

void uppercase(string &str) {
    char case_offset = 'a' - 'A';
    for (int i=0; i<str.size(); ++i) {
        if (is_lowercase(str[i])) {
            str[i] -= case_offset;
        }
    }
}

size_t _recursive_directory_size(fs::path path) {
    size_t total_size = 0;
    fs::recursive_directory_iterator it(path);
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            total_size += entry.file_size();
        }
    }
    return total_size;
}


size_t directory_size(fs::path path, bool recursive=true) {
    if (recursive) {
        return _recursive_directory_size(path);
    }
    size_t total_size = 0;
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            total_size += entry.file_size();
        }
    }
    return total_size;
}

size_t directory_size(string path, bool recursive=true) {
    return directory_size(fs::path(path), recursive);
}


void recurse_directory(fs::path path, string exclude) {
    fs::recursive_directory_iterator it(path);
    size_t counter = 0;
    for (decltype(it) end; it != end; ++it) {
        if (counter > 10000) {
            break;
        }

        if (it->path().string().find(exclude) != string::npos) {
            continue;
        }
        cout << it->path().string() << endl;
    }

}



int main(int argc, char** argv) {
    ofstream myfile;


    string exclude(argv[2]);
    fs::path path(argv[1]);

    myfile.open("out.txt", ios_base::out);
    fs::recursive_directory_iterator it(path);
    size_t counter = 0;
    for (decltype(it) end; it != end; ++it) {
        if (counter > 10000) {
            break;
        }
        counter++;

        if (it->path().string().find(exclude) != string::npos) {
            continue;
        }
        myfile << it->path().string() << endl;
    }


    
    myfile << "Hello there, this is a test\n";
    return 0;
}
