#include <iostream>
using namespace std;

string longestPalindrome(string s) {
    int start = 0, maxLen = 1;
    int n = s.length();

    for(int i = 0; i < n; i++) {

        int l = i, r = i;
        while(l >= 0 && r < n && s[l] == s[r]) {
            if(r - l + 1 > maxLen) {
                start = l;
                maxLen = r - l + 1;
            }
            l--;
            r++;
        }

        l = i;
        r = i + 1;
        while(l >= 0 && r < n && s[l] == s[r]) {
            if(r - l + 1 > maxLen) {
                start = l;
                maxLen = r - l + 1;
            }
            l--;
            r++;
        }
    }

    return s.substr(start, maxLen);
}

int main() {

    string s;

    cout << "Enter a string: ";
    cin >> s;

    cout << "Longest Palindromic Substring: " << longestPalindrome(s);

    return 0;
}