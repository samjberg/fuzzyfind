#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <future>

namespace fs = std::filesystem;
using namespace std;
typedef vector<vector<float>> Matrix;
size_t KB = 1024;
size_t MB = 1024 * 1024;

class Pos {
public:
    int x, y;

    Pos(int x, int y): x(x), y(y) {
    }

    string toStr() {
        stringstream ss;
        ss << "(" << this->x << ", " << this->y << ")";
        return ss.str();
    }

    const char* toCStr() {
        string s = this->toStr();
        const char* cstr = s.c_str();
        return cstr;
    }

    Pos operator+(const Pos other) {
        return Pos(this->x + other.x, this->y + other.y);
    }

    Pos operator-(const Pos other) {
        return Pos(this->x - other.x, this->y - other.y);
    }

    bool operator==(const Pos other) {
        return ((this->x == other.x) && (this->y == other.y));
    }

    bool operator<(const Pos other) {
        return (this->x < other.x) && (this->y < other.y);
    }

    bool operator<=(const Pos other) {
        return (this->x <= other.x) && (this->y <= other.y);
    }

    bool operator>(const Pos other) {
        return (this->x > other.x) && (this->y > other.y);
    }

    bool operator>=(const Pos other) {
        return (this->x >= other.x) && (this->y >= other.y);
    }


    ostream& operator<<(ostream &out) {
        out << this->toStr();
        return out;
    }

};

struct AlignmentResult {
    string query_aligned;
    string target_aligned;
    size_t target_start_pos;
    string match_string;
    double score;
};

float max4(float x1, float x2, float x3, float x4) {
    float maxVal = x1;
    if (x2 > maxVal) {
        maxVal = x2;
    }
    if (x3 > maxVal) {
        maxVal = x3;
    }
    if (x4 > maxVal) {
        return x4;
    }
    return maxVal;
}


Matrix create_matrix(size_t height, size_t width) {
    Matrix matrix(height, vector<float>(width, 0));
    return matrix;
}


Pos matrix_max_pos(vector<vector<float>> matrix) {
    float max = matrix[0][0];
    Pos max_pos(0, 0);
    size_t height = matrix.size();
    size_t width = matrix[0].size();

    for (int j=0; j<height; ++j) {
        for (int i=0; i<width; ++i) {
            if (matrix[j][i] > max) {
                max = matrix[j][i];
                max_pos = Pos(i, j);
            }

        }
    }
    return max_pos;
}


vector<fs::path> _recursive_listdir(fs::path path) {
    vector<fs::path> dirContents;
    for (const fs::path &entry : fs::recursive_directory_iterator(path)) {
        dirContents.push_back(entry);
    }
    return dirContents;
}

vector<fs::path> listdir(fs::path path, bool recursive=false)  {
    if (recursive) {
        return _recursive_listdir(path);
    }
    vector<fs::path> dirContents;
    for (const fs::path &entry : fs::directory_iterator(path)) {
        dirContents.push_back(entry);
    }
    return dirContents;
}

