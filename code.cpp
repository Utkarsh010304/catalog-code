#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

using namespace std;

// Trim spaces from start and end of string
string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Extract integer value from a line (assumes first number in line)
int getValueFromLine(const string& line) {
    for (size_t i = 0; i < line.size(); i++) {
        if (isdigit(line[i]) || (line[i] == '-' && i + 1 < line.size() && isdigit(line[i+1]))) {
            size_t start = i;
            size_t end = start + 1;
            while (end < line.size() && isdigit(line[end])) end++;
            return stoi(line.substr(start, end - start));
        }
    }
    cerr << "Error: No digits found in line: " << line << endl;
    exit(1);
}

// Parse n and k values from lines starting at startLine (looking for lines with "n" and "k")
void parseKeys(const vector<string>& lines, int startLine, int &n, int &k) {
    n = -1;
    k = -1;
    for (int i = startLine; i < (int)lines.size(); i++) {
        string line = lines[i];
        if (line.find("\"n\"") != string::npos) {
            n = getValueFromLine(line);
        } else if (line.find("\"k\"") != string::npos) {
            k = getValueFromLine(line);
        }
        if (n != -1 && k != -1) break;
    }
    if (n == -1 || k == -1) {
        cerr << "Error: Could not find n or k in input." << endl;
        exit(1);
    }
}

// Convert digit character to integer value (supports 0-9, a-f, A-F)
int charToDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    cerr << "Invalid digit character: " << c << endl;
    exit(1);
}

// Convert string number from base to decimal (as unsigned long long)
unsigned long long baseToDecimal(const string& val, int base) {
    unsigned long long result = 0;
    for (char c : val) {
        int digit = charToDigit(c);
        if (digit >= base) {
            cerr << "Digit " << c << " invalid for base " << base << endl;
            exit(1);
        }
        result = result * base + digit;
    }
    return result;
}

int main() {
    ifstream fin("input.json");
    if (!fin.is_open()) {
        cerr << "Error: Could not open input.json file" << endl;
        return 1;
    }

    vector<string> lines;
    string line;
    while (getline(fin, line)) {
        line = trim(line);
        if (!line.empty())
            lines.push_back(line);
    }
    fin.close();

    // Find line containing "keys"
    int keysLine = -1;
    for (int i = 0; i < (int)lines.size(); i++) {
        if (lines[i].find("\"keys\"") != string::npos) {
            keysLine = i;
            break;
        }
    }
    if (keysLine == -1) {
        cerr << "Error: No \"keys\" found in input" << endl;
        return 1;
    }

    int n, k;
    parseKeys(lines, keysLine, n, k);

    cout << "Parsed n = " << n << ", k = " << k << endl;

    // Now parse the roots (keys are integers as strings, e.g. "1", "2", "3", ...)
    // After the "keys" block, we have the roots
    // We will store root as map<int rootID, pair<int base, string value>>
    unordered_map<int, pair<int, string>> roots;

    for (int i = keysLine + 1; i < (int)lines.size(); i++) {
        string curLine = lines[i];

        // Root key line example: "1": {
        if (curLine.size() > 3 && curLine[0] == '"' && isdigit(curLine[1])) {
            // Extract root ID number
            int rootID = 0;
            size_t pos = 1;
            while (pos < curLine.size() && isdigit(curLine[pos])) {
                rootID = rootID * 10 + (curLine[pos] - '0');
                pos++;
            }

            // Read base and value in next few lines
            // Expecting lines:
            // "base": "10",
            // "value": "4"
            int base = -1;
            string value;

            // Safety check to not go out of range
            if (i + 2 >= (int)lines.size()) {
                cerr << "Input incomplete for root " << rootID << endl;
                return 1;
            }

            // Parse base line
            string baseLine = lines[i + 1];
            size_t bstart = baseLine.find("\"base\"");
            if (bstart == string::npos) {
                cerr << "Missing base for root " << rootID << endl;
                return 1;
            }
            size_t q1 = baseLine.find("\"", bstart + 6);
            size_t q2 = baseLine.find("\"", q1 + 1);
            if (q1 == string::npos || q2 == string::npos) {
                cerr << "Malformed base line for root " << rootID << endl;
                return 1;
            }
            string baseStr = baseLine.substr(q1 + 1, q2 - q1 - 1);
            base = stoi(baseStr);

            // Parse value line
            string valueLine = lines[i + 2];
            size_t vstart = valueLine.find("\"value\"");
            if (vstart == string::npos) {
                cerr << "Missing value for root " << rootID << endl;
                return 1;
            }
            size_t q3 = valueLine.find("\"", vstart + 7);
            size_t q4 = valueLine.find("\"", q3 + 1);
            if (q3 == string::npos || q4 == string::npos) {
                cerr << "Malformed value line for root " << rootID << endl;
                return 1;
            }
            value = valueLine.substr(q3 + 1, q4 - q3 - 1);

            roots[rootID] = {base, value};

            i += 3; // skip closing '}' line for this root
        }
    }

    cout << "Parsed Roots:\n";
    for (const auto& [id, p] : roots) {
        int base = p.first;
        string val = p.second;
        unsigned long long decVal = baseToDecimal(val, base);
        cout << "Root ID " << id << ": base = " << base << ", value = " << val << ", decimal = " << decVal << "\n";
    }

    return 0;
}
