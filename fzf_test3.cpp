#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <math.h>
#include <filesystem>
#include <future>
#include <sstream>
#include <unordered_set>
#include <set>



namespace fs = std::filesystem;
using namespace std;
typedef vector<vector<float>> Matrix;
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds ms;
typedef std::chrono::duration<float> fsec;
size_t KB = 1024;
size_t MB = 1024 * 1024;
size_t AVG_PATH_LEN = 110;
size_t SIZE_THRESHOLD = 5 * AVG_PATH_LEN;

string unquote_string(string s) {
  size_t l = s.size();
  if (l == 0)
    return s;
  size_t ll = l - 1;
  if (s[0] != s[ll]) {
    if (s.find_first_of("[{(") != 0) {
      return s;
    } else {
      if ((s[0] == '[' && s[ll] == ']') || (s[0] == '{' && s[ll] == '}') ||
          (s[0] == '(' && s[ll] == ')')) {
        return s.substr(1, ll - 1);
      }
      return s;
    }
  }
  if ((s[0] == '"' || s[0] == 39) && (s[ll] == '"' || s[ll] == 39)) {
    return s.substr(1, ll - 1);
  }
  return s;
}

template <typename T>
vector<T> slice(vector<T> lst, size_t start, size_t end = 0) {
  vector<T> res(lst.begin() + start, lst.begin() + end);
  return res;
}

class Pos {
public:
  size_t x, y;

  Pos(size_t x, size_t y) : x(x), y(y) {}

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

  ostream &operator<<(ostream &out) {
    out << this->toStr();
    return out;
  }
};

class PosScore {
public:
  size_t x, y;
  double score;

  PosScore(size_t x, size_t y, double score) : x(x), y(y), score(score) {}
  PosScore(Pos p, double s) : x(p.x), y(p.y), score(s) {}

  string toStr() {
    stringstream ss;
    ss << "(" << this->x << ", " << this->y << ")";
    return ss.str();
  }

  Pos getPos() { return Pos(this->x, this->y); }

  void operator=(const PosScore other) {
    this->x = other.x;
    this->y = other.y;
    this->score = other.score;
  }

  bool operator==(const PosScore other) { return (this->score == other.score); }

  bool operator==(const double other) { return this->score == other; }

  bool operator==(const Pos other) {
    return (this->x == other.x) && (this->y == other.y);
  }

  bool operator<(const PosScore other) { return this->score < other.score; }

  bool operator<(const double other) { return this->score < other; }

  bool operator<=(const PosScore other) { return this->score <= other.score; }

  bool operator<=(const double other) { return this->score <= other; }

  bool operator>(const PosScore other) { return this->score > other.score; }

  bool operator>(const double other) { return this->score > other; }

  bool operator>=(const PosScore other) { return this->score >= other.score; }

  bool operator>=(const double other) { return this->score >= other; }

