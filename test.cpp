#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <sstream>

using namespace std;
namespace fs = filesystem;

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

struct AlignmentResult {
    string query_aligned;
    string target_aligned;
    size_t target_start_pos;
    string match_string;
    double score;
    const bool operator==(const AlignmentResult &other) {
        return match_string == other.match_string;
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


void handle_top_score_insertion(PosScore posscore, vector<PosScore> &top_scores, size_t max_num_scores) {
    double val = posscore.score;
    if (top_scores.size() < max_num_scores) {
        int i=0;
        for (; i<top_scores.size(); ++i) {
            if (top_scores[i] < val) {
                break;
            }
        }
        top_scores.emplace(top_scores.begin() + i + 1, posscore);
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


string vd2s(vector<double> lst) {
    ostringstream ss;
    ss << "{";
    // for (double d : lst) {
    for (int i=0; i<lst.size()-1; ++i) {
        ss << lst[i] << ", ";
    }
    ss << lst.back() << "}";
    return ss.str();
}


bool cmpscoregte(PosScore ps1, PosScore ps2) {
    return ps1.score >= ps2.score;
}

bool cmp_posscore(PosScore ps1, PosScore ps2) {
    return ps1.score >= ps2.score;
}

string unquote_string(string s) {
    size_t l = s.size();
    if (l==0) return s;
    size_t ll = l - 1;
    if (s[0] != s[ll]) {
        if (s.find_first_of("[{(") != 0) {
            return s;
        }
        else {
            if ((s[0] == '[' && s[ll]==']') || (s[0] == '{' && s[ll] == '}') || (s[0] == '(' && s[ll] == ')')) {
                return s.substr(1, ll-1);
            }
            return s;
        }
    }
    if ((s[0]=='"' || s[0]==39) && (s[ll]=='"' || s[ll]==39)) { return s.substr(1, ll-1);}
    return s;
}


string dir_to_str(fs::path root_path, bool recurse=false) {
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


fs::path get_good_parent(fs::path path) {
    if (path == fs::path("/")) return path;
    path = path.parent_path();
    if (path == fs::path("/")) return path;
    string s;
    fs::path fs_root("/");
    while ((path != fs_root) && (path.string().find_first_of("._") == 0)) {
        path = path.parent_path();
    }
    return path;
}

template <typename T>
vector<T> slice(vector<T> lst, size_t start, size_t end=0) {
    vector<T> res(lst.begin() + start, lst.begin() + end);
    return res;
}


int main(int argc, char** argv) {

    vector<double> lst3 = {5.0, 4.3, 4.0, 3.5, 3.2, 1.6};
    vector<double> lst3_mid = slice<double>(lst3, 1, 4);
    for (double d : lst3_mid) {
        cout << d << endl;
    }
    return 0;
    double val = 3.6;
    int i = 0;
    for (; i<lst3.size(); ++i) {
        cout << "i: " << i << endl;
        if (lst3[i] < val) {
            break;
        }
    }
    cout << endl;
    size_t idx = min(static_cast<size_t>(i), lst3.size());
    cout << "idx: " << idx << endl;
    lst3.emplace(lst3.begin() + idx, val);
    for (double d : lst3) {
        cout << d << endl;
    }
    return 0;


    string s = argv[1];
    cout << s << endl;
    cout << unquote_string(s) << endl;
    return 0;
    cout << dir_to_str(s) << endl;
    return 0;
    PosScore p1 = PosScore(2, 3, 3);
    PosScore p2 = PosScore(0, 1, 2);
    PosScore p3 = PosScore(0, 2, 8);
    vector<PosScore> plst = {p1, p2, p3};
    sort(plst.begin(), plst.end(), cmpscoregte);
    for (int i=0; i<plst.size(); i++) {
        cout << plst[i].score << endl;
    }
    return 0;

    vector<int> lst = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    lst.emplace(lst.begin() + 4, 23);
    for (int i=0; i<lst.size(); i++) {
        cout << lst[i] << endl;
    }
    return 0;

    vector<double> scores = {5.0, 2.0, 0.2, 6.4, -0.3, 0.6, 9.3, -0.2, 5.6};
    vector<PosScore> posscores;
    for (int i=0; i<scores.size(); i++) {
        posscores.emplace_back(PosScore(Pos(0, 0), scores[i]));
    }
    sort(scores.begin(), scores.end(), greater<>());
    cout << vd2s(scores) << endl;
    handle_top_score_insertion(PosScore(Pos(0,0), 6.2), posscores, 10);
    cout << vd2s(scores) << endl;
    handle_top_score_insertion(PosScore(0, 0, 3.2), posscores, 10);
    cout << vd2s(scores) << endl;
    handle_top_score_insertion(PosScore(0, 0, 8.2), posscores, 10);
    cout << vd2s(scores) << endl;

    sort(scores.begin(), scores.end(), greater<double>());




    return 0;
}
