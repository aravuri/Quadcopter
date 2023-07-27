//
// Created by Muralidhar Ravuri on 10/21/18.
//

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <sys/time.h>

using namespace std;

vector<string> split(string &row, char separator) {
    stringstream ss(row);
    vector<string> result;
    while (ss.good()) {
        string substr;
        getline(ss, substr, separator);
        result.push_back(substr);
    }
    return result;
}

string mapToString(const map<string, string>& input) {
    stringstream ss;
    ss << "{";
    bool first = true;
    for (auto const &row : input) {
        if (first) {
            first = false;
        } else {
            ss << ",";
        }
        ss << '"' << row.first << '"' << ":" << '"' << row.second << '"';
    }
    ss << "}";
    return ss.str();
}

uint64_t currentMicroSecondsSinceEpoch() {
    struct timeval tv{};

    gettimeofday(&tv, nullptr);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

#endif /* UTIL_HPP_ */
