#include <iostream>
#include <string>
#include <vector>
using namespace std;

int lengthOfLongestSubstring(string s);

int main() {
    string str = "abcabcbb";
    cout << lengthOfLongestSubstring(str) << endl;
    return 0;
}

int lengthOfLongestSubstring(string s) {
    vector<int> table(256, -1);
    int currentLen = 0;
    int result = 0;
    int start = 0;
    for (int i = 0; i < s.size(); i++) {
        if (table[s[i]] == -1 || table[s[i]] < start) {
            currentLen++;
        } else {
            start = table[s[i]] + 1;
            currentLen = i - start + 1;
        }
        table[s[i]] = i;
        result = max(result, currentLen);
    }
    return result;
}