vector<fs::path> listdir(string path, bool recursive=false)  {
    fs::path fspath(path);
    return listdir(fspath, recursive);
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


size_t directory_filenames_size(fs::path path, bool recursive=true) {
    size_t names_size = 0;
    if (recursive) {
        for (const auto &entry : fs::recursive_directory_iterator(path)) {
            names_size += entry.path().string().size();
        }
    }
    else {
        for (const auto &entry : fs::directory_iterator(path)) {
            names_size += entry.path().string().size();
        }
    }
    return names_size;
}

size_t directory_filenames_size(string path) {
    return directory_size(fs::path(path));
}


AlignmentResult smith_waterman(string query, string target, char sep='\n') {
    size_t l1 = query.size();
    size_t l2 = target.size();
    Matrix matrix = create_matrix(l1+1, l2+1);


    float penalty = 2.0;
    //calculate matrix
    for (int j=1; j<l1+1; j++) {
        for (int i=1; i<l2+1; i++) {
            float score = (query[j-1] == target[i-1]) ? 3 : -3;
            float opt1 = matrix[j-1][i-1] + score;
            float opt2 = matrix[j-1][i] - penalty;
            float opt3 = matrix[j][i-1] - penalty;
            float opt4 = 0.0;
            matrix[j][i] = max4(opt1, opt2, opt3, opt4);

        }
    }

    //find position of max value (this is the end position, and then we trace it back to the start)
    Pos end_pos = matrix_max_pos(matrix);
    //trace end_pos back to the start (1, 1) by following max values while moving only up and/or left (cannot move right or down)
    Pos curr_pos(end_pos.x, end_pos.y);
    Pos prev_pos(curr_pos.x, curr_pos.y);
    vector<Pos> positions = {curr_pos};
    vector<Pos> offsets = {Pos(0, 0)};
    int safety_counter = 0;
    int curr_y = end_pos.y;
    int curr_x = end_pos.x;


    string aligned1 = "";
    string aligned2 = "";


    Pos min_pos(1, 1);
    double score = 0.0;

    while((matrix[curr_y][curr_x] > 0)) {
        // curr_pos = next_traceback_pos(matrix, curr_x, curr_y);
        curr_pos = Pos(curr_x, curr_y);
        float curr_score = matrix[curr_y][curr_x];
        score += curr_score;
        float diagonal = matrix[curr_y-1][curr_x-1];
        float up = matrix[curr_y-1][curr_x];
        float left = matrix[curr_y][curr_x-1];
        float sub_val = query[curr_y-1] == target[curr_x-1] ? 3.0 : -3.0;

        if (curr_score == diagonal + sub_val) {
            aligned1 += query[curr_y - 1];
            aligned2 += target[curr_x - 1];
            curr_y--; curr_x--;
        }
        else if (curr_score == up - penalty) {
            aligned1 += query[curr_y - 1];
            aligned2 += "_";
            curr_y--;
        }
        else if (curr_score == left - penalty) {
            aligned1 += "_";
            aligned2 += target[curr_x - 1];
            curr_x--;
        }
        else {
            break;
            aligned1 += "_";
            aligned2 += "_";
            curr_y--; curr_x--;

        }
    }


    size_t target_start_pos = static_cast<size_t>(curr_pos.x);
    size_t match_start_pos = target.rfind(sep, target_start_pos);
    size_t match_end_pos = target.find(sep, target_start_pos);
    string match_str = (match_start_pos == string::npos || match_end_pos == string::npos) ? "" : target.substr(match_start_pos, match_end_pos);


    reverse(aligned1.begin(), aligned1.end());
    reverse(aligned2.begin(), aligned2.end());
    AlignmentResult res{aligned1, aligned2, target_start_pos, match_str, score};
    return res;
}




vector<fs::path> fuzzy_search_orig(string query, fs::path root_path, double score_threshold=35.0, bool files_only=true) {
    vector<fs::path> results;
    wchar_t sep = fs::path::preferred_separator;
    for (const fs::path &path : fs::recursive_directory_iterator(root_path)) {
        if (!files_only || fs::is_regular_file(path)) {
            fs::path rel_path = fs::relative(path, root_path);
            string fname = path.filename().string();
            AlignmentResult res = smith_waterman(query, fname);
            if (res.score >= score_threshold) {
                results.push_back(rel_path);
            }

        }
    }
    return results;
}

size_t find_prev_newline_index(string s, size_t start) {
    return s.substr(0, start).find_last_of('\n');
}

vector<fs::path> fuzzy_search(string query, fs::path root_path, double score_threshold=35.0, bool files_only=true) {
    vector<fs::path> results;
    // vector<thread> threads_list;
    vector<future<AlignmentResult>> futures_list;
    vector<string> buffer_list;
    vector<Pos> pos_ranges;
    string buffer;
    size_t max_buffer_size = 10000;//10 * MB;
    buffer.reserve(max_buffer_size);
    double best_score = 0.0;
    size_t curr_pos = 0;
    size_t curr_start_pos = 0;
    size_t curr_size = 0;
    char sep = '\n';
    for (const auto direntry : fs::recursive_directory_iterator(root_path)) {
        fs::path path = direntry.path();
        string new_path_entry = path.string() + sep;
        buffer += new_path_entry;
        curr_pos = buffer.size();
        if (!files_only || fs::is_regular_file(path)) {
            size_t curr_buffer_size = curr_pos - curr_start_pos;
            if (curr_buffer_size >= max_buffer_size) {
                string sub_buffer = buffer.substr(curr_start_pos, curr_pos);
                if (futures_list.size() < 15) {
                    futures_list.push_back(std::async(smith_waterman, query, sub_buffer, sep));
                    pos_ranges.push_back(Pos(curr_start_pos, curr_pos));
                    buffer_list.push_back(sub_buffer);
                    curr_start_pos = curr_pos;

                }
            }
        }

    }

    // for (thread &t : threads_list) {
    int i=0;
    for (future<AlignmentResult> &fut : futures_list) {
        
        fut.wait();
        AlignmentResult res = fut.get();
        Pos range = pos_ranges[i];

        if (res.score >= score_threshold) {
            string sub_buffer = buffer_list[i];
            // size_t start_pos = sub_buffer.find_last_of(sep, res.target_start_pos);
            // size_t end_pos = sub_buffer.find_first_of(sep, res.target_start_pos);
            // size_t start_pos = sub_buffer.rfind(sep);
            // size_t end_pos = sub_buffer.find(sep);
            // if (start_pos == string::npos) {start_pos = 0;}
            // if (end_pos == string::npos) {end_pos = sub_buffer.size();}
            // string result_path = sub_buffer.substr(start_pos, end_pos);
            string result_path = res.match_string;
            results.push_back(result_path);
            // buffer = "";
            best_score = res.score;
        }
        i++;
    }

    // if (buffer.size() > query.size()) {
    //     AlignmentResult res = smith_waterman(query, buffer);
    //     // buffer_list.push_back(buffer);
    //     // size_t pos = buffer.substr(0, res.target_start_pos).find_last_of('\n');
    //     size_t start_pos = results.size()==0 ? 0 : buffer.find_last_of(sep, res.target_start_pos);
    //     size_t end_pos = buffer.find_first_of(sep, res.target_start_pos);
    //     string result_path = buffer.substr(start_pos, end_pos);
    //     results.push_back(result_path);
    // }

    cout << "Best score: " << best_score << endl;

    return results;
}


int main(int argc, char** argv) {
    fs::path cwd = fs::current_path();
    fs::path p("/Users/sjber/Coding/Python/Fuzzyfind");

    bool recursive = false;

    for (int i=1; i<argc; i++) {
        if (string(argv[i]).starts_with("recurs")) {
            recursive = true;
            break;
        }
    }

    string query(argv[1]);
    fs::path root_path = argc >= 3 ? fs::path(string(argv[2])) : fs::current_path();
    double threshold = 35.0;
    if (argc > 3) {
        stringstream ss;
        ss << argv[3];
        ss >> threshold;
    }
    vector<fs::path> results = fuzzy_search(query, root_path, threshold);
    for (fs::path &path : results) {
        cout << path << endl;
    }

    cout << "Total of " << results.size() << " found\n";
    cout << "Results for query: " << query << endl;

    // size_t names_size = directory_filenames_size(root_path);
    // cout << "Directory " << root_path.filename() << " has total (recursive) names size: " << names_size << endl;


}
