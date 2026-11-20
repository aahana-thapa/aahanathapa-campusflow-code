// ============================================================
// 1. GENERAL INPUT, TEXT AND FILE UTILITIES
// ============================================================

const string USER_FILE = "users.txt";
const string COMPLAINT_FILE = "complaints.txt";
const string STAFF_FILE = "staff.txt";
const string NOTIFICATION_FILE = "notifications.txt";
const string HISTORY_FILE = "history.txt";
const string LOCATION_FILE = "campus_locations.txt";
const string ROAD_FILE = "campus_roads.txt";
const string REPORT_FILE = "campusflow_report.csv";

string trim(const string &value) {
    size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string normalize(const string &value) {
    string result;
    bool pendingSpace = false;
    for (unsigned char ch : value) {
        if (isspace(ch)) {
            if (!result.empty()) pendingSpace = true;
        } else {
            if (pendingSpace) result += ' ';
            result += static_cast<char>(tolower(ch));
            pendingSpace = false;
        }
    }
    return result;
}

string safeField(string value) {
    for (char &ch : value) {
        if (ch == '|') ch = '/';
        if (ch == '\n' || ch == '\r') ch = ' ';
    }
    return trim(value);
}

vector<string> split(const string &line, char delimiter = '|') {
    vector<string> fields;
    string field;
    stringstream input(line);
    while (getline(input, field, delimiter)) fields.push_back(field);
    if (!line.empty() && line.back() == delimiter) fields.push_back("");
    return fields;
}

string currentDateTime() {
    time_t now = time(nullptr);
    tm *local = localtime(&now);
    if (!local) return "Unknown date";
    stringstream output;
    output << put_time(local, "%d-%m-%Y %H:%M:%S");
    return output.str();
}

string currentDate() {
    time_t now = time(nullptr);
    tm *local = localtime(&now);
    if (!local) return "Unknown date";
    stringstream output;
    output << put_time(local, "%d-%m-%Y");
    return output.str();
}

string readLine(const string &prompt, bool allowEmpty = false) {
    string value;
    while (true) {
        cout << prompt;
        if (!getline(cin, value)) {
            if (cin.eof()) {
                cout << "\nInput stream closed. Exiting safely.\n";
                exit(0);
            }
        }
        value = safeField(value);
        if (allowEmpty || !trim(value).empty()) return value;
        cout << "This field cannot be empty. Please try again.\n";
    }
}

int readInt(const string &prompt, int minimum, int maximum) {
    while (true) {
        string input = readLine(prompt);
        try {
            size_t used = 0;
            long long value = stoll(input, &used);
            if (used != input.size()) throw invalid_argument("extra characters");
            if (value < minimum || value > maximum) {
                cout << "Enter a value from " << minimum << " to " << maximum << ".\n";
                continue;
            }
            return static_cast<int>(value);
        } catch (const exception &) {
            cout << "Invalid input. Please enter a whole number.\n";
        }
    }
}

long long readLongLong(const string &prompt, long long minimum, long long maximum) {
    while (true) {
        string input = readLine(prompt);
        try {
            size_t used = 0;
            long long value = stoll(input, &used);
            if (used != input.size()) throw invalid_argument("extra characters");
            if (value < minimum || value > maximum) {
                cout << "Enter a value from " << minimum << " to " << maximum << ".\n";
                continue;
            }
            return value;
        } catch (const exception &) {
            cout << "Invalid input. Please enter a valid whole number.\n";
        }
    }
}

bool readYesNo(const string &prompt) {
    while (true) {
        string answer = normalize(readLine(prompt));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no") return false;
        cout << "Please enter Y or N.\n";
    }
}

void pauseScreen() {
    cout << "\nPress ENTER to continue...";
    string ignored;
    getline(cin, ignored);
}

string csvEscape(const string &value) {
    string result = "\"";
    for (char ch : value) {
        if (ch == '"') result += "\"\"";
        else result += ch;
    }
    result += '"';
    return result;
}

bool validStatus(const string &status) {
    return status == "Pending" || status == "In Progress" || status == "Resolved";
}

string statusFromChoice(int choice) {
    if (choice == 1) return "Pending";
    if (choice == 2) return "In Progress";
    return "Resolved";
}