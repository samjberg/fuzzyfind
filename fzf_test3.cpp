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
size_t AVG_PATH_LEN = 110;
size_t SIZE_THRESHOLD = 5 * AVG_PATH_LEN;

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

class PosScore {
public:
    size_t x, y;
    double score;

    PosScore(size_t x, size_t y, double score): x(x), y(y), score(score){}
    PosScore(Pos p, double s): x(p.x), y(p.y), score(s){}

    string toStr() {
        stringstream ss;
        ss << "(" << this->x << ", " << this->y << ")";
        return ss.str();
    }

    void operator=(const PosScore other) {
        this->x = other.x;
        this->y = other.y;
        this->score = other.score;
    }

    void operator=(const double other) {
        this->score = other;
    }

    void operator=(const Pos other) {
        this->x = other.x;
        this->y = other.y;

    }


    bool operator==(const PosScore other) {
        return (this->score == other.score);
    }

    bool operator==(const double other) {
        return this->score == other;
    }

    bool operator==(const Pos other) {
        return (this->x == other.x) && (this->y == other.y);
    }

    bool operator<(const PosScore other) {
        return this->score < other.score;
    }

    bool operator<(const double other) {
        return this->score < other;
    }

    bool operator<=(const PosScore other) {
        return this->score <= other.score;
    }

    bool operator<=(const double other) {
        return this->score <= other;
    }

    bool operator>(const PosScore other) {
        return this->score > other.score;
    }

    bool operator>(const double other) {
        return this->score > other;
    }


    bool operator>=(const PosScore other) {
        return this->score >= other.score;
    }

    bool operator>=(const double other) {
        return this->score >= other;
    }

    ostream& operator<<(ostream &out) {
        out << this->toStr();
        return out;
    }

};

vector<Pos> get_positions(vector<PosScore> lst) {
    vector<Pos> positions;
    for (const PosScore &ps : lst) {
        positions.emplace_back(Pos(ps.x, ps.y));
    }
    return positions;
}

vector<double> get_scores(vector<PosScore> lst) {
    vector<double> scores;
    for (const PosScore &ps : lst) {
        scores.push_back(ps.score);
    }
    return scores;
}

struct AlignmentResult {
    string query_aligned;
    string target_aligned;
    size_t target_start_pos;
    string match_string;
    double score;
};

struct PosScores {
    vector<Pos> positions;
    vector<double> scores;
};

// struct PosScore {
//     Pos pos;
//     double score;
// };
//
bool cmp_posscore_gte(PosScore ps1, PosScore ps2) {
    return ps1.score >= ps2.score;
}


bool cmp_posscore_gt(PosScore ps1, PosScore ps2) {
    return ps1.score > ps2.score;
}


bool cmp_posscore_lt(PosScore ps1, PosScore ps2) {
    return ps1.score < ps2.score;
}

bool cmp_posscore_lte(PosScore ps1, PosScore ps2) {
    return ps1.score <= ps2.score;
}

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

//Set str to be all lowercase, in place.
void lowercase(string &str) {
    char case_offset = 'a' - 'A';
    for (int i=0; i<str.size(); ++i) {
        if (is_uppercase(str[i])) {
            str[i] += case_offset;
        }
    }
}

string to_lowercase(string &str) {
    string lower_str(str);
    lowercase(lower_str);
    return lower_str;
}

void uppercase(string &str) {
    char case_offset = 'a' - 'A';
    for (int i=0; i<str.size(); ++i) {
        if (is_lowercase(str[i])) {
            str[i] -= case_offset;
        }
    }
}


string to_uppercase(string &str) {
    string upper_str(str);
    lowercase(upper_str);
    return upper_str;
}

vector<string> splitstr(string s, char delim='\n') {
    int i = 0;
    int prev_i = 0;
    int l = s.size();
    vector<string> results;
    vector<size_t> indices;
    while (i < l) {
        i = s.find(delim, prev_i);
        if (i == string::npos || i == prev_i) {
            break;
        }
        results.emplace_back(s.substr(prev_i, i-prev_i+1));
        indices.emplace_back(i);
        prev_i = i+1;
    }
    return results;
}




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
template <typename T> T vecmin(vector<T> vec) {
    if (vec.size() == 0) {
        T val = 0;
        // cout << "vecmax: " << val << endl;
        return val;
    }
    T maxVal = vec[0];
    for (T val : vec) {
        if (val < maxVal) {
            maxVal = val;
        }
    }
    // cout << "vecmax: " << maxVal << endl;
    return maxVal;
}

