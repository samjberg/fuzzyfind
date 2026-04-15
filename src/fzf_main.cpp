#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <future>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


namespace fs = std::filesystem;
using namespace std;
typedef vector<vector<float>> Matrix;
size_t KB = 1024;
size_t MB = 1024 * 1024;

class Pos {
public:
    size_t x, y;

    Pos(size_t x, size_t y): x(x), y(y) {
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

template <typename T> T vecmax(vector<T> vec) {
    if (vec.size() == 0) {
        T val = 0;
        // cout << "vecmax: " << val << endl;
        return val;
    }
    T maxVal = vec[0];
    for (T val : vec) {
        if (val > maxVal) {
            maxVal = val;
        }
    }
    // cout << "vecmax: " << maxVal << endl; 
    return maxVal;
}

vector<AlignmentResult> smith_waterman(string query, string target, char sep='\n', size_t num_scores=5) {
    // cout << "in smith_waterman, target = " << target << "\n\n\n";
    size_t l1 = query.size();
    size_t l2 = target.size();
    Matrix matrix = create_matrix(l1+1, l2+1);
    vector<double> top_scores;
    vector<Pos> top_scores_positions;

    size_t num_scores_counted = 0;


    Pos zero_pos(0, 0);

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
            float cell_value = matrix[j][i];
            // if (cell_value > 3.0) {
            //     cout << "cell_value: " << cell_value << endl;
            // }
            // for (int idx = 0; idx < ; idx++) {
            if (cell_value > vecmax<double>(top_scores)) {
                top_scores.push_back(cell_value);
                top_scores_positions.push_back(Pos(i, j));
                num_scores_counted++;
            }
                // if (num_scores_counted >= num_scores) {
                //     break;
                //
                // }
            // }
        }
    }
    num_scores = top_scores.size();

    int num_nonzero_scores = 0;
    if (top_scores.size() > 0) {
        // cout << "top scores and positions:" << endl;
        for (int i=0; i<num_scores; i++) {
            // cout << "pos: " << top_scores_positions[i].toStr() << ", score: " << top_scores[i] << "str: " << target.substr(top_scores_positions[i].x, 10) << endl;
            if (top_scores[i] > 0.1) {
                num_nonzero_scores++;
            }
        }
    }
    else {
        vector<AlignmentResult> noresults;
        return noresults;

    }

    // cout << "Found a total of: " << num_nonzero_scores << " nonzero scores" << endl;



    //find position of max value (this is the end position, and then we trace it back to the start)
    //trace end_pos back to the start (1, 1) by following max values while moving only up and/or left (cannot move right or down)
    
    vector<AlignmentResult> results;

    for (int i=0; i<num_scores; i++) {
        Pos end_pos = top_scores_positions[i];
        Pos curr_pos(end_pos.x, end_pos.y);
        vector<Pos> positions = {curr_pos};
        vector<Pos> offsets = {Pos(0, 0)};
        size_t safety_counter = 0;
        size_t curr_y = end_pos.y;
        size_t curr_x = end_pos.x;

        double score = 0.0;


        string aligned1 = "";
        string aligned2 = "";


        while((matrix[curr_y][curr_x] > 0)) {
            // cout << "in while loop" << endl;
            // curr_pos = next_traceback_pos(matrix, curr_x, curr_y);
            //
            float curr_score = matrix[curr_y][curr_x];
            curr_pos = Pos(curr_x, curr_y);
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
        size_t match_end_pos = target.find(sep, end_pos.x);

        // 1. If rfind doesn't find a newline, the path starts at index 0. 
        // Otherwise, it starts at match_start_pos + 1 (to skip the newline itself).
        size_t actual_start = (match_start_pos == string::npos) ? 0 : match_start_pos + 1;

        // 2. Calculate the length. 
        // If find doesn't find a trailing newline, take everything to the end of the string.
        size_t length;
        if (match_end_pos == string::npos) {
            length = string::npos; 
        } else {
            length = match_end_pos - actual_start;
        }

        if (length < query.size()/3) {
            break;
        }

        string match_str = target.substr(actual_start, length);



        reverse(aligned1.begin(), aligned1.end());
        reverse(aligned2.begin(), aligned2.end());
        AlignmentResult res{aligned1, aligned2, target_start_pos, match_str, score};
        results.push_back(res);
        // cout << "match_str: " << match_str << endl;

    }
    // cout << "match_str: " << match_str << ",\tscore: " << score << endl;
    return results;
}






size_t find_prev_newline_index(string s, size_t start) {
    return s.substr(0, start).find_last_of('\n');
}

vector<fs::path> fuzzy_search(string query, fs::path root_path, double score_threshold=35.0, bool files_only=true) {
    vector<fs::path> path_results;
    // vector<thread> threads_list;
    vector<future<vector<AlignmentResult>>> futures_list;
    vector<string> buffer_list;
    vector<Pos> pos_ranges;
    string buffer;
    size_t max_buffer_size = 20 * KB;
    buffer.reserve(max_buffer_size);
    double best_score = 0.0;
    size_t curr_pos = 0;
    size_t curr_start_pos = 0;
    size_t curr_size = 0;
    size_t max_num_threads = 120;
    size_t results_per_run = 50;
    char sep = '\n';
    for (const auto &direntry : fs::recursive_directory_iterator(root_path)) {
        fs::path path = direntry.path();
        string new_path_entry = path.string() + sep;
        buffer += new_path_entry;
        if (!files_only || fs::is_regular_file(path)) {
            if (buffer.size() >= max_buffer_size) {
                if (futures_list.size() < max_num_threads) {
                    futures_list.emplace_back(std::async(smith_waterman, query, std::move(buffer), sep, results_per_run));
                    buffer.clear();
                    buffer.reserve(max_buffer_size);
                }
                else {break;}
            }
        }

    }

    // for (thread &t : threads_list) {
    int i=0;
    for (future<vector<AlignmentResult>> &fut : futures_list) {
        cout << "Getting results from thread: " << i << endl;
        vector<AlignmentResult> results = fut.get();
        // Pos range = pos_ranges[i];
        for (AlignmentResult res : results) {
            if (res.score >= score_threshold) {
                string result_path = res.match_string;
                path_results.push_back(result_path);
                best_score = res.score;
            }
        }
        i++;
    }
    cout << "Best score: " << best_score << endl;
    return path_results;
}



#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;


PYBIND11_MODULE(fuzzyfind_functions, m) {
    pybind11::class_<AlignmentResult>(m, "AlignmentResult")
        .def_readonly("query_aligned", &AlignmentResult::query_aligned)
        .def_readonly("target_aligned", &AlignmentResult::target_aligned)
        .def_readonly("target_start_pos", &AlignmentResult::target_start_pos)
        .def_readonly("score", &AlignmentResult::score);

    m.def("smith_waterman", [](string s1, string s2) {
            vector<AlignmentResult> res = smith_waterman(s1, s2);
            return res;
    });

    m.def("fuzzy_search", [](string query, string target, double score_threshold = 35.0, bool files_only = true) {
            vector<fs::path> path_list = fuzzy_search(query, target, score_threshold, files_only);
            vector<string> path_str_list;
            for (const auto &path : path_list) {
                path_str_list.push_back(path.string());
            }
            return path_str_list;
    });


#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