  ostream &operator<<(ostream &out) {
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

  const bool operator==(AlignmentResult other) {
    return (this->target_start_pos == other.target_start_pos &&
            this->match_string == other.match_string &&
            this->score == other.score);
  }
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

template <typename T> size_t find_in_vec(vector<T> vec, T item) {
  return find(vec.begin(), vec.end(), item);
}

template <typename T> bool contains(vector<T> vec, T item) {
  return find(vec.begin(), vec.end(), item) != vec.end();
}

bool is_lowercase(char c) { return (c >= 'a') && (c <= 'z'); }

bool is_lowercase(string &str) {
  for (char c : str) {
    if (!is_lowercase(c)) {
      return false;
    }
  }
  return true;
}

bool is_uppercase(char c) { return (c >= 'A') && (c <= 'Z'); }

bool is_uppercase(string &str) {
  for (char c : str) {
    if (!is_uppercase(c)) {
      return false;
    }
  }
  return true;
}

bool is_alpha(char c) { return is_lowercase(c) || is_uppercase(c); }

bool is_numer(char c) { return (c >= '0') && (c <= '9'); }

// Set str to be all lowercase, in place.
void lowercase(string &str) {
  char case_offset = 'a' - 'A';
  for (int i = 0; i < str.size(); ++i) {
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
  for (int i = 0; i < str.size(); ++i) {
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

vector<string> splitstr(string s, char delim = '\n') {
  int i = 0;
  int prev_i = 0;
  int l = s.size();
  vector<string> results;
  vector<size_t> indices;
  while (i < l) {
    i = s.find(delim, prev_i);
    if (i == string::npos || i <= prev_i) {
      break;
    }
    results.emplace_back(s.substr(prev_i, i - prev_i + 1));
    indices.emplace_back(i);
    prev_i = i + 1;
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

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
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

vector<fs::path> listdir(fs::path path, bool recursive = false) {
  if (recursive) {
    return _recursive_listdir(path);
  }
  vector<fs::path> dirContents;
  for (const fs::path &entry : fs::directory_iterator(path)) {
    dirContents.push_back(entry);
  }
  return dirContents;
}

vector<fs::path> listdir(string path, bool recursive = false) {
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

size_t directory_size(fs::path path, bool recursive = true) {
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

size_t directory_size(string path, bool recursive = true) {
  return directory_size(fs::path(path), recursive);
}

size_t directory_filenames_size(fs::path path, bool recursive = true) {
  size_t names_size = 0;
  if (recursive) {
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
      names_size += entry.path().string().size();
    }
  } else {
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
  for (int i = 0; i < pos_scores.size(); ++i) {
    if (pos_scores[i].score > max_val) {
      max_val = pos_scores[i].score;
    }
  }
  return max_val;
}

bool _is_top_score(double val, double top_score, double num_scores_counted,
                   size_t num_scores) {
  return (val >= 3.0) &&
         ((val > top_score) || (num_scores_counted < num_scores));
}

// Inserts val into top_scores at the correct position if possible, ensuring
// top_scores remains sorted
void handle_top_score_insertion(PosScore posscore, vector<PosScore> &top_scores,
                                size_t max_num_scores, size_t query_length) {
  double val = posscore.score;
  // if (val < 10.0) {
  if (val < query_length) {
    return;
  }
  if (top_scores.size() < max_num_scores) {
    int i = 0;
    for (; i < top_scores.size(); ++i) {
      if (top_scores[i].score < val) {
        break;
      }
    }
    size_t idx = min(static_cast<size_t>(i), top_scores.size());
    top_scores.emplace(top_scores.begin() + idx, posscore);
    return;
  }
  double min_topscore = min_posscore(top_scores);
  size_t num_scores = top_scores.size();
  if (val < min_topscore) {
    if (num_scores >= max_num_scores) {
      return;
      // for (int i=0; i<num_scores; ++i) {
      //     if (val > top_scores[i].score) {
      //         top_scores[i] = posscore;
      //         return;
      //     }
      // }
    } else {
      top_scores.push_back(posscore);
      // sort(top_scores.begin(), top_scores.end(), greater<>());
    }
  } else {
    // val > min_topscore, it is added no matter what
    if (num_scores >= max_num_scores) {

      for (int i = 0; i < num_scores; i++) {
        if (top_scores[i].score <= val) {
          top_scores[i] = posscore;
          return;
        }
      }
      // made it through list without changing any values, return false
      return;
    } else {
      int i = 0;
      for (; i < top_scores.size(); ++i) {
        if (top_scores[i].score < val) {
          break;
        }
      }
      size_t idx = min(static_cast<size_t>(i), top_scores.size());
      top_scores.emplace(top_scores.begin() + idx, posscore);
      // sort(top_scores.begin(), top_scores.end(), greater<>());
      return;
    }
  }

  return;
}

vector<AlignmentResult> smith_waterman_multi(string query, string target,
                                             char sep = '\n',
                                             size_t num_scores = 5,
                                             double score_threshold = 50.0) {
  // cout << "in smith_waterman, target = " << target << "\n\n\n";
  string target_original_case(target);
  lowercase(query);
  lowercase(target);
  size_t l1 = query.size();
  size_t l2 = target.size();
  Matrix matrix = create_matrix(l1 + 1, l2 + 1);
  // double best_cell_score = 0.0;
  // Pos best_end_pos(0, 0);
  vector<AlignmentResult> results;
  vector<PosScore> top_scores;

  float penalty = 2.0;
  // calculate matrix, keeping track of the best score and its position
  for (int j = 1; j < l1 + 1; j++) {
    for (int i = 1; i < l2 + 1; i++) {
      float score = (query[j - 1] == target[i - 1]) ? 3 : -3;
      float opt1 = matrix[j - 1][i - 1] + score;
      float opt2 = matrix[j - 1][i] - penalty;
      float opt3 = matrix[j][i - 1] - penalty;
      float opt4 = 0.0;
      //this ternary operator is very important.  It ensures that we can never have a separator (newline)
      //inside of a match.  This improves results, and prevents the issue of invalid paths (because of 
      //the path actually being two paths joined by a newline, because the match had a newline in it)
      matrix[j][i] = target[i-1] != sep ? max4(opt1, opt2, opt3, opt4) : 0;
      float cell_value = matrix[j][i];
      // if (cell_value > best_cell_score) {
      PosScore posscore(Pos(i, j), cell_value);
      // if (posscore.score >= 1.0) {
      handle_top_score_insertion(posscore, top_scores, num_scores, l1);
      // }
      // best_cell_score = cell_value;
      // best_end_pos = Pos(i, j);
      // }
    }
  }

  if (top_scores.size() == 0) {
    return results;
  } else if (top_scores[0] <= 0.0) {
    return results;
  }

  cout << "Processing: " << top_scores.size() << " top_scores" << endl;

  for (int i = 0; i < top_scores.size(); ++i) {
    // find position of max value (this is the end position, and then trace it
    // back)
    Pos end_pos = top_scores[i].getPos();
    Pos curr_pos(end_pos.x, end_pos.y);
    size_t curr_y = end_pos.y;
    size_t curr_x = end_pos.x;
    double score = 0.0;
    string aligned1 = "";
    string aligned2 = "";

    while ((matrix[curr_y][curr_x] > 0)) {
      float curr_score = matrix[curr_y][curr_x];
      curr_pos = Pos(curr_x, curr_y);
      score += curr_score;
      float diagonal = matrix[curr_y - 1][curr_x - 1];
      float up = matrix[curr_y - 1][curr_x];
      float left = matrix[curr_y][curr_x - 1];
      float sub_val = query[curr_y - 1] == target[curr_x - 1] ? 3.0 : -3.0;

      if (curr_score == diagonal + sub_val) {
        aligned1 += query[curr_y - 1];
        aligned2 += target[curr_x - 1];
        curr_y--;
        curr_x--;
      } else if (curr_score == up - penalty) {
        aligned1 += query[curr_y - 1];
        aligned2 += "_";
        curr_y--;
      } else if (curr_score == left - penalty) {
        aligned1 += "_";
        aligned2 += target[curr_x - 1];
        curr_x--;
      } else {
        break;
      }
    }

    if (score > score_threshold) {

      size_t target_start_pos = static_cast<size_t>(curr_pos.x);
      size_t match_start_pos = target.rfind(sep, target_start_pos);
      size_t match_end_pos = target.find(sep, end_pos.x);

      // 1. If rfind doesn't find a newline, the path starts at index 0.
      // Otherwise, it starts at match_start_pos + 1 (to skip the newline
      // itself).
      size_t actual_start =
          (match_start_pos == string::npos) ? 0 : match_start_pos + 1;

      // 2. Calculate the length.
      // If find doesn't find a trailing newline, take everything to the end of
      // the string.
      size_t length;
      if (match_end_pos == string::npos) {
        length = string::npos;
      } else {
        length = match_end_pos - actual_start;
      }

      if (length < query.size() / 3) {
        continue;
      }

      string match_str = target_original_case.substr(actual_start, length);
      if (match_str.starts_with('[')) {
        cout << "MATCHED PATH STARTS WITH '['!!!!.  MATCHED_PATH: " << match_str
             << endl;
      }
      reverse(aligned1.begin(), aligned1.end());
      reverse(aligned2.begin(), aligned2.end());
      results.emplace_back(AlignmentResult(
          {aligned1, aligned2, target_start_pos, match_str, score}));
    }
  }
  return results;
}

// Calculates best alignment between query and target, returning a single
// AlignmentResult.
AlignmentResult smith_waterman(string query, string target, char sep = '\n',
                               size_t num_scores = 5) {
  // cout << "in smith_waterman, target = " << target << "\n\n\n";
  string target_original_case(target);
  lowercase(query);
  lowercase(target);
  size_t l1 = query.size();
  size_t l2 = target.size();
  Matrix matrix = create_matrix(l1 + 1, l2 + 1);
  double best_cell_score = 0.0;
  Pos best_end_pos(0, 0);

  float penalty = 2.0;
  // calculate matrix, keeping track of the best score and its position
  for (int j = 1; j < l1 + 1; j++) {
    for (int i = 1; i < l2 + 1; i++) {
      float score = (query[j - 1] == target[i - 1]) ? 3 : -3;
      float opt1 = matrix[j - 1][i - 1] + score;
      float opt2 = matrix[j - 1][i] - penalty;
      float opt3 = matrix[j][i - 1] - penalty;
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

  // find position of max value (this is the end position, and then trace it
  // back)
  Pos end_pos = best_end_pos;
  Pos curr_pos(end_pos.x, end_pos.y);
  size_t curr_y = end_pos.y;
  size_t curr_x = end_pos.x;
  double score = 0.0;
  string aligned1 = "";
  string aligned2 = "";

  while ((matrix[curr_y][curr_x] > 0)) {
    float curr_score = matrix[curr_y][curr_x];
    curr_pos = Pos(curr_x, curr_y);
    score += curr_score;
    float diagonal = matrix[curr_y - 1][curr_x - 1];
    float up = matrix[curr_y - 1][curr_x];
    float left = matrix[curr_y][curr_x - 1];
    float sub_val = query[curr_y - 1] == target[curr_x - 1] ? 3.0 : -3.0;

    if (curr_score == diagonal + sub_val) {
      aligned1 += query[curr_y - 1];
      aligned2 += target[curr_x - 1];
      curr_y--;
      curr_x--;
    } else if (curr_score == up - penalty) {
      aligned1 += query[curr_y - 1];
      aligned2 += "_";
      curr_y--;
    } else if (curr_score == left - penalty) {
      aligned1 += "_";
      aligned2 += target[curr_x - 1];
      curr_x--;
    } else {
      break;
    }
  }

  size_t target_start_pos = static_cast<size_t>(curr_pos.x);
  size_t match_start_pos = target.rfind(sep, target_start_pos);
  size_t match_end_pos = target.find(sep, end_pos.x);

  // 1. If rfind doesn't find a newline, the path starts at index 0.
  // Otherwise, it starts at match_start_pos + 1 (to skip the newline itself).
  size_t actual_start =
      (match_start_pos == string::npos) ? 0 : match_start_pos + 1;

  // 2. Calculate the length.
  // If find doesn't find a trailing newline, take everything to the end of the
  // string.
  size_t length;
  if (match_end_pos == string::npos) {
    length = string::npos;
  } else {
    length = match_end_pos - actual_start;
  }

  if (length < query.size() / 3) {
    return AlignmentResult{"", "", target_start_pos, "", score};
  }

  string match_str = target_original_case.substr(actual_start, length);
  reverse(aligned1.begin(), aligned1.end());
  reverse(aligned2.begin(), aligned2.end());
  return AlignmentResult{aligned1, aligned2, target_start_pos, match_str,
                         score};
}

string dir_to_str(fs::path root_path, bool recurse = false) {
  if (!fs::is_directory(root_path)) {
    root_path = root_path.parent_path();
  }
  string s;
  if (recurse) {
    for (const auto &direntry : fs::recursive_directory_iterator(root_path)) {
      s += unquote_string(direntry.path().string()) + '\n';
    }
  } else {
    for (const auto &direntry : fs::directory_iterator(root_path)) {
      s += unquote_string(direntry.path().string()) + '\n';
    }
  }
  return s;
}

// Calculates num_scores best alignments between query and target, returning a
// vector of AlignmentResults.

// Run smith_waterman on every individual path in 'target', which must be a
// newline separated string of paths Returns vector<AlignmentResult> of all
// results at or above 'score_threshold'
vector<AlignmentResult> smith_waterman_helper_string(string &query,
                                                     string &target,
                                                     double score_threshold) {
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

// Run smith_waterman on every individual path rooted at 'root_path'
// Returns vector<AlignmentResult> of all results at or above 'score_threshold'
vector<AlignmentResult> smith_waterman_helper_path(string query, fs::path root_path, double score_threshold) {
  vector<AlignmentResult> results;
  if (!fs::exists(root_path)) {
      cout << "PROBLEM CAUGHT IN smith_waterman_helper_path!!!!!!!" << endl;
      cout << "GIVEN root_path: " << root_path << " does not exist!!!!" << endl;

  }
  if (!fs::is_directory(root_path)) {
      root_path = root_path.parent_path();
  }
  for (const auto &direntry : fs::recursive_directory_iterator(root_path)) {
    AlignmentResult res = smith_waterman(query, direntry.path().string());
    if (res.score >= score_threshold) {
      res.match_string = unquote_string(res.match_string);
      results.push_back(res);
    }
  }
  return results;
}

size_t find_prev_newline_index(string s, size_t start) {
  return s.substr(0, start).find_last_of('\n');
}

double get_elapsed_time(
    chrono::time_point<chrono::system_clock,
                       chrono::duration<long long, ratio<1LL, 1000000000LL>>>
        start_time) {
  auto end_time = Time::now();
  fsec fs = end_time - start_time;
  ms dur = chrono::duration_cast<ms>(fs);
  double seconds = static_cast<double>(fs.count());
  return seconds;
}

bool candidates_contains(unordered_set<fs::path> &candidates, fs::path &path) {
  if (fs::is_directory(path)) {
    return candidates.contains(path);
  }
  if (path.has_parent_path()) {
    return candidates.contains(path.parent_path());
  }
  return false;
}

void add_known_candidate(unordered_set<fs::path> &candidates, fs::path &path) {
  if (fs::is_directory(path)) {
    candidates.insert(path);
  } else {
    if (path.has_parent_path()) {
      candidates.insert(path.parent_path());
    }
  }
}

size_t _calculate_max_num_scores(size_t buffer_size, size_t sqrt_weight=2, size_t pathlen_weight=1) {
    size_t option1 = sqrt<size_t>(buffer_size);
    size_t option2 = buffer_size / AVG_PATH_LEN;
    return (option1*sqrt_weight + option2*pathlen_weight) / (sqrt_weight + pathlen_weight);
    // return max_num_scores;
}

size_t _calculate_score_threshold(string query) {
    size_t query_length = query.size();
    size_t pipe_idx = query.find('|');
    cout << "pipe_idx: " << pipe_idx << endl;
    if (pipe_idx != string::npos) {
        vector<string> parts = splitstr(query, '|');
        //use average length of parts as query_length (for purposes of calculating score threshold,
        //since this is in the case where there is an '|' in the query, indicating an OR operation
        cout << "before division" << endl;
        cout << "parts.size: " << parts.size() << endl;
        query_length /= parts.size();
        cout << "after division" << endl;
    }
    if (query_length <= 2) return 5;  // Or require exact match
    if (query_length == 3) return 15;
    if (query_length == 4) return 30; // Interpolated
    if (query_length >= 5 && query_length <= 8) return 50 + ((query_length - 5) * 7);
    if (query_length > 8 && query_length <= 14) return 80;
    
    // For anything longer, let it grow slowly or cap it
    return static_cast<size_t>(min<double>(70 + (query_length - 14) * 1.5, 95.0));
}

vector<fs::path> fuzzy_search(string query, fs::path root_path, size_t max_buffer_size = 50 * KB,
                              size_t score_threshold=50, bool no_retry = false) {
  vector<future<vector<AlignmentResult>>> futures;
  string buffer;
  buffer.reserve(max_buffer_size);
  fs::recursive_directory_iterator it(root_path);
  set<fs::path> results;
  vector<fs::path> results_vector;
  unordered_set<fs::path> known_candidates;
  char sep = '\n';
  bool complete = false;
  // size_t score_threshold = _calculate_score_threshold(query.size());
  // size_t N = max_buffer_size / AVG_PATH_LEN;
  // size_t max_num_scores = max_buffer_size / 500;
  // size_t max_num_scores = sqrt<size_t>(max_buffer_size);
  size_t max_num_scores = _calculate_max_num_scores(max_buffer_size);

  auto start_time = Time::now();

  for (decltype(it) end; it != end; it++) {
    fs::path path = it->path();
    string path_str = unquote_string(path.string());
    if (candidates_contains(known_candidates, path)) {
      continue;
    }
    buffer += path_str + sep;
    if (buffer.size() >= max_buffer_size) {
      vector<AlignmentResult> candidates =
          smith_waterman_multi(query, buffer, sep, max_num_scores, score_threshold);
      for (AlignmentResult res : candidates) {
        // AlignmentResult res = smith_waterman(query, buffer);
        //
        if (res.score >= score_threshold) {
          if (!known_candidates.contains(res.match_string)) {
            fs::path matched_path = res.match_string;
            // if (fs::exists(matched_path)) {
            if (!candidates_contains(known_candidates, matched_path)) {
              add_known_candidate(known_candidates, matched_path);
              cout << "Found candidate: " << res.match_string << endl;
              futures.emplace_back(async(smith_waterman_helper_path, query,
                                         matched_path, score_threshold));
              // known_candidates.insert(matched_path);
            }
            // else {
            // }

            // }
          }
        }
      }

      buffer.clear();
      buffer.reserve(max_buffer_size);
    }
  }

  for (future<vector<AlignmentResult>> &fut : futures) {
    vector<AlignmentResult> reslst = fut.get();
    for (AlignmentResult res : reslst) {
      if (res.score >= score_threshold) {
        results.insert(res.match_string);
      }
    }
  }
  auto end_time = Time::now();
  fsec fs = end_time - start_time;
  ms dur = chrono::duration_cast<ms>(fs);
  float seconds = fs.count();
  // if (futures.size() == 0 && seconds < 0.01) {
  //     //If we were
  //     no_retry = true;
  //     vector<AlignmentResult> certain_results = smith_waterman_helper_path(query, root_path, score_threshold);
  //
  // }
  if (results.size() == 0 && seconds < 0.01) {
      cout << "IN ABSOLUTE RESULTS PART" << endl;
      //If we were
      no_retry = true;
      vector<AlignmentResult> certain_results = smith_waterman_helper_path(query, root_path, score_threshold);
      for (const AlignmentResult res : certain_results) {
          results_vector.push_back(res.match_string);
      }
      return results_vector;
  }
  cout << "in fuzzy_search after processing futures. seconds: " << seconds << endl;
  // double seconds = get_elapsed_time(start_time);
  if (!no_retry) {
      

    if (seconds < 0.1) {
        for (int i = 0; i < 6; i++) {
            max_buffer_size /= 2;
            cout << "i: " << i << ", max_buffer_size: " << max_buffer_size << endl;
            // use no_retry=true to avoid entering this if branch in the recursive
            // call, avoiding an infinite loop

            vector<fs::path> new_results = fuzzy_search(query, root_path, max_buffer_size, score_threshold, true);
            for (const fs::path &path : new_results) {
                if (find(results.begin(), results.end(), path) == results.end()) {
                    results.insert(path);
                }
            }
        }
    }
  }

  for (const fs::path &path : results) {
      results_vector.push_back(path);
  }
  return results_vector;
}

int main(int argc, char **argv) {
  fs::path cwd = fs::current_path();
  fs::path p("/Users/sjber/Coding/Python/Fuzzyfind");

  bool recursive = false;

  for (int i = 1; i < argc; i++) {
    if (string(argv[i]).starts_with("recurs")) {
      recursive = true;
      break;
    }
  }
  size_t max_num_threads = 120;
  size_t results_per_run = 50;
  size_t max_memory_budget = 20 * MB;
  string query(argv[1]);
  size_t score_threshold = _calculate_score_threshold(query);
  fs::path root_path = argc >= 3 ? fs::path(string(argv[2])) : fs::current_path();
  
  if (argc > 3) {
    stringstream ss;
    ss << argv[3];
    ss >> max_memory_budget;
  }
  if (argc > 4) {
    stringstream ss;
    ss << argv[4];
    ss >> score_threshold;
  }

  if (argc > 5) {
    stringstream ss;
    ss << argv[5];
    ss >> results_per_run;
  }

  cout << "memory budget: " << max_memory_budget / KB << "KB\n";

  auto start_time = Time::now();


  vector<fs::path> results = fuzzy_search(query, root_path, max_memory_budget, score_threshold);

  double seconds = get_elapsed_time(start_time);
  // for (int i=0; i<4; i++) {
  //     if (results.size() == 0) {
  //         max_memory_budget /= 2;
  //         cout << "No results found, halving max buffer size and trying
  //         again"; results = fuzzy_search(query, root_path, threshold,
  //         max_memory_budget);
  //     }
  //     else{
  //         break;
  //     }
  // }
  for (fs::path &path : results) {
    cout << path << endl;
  }

  cout << "Total of " << results.size() << " found\n";
  cout << "Results for query: " << query << endl;
  cout << "Took " << seconds << " seconds" << endl;
  // size_t num_results = sqrt<size_t>(max_memory_budget);
  size_t num_results = _calculate_max_num_scores(max_memory_budget);
  cout << "Used num_results: " << num_results << endl;
  cout << "Used score_threshold: " << score_threshold << endl;

  // size_t names_size = directory_filenames_size(root_path);
  // cout << "Directory " << root_path.filename() << " has total (recursive)
  // names size: " << names_size << endl;
}