double min_posscore(vector<PosScore> pos_scores) {
    double max_val = pos_scores[0].score;
    for (int i=0; i<pos_scores.size(); ++i) {
        if (pos_scores[i].score > max_val) {
            max_val = pos_scores[i].score;
        }
    }
    return max_val;

}

bool _is_top_score(double val, double top_score, double num_scores_counted, size_t num_scores) {
    return (val >= 3.0) && ((val > top_score) || (num_scores_counted < num_scores));
}


//Inserts val into top_scores at the correct position if possible, ensuring top_scores remains sorted
void handle_top_score_insertion(PosScore posscore, vector<PosScore> &top_scores, size_t max_num_scores) {
    double val = posscore.score;
    if (top_scores.size() < max_num_scores) {
        int i=0;
        for (; i<top_scores.size(); ++i) {
            if (top_scores[i] < val) {
                break;
            }
        }
        size_t idx = min(static_cast<size_t>(i+1), top_scores.size());
        top_scores.emplace(top_scores.begin() + idx, posscore);
        return;

    }
    double min_topscore = min_posscore(top_scores);
    size_t num_scores = top_scores.size();
    if (val < min_topscore) {
        if (num_scores >= max_num_scores) {
            for (int i=0; i<num_scores; ++i) {
                if (val > top_scores[i].score) {
                    top_scores[i] = posscore;
                    return;
                }
            }
        }
        else {
            top_scores.push_back(posscore);
            sort(top_scores.begin(), top_scores.end(), greater<>());
        }
    }
    else {
        //val > min_topscore, it is added no matter what
        if (num_scores >= max_num_scores) {

            for (int i=0; i<num_scores; i++) {
                if (top_scores[i] <= val) {
                    top_scores[i] = val;
                    return; 
                }
            }
            //made it through list without changing any values, return false
            return;
        }
        else {
            top_scores.push_back(posscore);
            sort(top_scores.begin(), top_scores.end(), greater<>());
            return;
        }
    }

    return;
}




//Calculates best alignment between query and target, returning a single AlignmentResult.
AlignmentResult smith_waterman(string query, string target, char sep='\n', size_t num_scores=5) {
    // cout << "in smith_waterman, target = " << target << "\n\n\n";
    string target_original_case(target);
    lowercase(query);
    lowercase(target);
    size_t l1 = query.size();
    size_t l2 = target.size();
    Matrix matrix = create_matrix(l1+1, l2+1);
    double best_cell_score = 0.0;
    Pos best_end_pos(0, 0);

    float penalty = 2.0;
    //calculate matrix, keeping track of the best score and its position
    for (int j=1; j<l1+1; j++) {
        for (int i=1; i<l2+1; i++) {
            float score = (query[j-1] == target[i-1]) ? 3 : -3;
            float opt1 = matrix[j-1][i-1] + score;
            float opt2 = matrix[j-1][i] - penalty;
            float opt3 = matrix[j][i-1] - penalty;
            float opt4 = 0.0;
            matrix[j][i] = max4(opt1, opt2, opt3, opt4);
            float cell_value = matrix[j][i];
            if (cell_value > best_cell_score) {
                best_cell_score = cell_value;
                best_end_pos = Pos(i, j);
            }
        }
    }

    if (best_cell_score <= 0.0) {
        return AlignmentResult{"", "", 0, "", 0.0};
    }

    //find position of max value (this is the end position, and then trace it back)
    Pos end_pos = best_end_pos;
    Pos curr_pos(end_pos.x, end_pos.y);
    size_t curr_y = end_pos.y;
    size_t curr_x = end_pos.x;
    double score = 0.0;
    string aligned1 = "";
    string aligned2 = "";

    while((matrix[curr_y][curr_x] > 0)) {
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
        return AlignmentResult{"", "", target_start_pos, "", score};
    }

    string match_str = target_original_case.substr(actual_start, length);
    reverse(aligned1.begin(), aligned1.end());
    reverse(aligned2.begin(), aligned2.end());
    return AlignmentResult{aligned1, aligned2, target_start_pos, match_str, score};
}

