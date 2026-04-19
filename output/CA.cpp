#include <iostream>
using namespace std;

string longestPal(string s) {

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

    cout << "Enter string: ";
    cin >> s;

    cout << "Longest Palindrome substring: " << longestPal(s);

    return 0;
}
// #include <iostream>
// using namespace std;

// bool rotateString(string s, string goal) {

//     if(s.length() != goal.length())
//         return false;

//     string temp = s + s;

//     int n = temp.length();
//     int m = goal.length();

//     for(int i = 0; i <= n - m; i++) {

//         int j;

//         for(j = 0; j < m; j++) {

//             if(temp[i + j] != goal[j])
//                 break;
//         }

//         if(j == m)
//             return true;
//     }

//     return false;
// }

// int main() {

//     string s, goal;

//     cout << "Enter first string: ";
//     cin >> s;

//     cout << "Enter second string: ";
//     cin >> goal;

//     if(rotateString(s, goal))
//         cout << "TRUE";
//     else
//         cout << "FALSE";

//     return 0;
// }