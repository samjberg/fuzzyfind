#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char** argv) {

    size_t npos = string::npos;
    string s = "hello there\nthis is the second line\nthis is the 3rd line";
    char sep = '\n';
    size_t start_pos = s.find("second");

    // if (start_pos == npos) {
    //     cout << "Failed to find sep in s" << endl;
    // }


    // cout << s << endl << endl << endl << endl;
    // cout << s.substr(start_pos) << endl;

    size_t sub_start_pos = s.rfind(sep, start_pos);
    // if (sub_start_pos == npos) {
    //     cout << "Failed to find sep in s" << endl;
    // }
    size_t sub_end_pos = s.find(sep, start_pos);
    string subs = s.substr(sub_start_pos, sub_end_pos);

    cout << subs << endl;


    return 0;
}