string unquote_string(string s) {
    size_t l = s.size();
    if (l==0) return s;
    size_t ll = l - 1;
    if (s[0] != s[ll]) return s;
    if ((s[0]=='"' || s[0]==39) && (s[ll]=='"' || s[ll]==39)) { return s.substr(1, ll-1);}
    return s;
}



string dir_to_str(fs::path root_path, bool recurse=false) {
    if (!fs::is_directory(root_path)) {
        root_path = root_path.parent_path();
    }
    string s;
    if (recurse) {
        for (const auto &direntry : fs::recursive_directory_iterator(root_path)) {
            s += unquote_string(direntry.path().string()) + '\n';
        }
    }
    else {
        for (const auto &direntry : fs::directory_iterator(root_path)) {
            s += unquote_string(direntry.path().string()) + '\n';
        }
    }
    return s;

}

//Calculates num_scores best alignments between query and target, returning a vector of AlignmentResults.


//Run smith_waterman on every individual path in 'target', which must be a newline separated string of paths
//Returns vector<AlignmentResult> of all results at or above 'score_threshold'
vector<AlignmentResult> smith_waterman_helper_string(string &query, string &target, double score_threshold) {
    string q = to_lowercase(query);
    string t = to_lowercase(target);
    vector<string> lines = splitstr(t);
    vector<AlignmentResult> results;
    for (string line : lines) {
        AlignmentResult res = smith_waterman(query, line);
        if (res.score >= score_threshold) {
            results.push_back(res);
        }
    }
    return results;
}

//Run smith_waterman on every individual path rooted at 'root_path'
//Returns vector<AlignmentResult> of all results at or above 'score_threshold'
vector<AlignmentResult> smith_waterman_helper_path(string query, fs::path root_path, double score_threshold) {
    vector<AlignmentResult> results;
    if (!fs::is_directory(root_path)) {
        cout << "NOT A DIRECTORY: " << root_path;
        root_path = root_path.parent_path();
        cout << "USING PARENT PATH: " << root_path;

    }
    for (const auto & direntry : fs::recursive_directory_iterator(root_path)) {
        AlignmentResult res = smith_waterman(query, direntry.path().string());
        if (res.score >= score_threshold) {
            res.match_string = unquote_string(res.match_string);
            results.push_back(res);
        }
    }
    return results;
}



//Recursive version of smith_waterman.  It finds a list (vector) of matches, and then saves the result as the
//context around those matches, and then recursively calls itself (smith_waterman_new) on the matches
//in the found context, until the found context has a short enough length, at which point all matches are
//found manually
vector<AlignmentResult> smith_waterman_new(string query, string target, char sep='\n', size_t num_scores=5, double score_threshold=30.0) {
    string target_original_case(target);
    lowercase(query);
    lowercase(target);
    size_t l1 = query.size();
    size_t l2 = target.size();
    if (l2 < SIZE_THRESHOLD) {
        return smith_waterman_helper_string(query, target, score_threshold);
    }
    Matrix matrix = create_matrix(l1+1, l2+1);
    double top_score = 0.0;

    // vector<PosScore> top_posscores;

    size_t num_scores_counted = 0;

    // vector<double> top_scores;
    vector<PosScore> top_scores;

    Pos zero_pos(0, 0);

    float penalty = 2.0;
    //calculate matrix, keeping track of [num_scores] top scores and their positions
    for (int j=1; j<l1+1; j++) {
        for (int i=1; i<l2+1; i++) {
            float score = (query[j-1] == target[i-1]) ? 3 : -3;
            float opt1 = matrix[j-1][i-1] + score;
            float opt2 = matrix[j-1][i] - penalty;
            float opt3 = matrix[j][i-1] - penalty;
            float opt4 = 0.0;
            matrix[j][i] = max4(opt1, opt2, opt3, opt4);
            float cell_value = matrix[j][i];
            PosScore pos_score = {Pos(i, j), cell_value};
            // if (cell_value > 3 && ((cell_value > top_score) || (num_scores_counted < num_scores))) {
            handle_top_score_insertion(pos_score, top_scores, num_scores);
            
        }
    }

    vector<Pos> top_scores_positions = get_positions(top_scores);
    // vector<double> top_scores = top_posscores.scores;
    // the sorting is done automatically as top_scores is created now
    vector<double> top_scores_sorted = get_scores(top_scores);
    // copy(top_scores.begin(), top_scores.end(), top_scores_sorted.begin());
    // sort(top_scores_sorted.begin(), top_scores_sorted.end());
    num_scores = top_scores.size();
    // int cutoff_idx =

    int num_nonzero_scores = 0;
    if (top_scores.size() > 0) {
        double min_nonzero_topscore = top_scores[0].score;
        for (int i=0; i<num_scores; i++) {
            if (top_scores[i] > 0.1) {
                min_nonzero_topscore = top_scores[i].score;
                num_nonzero_scores++;
            }
        }
    }
    else {
        vector<AlignmentResult> noresults;
        return noresults;

    }

    //find position of max value (this is the end position, and then we trace it back to the start)
    //trace end_pos back to the start (1, 1) by following max values while moving only up and/or left (cannot move right or down)

    vector<AlignmentResult> results;

    for (int i=0; i<num_scores; i++) {
        // cout << i << endl;
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

        string match_str = target_original_case.substr(actual_start, length);



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

vector<fs::path> fuzzy_search(string query, fs::path root_path, double score_threshold=35.0, size_t max_buffer_size=50*KB) {
    vector<future<vector<AlignmentResult>>> futures;
    string buffer;
    buffer.reserve(max_buffer_size);
    fs::recursive_directory_iterator it(root_path);
    vector<fs::path> results;
    char sep = '\n';
    bool complete = false;
    for (decltype(it) end; it != end; it++) {
        fs::path path = it->path();
        string path_str = unquote_string(path.string());
        buffer += path_str + sep;
        if (buffer.size() >= max_buffer_size) {
            AlignmentResult res = smith_waterman(query, buffer);
            if (res.score >= score_threshold) {
                cout << "Found candidate: " << res.match_string << endl;
                fs::path matched_path = res.match_string;
                string matched_buffer = dir_to_str(matched_path, true);
                futures.emplace_back(async(smith_waterman_helper_path, query, matched_path, score_threshold));
            }

            buffer.clear();
            buffer.reserve(max_buffer_size);

        }

    }

    for (future<vector<AlignmentResult>> &fut : futures) {
        vector<AlignmentResult> reslst = fut.get();
        for (AlignmentResult res : reslst) {
            if (res.score >= score_threshold) {
                results.push_back(fs::path(res.match_string));
            }
        }
    }
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
    size_t max_num_threads=120;
    size_t results_per_run=50;
    size_t max_memory_budget = 20 * MB;
    string query(argv[1]);
    fs::path root_path = argc >= 3 ? fs::path(string(argv[2])) : fs::current_path();
    double threshold = 35.0;
    if (argc > 3) {
        stringstream ss;
        ss << argv[3];
        ss >> threshold;
    }
    if (argc > 4) {
        stringstream ss;
        ss << argv[4];
        ss >> max_memory_budget;
    }

    if (argc > 5) {
        stringstream ss;
        ss << argv[5];
        ss >> results_per_run;
    }

    cout << "memory budget: " << max_memory_budget/KB << "KB\n";


    vector<fs::path> results = fuzzy_search(query, root_path, threshold, max_memory_budget);
    for (int i=0; i<4; i++) {
        if (results.size() == 0) {
            max_memory_budget /= 2;
            cout << "No results found, halving max buffer size and trying again";
            results = fuzzy_search(query, root_path, threshold, max_memory_budget);
        }
        else{
            break;
        }
    }
    for (fs::path &path : results) {
        cout << path << endl;
    }

    cout << "Total of " << results.size() << " found\n";
    cout << "Results for query: " << query << endl;
    cout << "Used results_per_run: " << results_per_run << ", max_num_threads: " << max_num_threads << ", threshold: " << threshold << endl;

    // size_t names_size = directory_filenames_size(root_path);
    // cout << "Directory " << root_path.filename() << " has total (recursive) names size: " << names_size << endl;


}
