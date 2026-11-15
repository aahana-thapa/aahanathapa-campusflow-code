#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <limits>
#include <cctype>
#include <iomanip>
#include <ctime>
#include <functional>
#include <stdexcept>

using namespace std;

/*
    CAMPUSFLOW - SMART CAMPUS COMPLAINT MANAGEMENT SYSTEM
    ----------------------------------------------------
    Build: g++ -std=c++17 -Wall -Wextra CampusFlow.cpp -o CampusFlow
    Windows: CampusFlow.exe
    Linux/macOS: ./CampusFlow

    Data files are created in the program's current working directory:
      users.txt, complaints.txt, staff.txt, notifications.txt,
      history.txt, campus_locations.txt, campus_roads.txt

    This is an educational console application. Passwords are stored as
    plain text in users.txt and must not be used for real authentication.
*/

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

// ============================================================
// 2. USER, STUDENT AND ADMINISTRATOR CLASSES
// ============================================================

class User {
protected:
    string userID;
    string name;
    string password;
public:
    User(const string &id = "", const string &n = "", const string &p = "")
        : userID(id), name(n), password(p) {}
    virtual void showRole() const = 0;
    virtual string roleName() const = 0;
    virtual ~User() = default;
    string getID() const { return userID; }
    string getName() const { return name; }
    bool checkPassword(const string &entered) const { return password == entered; }
};

class Student : public User {
public:
    Student(const string &id = "", const string &n = "", const string &p = "")
        : User(id, n, p) {}
    void showRole() const override { cout << "Student"; }
    string roleName() const override { return "Student"; }
};

class Administrator : public User {
public:
    Administrator(const string &id = "", const string &n = "", const string &p = "")
        : User(id, n, p) {}
    void showRole() const override { cout << "Administrator"; }
    string roleName() const override { return "Administrator"; }
};

struct Account {
    string id;
    string name;
    string password;
    string role;
    string department;
};

class UserManager {
    unordered_map<string, Account> accounts;
public:
    UserManager() { load(); }

    bool exists(const string &id) const {
        return accounts.find(normalize(id)) != accounts.end();
    }

    bool registerStudent() {
        cout << "\n--- Student Account Registration ---\n";
        Account account;
        account.id = readLine("Create student ID: ");
        string key = normalize(account.id);
        if (accounts.count(key)) {
            cout << "This student ID is already registered.\n";
            return false;
        }
        account.name = readLine("Full name: ");
        account.department = readLine("Department / branch: ");
        account.password = readLine("Create password: ");
        if (account.password.size() < 4) {
            cout << "For this demo, use a password of at least 4 characters.\n";
            return false;
        }
        account.role = "Student";
        accounts[key] = account;
        save();
        cout << "Student account created successfully. You can now log in.\n";
        return true;
    }

    bool loginStudent(Account &loggedIn) const {
        cout << "\n--- Student Login ---\n";
        string id = readLine("Student ID: ");
        string password = readLine("Password: ");
        auto it = accounts.find(normalize(id));
        if (it == accounts.end() || it->second.role != "Student" ||
            it->second.password != password) {
            cout << "Incorrect student ID or password.\n";
            return false;
        }
        loggedIn = it->second;
        cout << "Welcome, " << loggedIn.name << ".\n";
        return true;
    }

    bool loginAdmin(Account &loggedIn) const {
        cout << "\n--- Administrator Login ---\n";
        string id = readLine("Administrator ID: ");
        string password = readLine("Password: ");
        auto it = accounts.find(normalize(id));
        if (it == accounts.end() || it->second.role != "Administrator" ||
            it->second.password != password) {
            cout << "Incorrect administrator credentials.\n";
            return false;
        }
        loggedIn = it->second;
        cout << "Welcome, " << loggedIn.name << ".\n";
        return true;
    }

    void listUsers() const {
        cout << "\n--- Registered Users ---\n";
        if (accounts.empty()) {
            cout << "No accounts found.\n";
            return;
        }
        for (const auto &entry : accounts) {
            const Account &a = entry.second;
            cout << a.role << " | ID: " << a.id << " | Name: " << a.name
                 << " | Department: " << a.department << '\n';
        }
    }

    void save() const {
        ofstream out(USER_FILE);
        if (!out) throw runtime_error("Unable to write users file.");
        for (const auto &entry : accounts) {
            const Account &a = entry.second;
            out << safeField(a.id) << '|' << safeField(a.name) << '|'
                << safeField(a.password) << '|' << a.role << '|'
                << safeField(a.department) << '\n';
        }
    }

    void load() {
        ifstream in(USER_FILE);
        string line;
        while (getline(in, line)) {
            vector<string> f = split(line);
            if (f.size() != 5) continue;
            if (f[0].empty() || (f[3] != "Student" && f[3] != "Administrator")) continue;
            Account a{f[0], f[1], f[2], f[3], f[4]};
            accounts[normalize(a.id)] = a;
        }
    }

    void createFirstAdministrator() {
        bool adminFound = false;
        for (const auto &entry : accounts)
            if (entry.second.role == "Administrator") adminFound = true;
        if (adminFound) return;
        cout << "\nNo administrator account exists. Create the first administrator.\n";
        Account a;
        a.id = readLine("New administrator ID: ");
        a.name = readLine("Administrator name: ");
        a.department = "Administration";
        a.password = readLine("Create administrator password: ");
        if (a.password.size() < 4) {
            cout << "Password must contain at least 4 characters. Please create it again.\n";
            a.password = readLine("Create administrator password: ");
        }
        a.role = "Administrator";
        accounts[normalize(a.id)] = a;
        save();
        cout << "Administrator account created. Keep these credentials safe.\n";
    }
};

// ============================================================
// 3. COMPLAINT DATA MODEL AND LINKED-LIST NODE
// ============================================================

struct Complaint {
    int id = 0;
    string studentID;
    string studentName;
    string category;
    string location;
    string description;
    string date;
    string createdAt;
    string status = "Pending";
    string assignedStaff = "Unassigned";
    string staffID;
    string lastUpdated;
    int severity = 1;
    int affectedPeople = 1;
    int daysPending = 0;
    int priority = 0;

    void calculatePriority() {
        priority = severity * 20 + min(affectedPeople, 100) + min(daysPending, 30);
    }

    bool isEmergency() const {
        return severity >= 9 || (severity >= 7 && affectedPeople >= 50);
    }
};

struct ComplaintNode {
    Complaint data;
    ComplaintNode *next;
    explicit ComplaintNode(const Complaint &complaint) : data(complaint), next(nullptr) {}
};

// ============================================================
// 4. NOTIFICATION QUEUE
// ============================================================

struct Notification {
    string recipientID;
    string message;
    string time;
    bool read = false;
};

class NotificationManager {
    queue<Notification> messages;
    vector<Notification> archive;
public:
    void add(const string &recipient, const string &message) {
        Notification n{recipient, message, currentDateTime(), false};
        messages.push(n);
        archive.push_back(n);
    }

    void notifyAll(const string &message) { add("ALL", message); }

    void showFor(const string &userID, bool administrator = false) const {
        bool found = false;
        cout << "\n--- Notifications ---\n";
        for (const Notification &n : archive) {
            if (n.recipientID == userID || n.recipientID == "ALL" || administrator) {
                cout << (n.read ? "[Read] " : "[New] ") << n.time
                     << " | " << n.message << '\n';
                found = true;
            }
        }
        if (!found) cout << "No notifications available.\n";
    }

    void markRead(const string &userID) {
        bool changed = false;
        for (Notification &n : archive) {
            if (n.recipientID == userID || n.recipientID == "ALL") {
                n.read = true;
                changed = true;
            }
        }
        if (changed) cout << "Your notifications have been marked as read.\n";
        else cout << "No notifications to mark as read.\n";
    }

    void showQueue() const {
        if (messages.empty()) {
            cout << "Notification queue is empty.\n";
            return;
        }
        queue<Notification> copy = messages;
        cout << "\n--- Notification Queue (oldest first) ---\n";
        while (!copy.empty()) {
            const Notification &n = copy.front();
            cout << n.time << " | " << n.recipientID << " | " << n.message << '\n';
            copy.pop();
        }
    }

    void clearQueue() {
        while (!messages.empty()) messages.pop();
        cout << "Pending notification queue cleared. Archived notifications remain saved.\n";
    }

    void save() const {
        ofstream out(NOTIFICATION_FILE);
        if (!out) throw runtime_error("Unable to save notifications.");
        for (const Notification &n : archive)
            out << safeField(n.recipientID) << '|' << safeField(n.time) << '|'
                << (n.read ? 1 : 0) << '|' << safeField(n.message) << '\n';
    }

    void load() {
        ifstream in(NOTIFICATION_FILE);
        string line;
        while (getline(in, line)) {
            vector<string> f = split(line);
            if (f.size() != 4) continue;
            Notification n;
            n.recipientID = f[0];
            n.time = f[1];
            n.read = f[2] == "1";
            n.message = f[3];
            archive.push_back(n);
            if (!n.read) messages.push(n);
        }
    }
};

// ============================================================
// 5. STATUS HISTORY STACK AND UNDO
// ============================================================

struct StatusAction {
    int complaintID = 0;
    string oldStatus;
    string newStatus;
    string changedAt;
};

class HistoryManager {
    stack<StatusAction> actions;
    vector<StatusAction> allActions;
public:
    void record(int id, const string &oldStatus, const string &newStatus) {
        StatusAction action{id, oldStatus, newStatus, currentDateTime()};
        actions.push(action);
        allActions.push_back(action);
    }

    bool undoAction(StatusAction &action) {
        if (actions.empty()) return false;
        action = actions.top();
        actions.pop();
        return true;
    }

    void show() const {
        if (allActions.empty()) {
            cout << "No status history recorded.\n";
            return;
        }
        cout << "\n--- Complete Status History ---\n";
        for (auto it = allActions.rbegin(); it != allActions.rend(); ++it)
            cout << "#" << it->complaintID << " | " << it->oldStatus << " -> "
                 << it->newStatus << " | " << it->changedAt << '\n';
    }

    void save() const {
        ofstream out(HISTORY_FILE);
        if (!out) throw runtime_error("Unable to save history.");
        for (const StatusAction &a : allActions)
            out << a.complaintID << '|' << safeField(a.oldStatus) << '|'
                << safeField(a.newStatus) << '|' << safeField(a.changedAt) << '\n';
    }

    void load() {
        ifstream in(HISTORY_FILE);
        string line;
        while (getline(in, line)) {
            vector<string> f = split(line);
            if (f.size() != 4) continue;
            try {
                StatusAction a{stoi(f[0]), f[1], f[2], f[3]};
                allActions.push_back(a);
                actions.push(a);
            } catch (const exception &) {
                continue;
            }
        }
    }
};

// ============================================================
// 6. CUSTOM MAX-HEAP FOR PRIORITY MANAGEMENT
// ============================================================

struct HeapItem {
    int complaintID;
    int priority;
};

class PriorityManager {
    vector<HeapItem> heap;

    bool higher(const HeapItem &a, const HeapItem &b) const {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.complaintID < b.complaintID;
    }

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (!higher(heap[index], heap[parent])) break;
            swap(heap[index], heap[parent]);
            index = parent;
        }
    }

    void heapifyDown(int index) {
        int count = static_cast<int>(heap.size());
        while (true) {
            int best = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            if (left < count && higher(heap[left], heap[best])) best = left;
            if (right < count && higher(heap[right], heap[best])) best = right;
            if (best == index) break;
            swap(heap[index], heap[best]);
            index = best;
        }
    }
public:
    void clear() { heap.clear(); }
    bool empty() const { return heap.empty(); }
    int size() const { return static_cast<int>(heap.size()); }

    void insert(int id, int priority) {
        heap.push_back({id, priority});
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    HeapItem top() const {
        if (heap.empty()) throw underflow_error("Priority heap is empty.");
        return heap.front();
    }

    HeapItem removeTop() {
        if (heap.empty()) throw underflow_error("Priority heap is empty.");
        HeapItem result = heap.front();
        heap.front() = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return result;
    }

    void display() const {
        if (heap.empty()) {
            cout << "No active complaints in the priority queue.\n";
            return;
        }
        PriorityManager copy = *this;
        cout << "\n--- Active Complaints: Highest Priority First ---\n";
        int position = 1;
        while (!copy.empty()) {
            HeapItem item = copy.removeTop();
            cout << position++ << ". Complaint #" << item.complaintID
                 << " | Priority: " << item.priority << '\n';
        }
    }
};

// ============================================================
// 7. STAFF DIRECTORY AND AUTOMATIC STAFF ALLOCATION
// ============================================================

struct StaffMember {
    string id;
    string name;
    string department;
    bool available = true;
    int activeAssignments = 0;
};

class StaffManager {
    vector<StaffMember> staff;
public:
    StaffManager() { load(); }

    int count() const { return static_cast<int>(staff.size()); }

    void addStaff() {
        cout << "\n--- Add Maintenance Staff ---\n";
        StaffMember member;
        member.id = readLine("Staff ID: ");
        if (findByID(member.id) != nullptr) {
            cout << "Staff ID already exists.\n";
            return;
        }
        member.name = readLine("Staff name: ");
        member.department = readLine("Department / skill (Electrical, Plumbing, Network, Cleaning, General): ");
        member.available = true;
        member.activeAssignments = 0;
        staff.push_back(member);
        save();
        cout << "Staff member added.\n";
    }

    StaffMember *findByID(const string &id) {
        for (StaffMember &member : staff)
            if (normalize(member.id) == normalize(id)) return &member;
        return nullptr;
    }

    const StaffMember *findByID(const string &id) const {
        for (const StaffMember &member : staff)
            if (normalize(member.id) == normalize(id)) return &member;
        return nullptr;
    }

    void showStaff() const {
        cout << "\n--- Staff Directory ---\n";
        if (staff.empty()) {
            cout << "No staff members added. Add staff before assigning complaints.\n";
            return;
        }
        for (const StaffMember &member : staff) {
            cout << "ID: " << member.id << " | " << member.name
                 << " | Department: " << member.department
                 << " | " << (member.available ? "Available" : "Unavailable")
                 << " | Active assignments: " << member.activeAssignments << '\n';
        }
    }

    string automaticAssign(const string &category, string &staffID) {
        StaffMember *best = nullptr;
        string wanted = normalize(category);
        for (StaffMember &member : staff) {
            if (!member.available) continue;
            string department = normalize(member.department);
            bool match = department == "general" || department == wanted ||
                         wanted.find(department) != string::npos ||
                         department.find(wanted) != string::npos;
            if (!match) continue;
            if (!best || member.activeAssignments < best->activeAssignments) best = &member;
        }
        if (!best) return "Unassigned";
        ++best->activeAssignments;
        staffID = best->id;
        save();
        return best->name + " (" + best->department + ")";
    }

    bool manualAssign(const string &id, string &displayName) {
        StaffMember *member = findByID(id);
        if (!member) return false;
        if (!member->available) {
            cout << "This staff member is currently unavailable.\n";
            return false;
        }
        ++member->activeAssignments;
        displayName = member->name + " (" + member->department + ")";
        save();
        return true;
    }

    void releaseAssignment(const string &id) {
        StaffMember *member = findByID(id);
        if (member && member->activeAssignments > 0) {
            --member->activeAssignments;
            save();
        }
    }

    void changeAvailability() {
        showStaff();
        if (staff.empty()) return;
        string id = readLine("Enter staff ID: ");
        StaffMember *member = findByID(id);
        if (!member) {
            cout << "Staff member not found.\n";
            return;
        }
        member->available = !member->available;
        save();
        cout << member->name << " is now " << (member->available ? "available" : "unavailable") << ".\n";
    }

    void save() const {
        ofstream out(STAFF_FILE);
        if (!out) throw runtime_error("Unable to save staff directory.");
        for (const StaffMember &member : staff)
            out << safeField(member.id) << '|' << safeField(member.name) << '|'
                << safeField(member.department) << '|' << (member.available ? 1 : 0)
                << '|' << member.activeAssignments << '\n';
    }

    void load() {
        ifstream in(STAFF_FILE);
        string line;
        while (getline(in, line)) {
            vector<string> f = split(line);
            if (f.size() != 5) continue;
            try {
                StaffMember m{f[0], f[1], f[2], f[3] == "1", stoi(f[4])};
                staff.push_back(m);
            } catch (const exception &) {
                continue;
            }
        }
    }
};

// ============================================================
// 8. CAMPUS GRAPH: BFS, DFS AND DIJKSTRA SHORTEST PATH
// ============================================================

class CampusGraph {
    vector<string> places;
    vector<vector<pair<int, int>>> adjacency;

    int indexOf(const string &name) const {
        for (int i = 0; i < static_cast<int>(places.size()); ++i)
            if (normalize(places[i]) == normalize(name)) return i;
        return -1;
    }

    void dfsVisit(int current, vector<bool> &visited) const {
        visited[current] = true;
        cout << places[current] << " -> ";
        for (const auto &edge : adjacency[current])
            if (!visited[edge.first]) dfsVisit(edge.first, visited);
    }
public:
    CampusGraph() { load(); }

    int placeCount() const { return static_cast<int>(places.size()); }

    void addLocation(const string &name) {
        if (indexOf(name) != -1) {
            cout << "That campus location already exists.\n";
            return;
        }
        places.push_back(name);
        adjacency.emplace_back();
        save();
        cout << "Location added. Add roads to connect it to other locations.\n";
    }

    void addRoadByName(const string &from, const string &to, int distance) {
        int u = indexOf(from), v = indexOf(to);
        if (u == -1 || v == -1 || u == v || distance <= 0) {
            cout << "Invalid locations or distance. Road was not added.\n";
            return;
        }
        for (const auto &edge : adjacency[u]) {
            if (edge.first == v) {
                cout << "A road between these locations already exists.\n";
                return;
            }
        }
        adjacency[u].push_back({v, distance});
        adjacency[v].push_back({u, distance});
        save();
        cout << "Road added successfully.\n";
    }

    void showPlaces() const {
        cout << "\n--- Campus Locations ---\n";
        if (places.empty()) {
            cout << "No locations configured.\n";
            return;
        }
        for (int i = 0; i < static_cast<int>(places.size()); ++i)
            cout << i + 1 << ". " << places[i] << '\n';
    }

    void addLocationInteractive() {
        string name = readLine("New location name: ");
        addLocation(name);
    }

    void addRoadInteractive() {
        showPlaces();
        string from = readLine("Starting location (type exact name): ");
        string to = readLine("Destination location (type exact name): ");
        int distance = readInt("Distance / travel cost (positive integer): ", 1, 1000000);
        addRoadByName(from, to, distance);
    }

    void bfs(int start) const {
        if (start < 0 || start >= placeCount()) return;
        vector<bool> visited(places.size(), false);
        queue<int> pending;
        visited[start] = true;
        pending.push(start);
        cout << "BFS traversal: ";
        while (!pending.empty()) {
            int current = pending.front();
            pending.pop();
            cout << places[current] << " -> ";
            for (const auto &edge : adjacency[current]) {
                if (!visited[edge.first]) {
                    visited[edge.first] = true;
                    pending.push(edge.first);
                }
            }
        }
        cout << "END\n";
    }

    void dfs(int start) const {
        if (start < 0 || start >= placeCount()) return;
        vector<bool> visited(places.size(), false);
        cout << "DFS traversal: ";
        dfsVisit(start, visited);
        cout << "END\n";
    }

    void shortestPath(int start, int destination) const {
        int count = placeCount();
        if (start < 0 || destination < 0 || start >= count || destination >= count) return;
        const int INF = numeric_limits<int>::max();
        vector<int> distance(count, INF), parent(count, -1);
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pending;
        distance[start] = 0;
        pending.push({0, start});
        while (!pending.empty()) {
            int cost = pending.top().first;
            int current = pending.top().second;
            pending.pop();
            if (cost != distance[current]) continue;
            for (const auto &edge : adjacency[current]) {
                int next = edge.first;
                int weight = edge.second;
                if (distance[current] <= INF - weight && distance[current] + weight < distance[next]) {
                    distance[next] = distance[current] + weight;
                    parent[next] = current;
                    pending.push({distance[next], next});
                }
            }
        }
        if (distance[destination] == INF) {
            cout << "No connected route exists between these locations.\n";
            return;
        }
        vector<int> path;
        for (int at = destination; at != -1; at = parent[at]) path.push_back(at);
        reverse(path.begin(), path.end());
        cout << "Shortest route: ";
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) cout << " -> ";
            cout << places[path[i]];
        }
        cout << "\nTotal distance units: " << distance[destination] << '\n';
    }

    void navigationMenu() const {
        if (places.empty()) {
            cout << "No campus locations available.\n";
            return;
        }
        showPlaces();
        int start = readInt("Starting location number: ", 1, placeCount()) - 1;
        int destination = readInt("Destination location number: ", 1, placeCount()) - 1;
        bfs(start);
        dfs(start);
        shortestPath(start, destination);
    }

    void save() const {
        ofstream locationOut(LOCATION_FILE);
        ofstream roadOut(ROAD_FILE);
        if (!locationOut || !roadOut) throw runtime_error("Unable to save campus graph files.");
        for (const string &place : places) locationOut << safeField(place) << '\n';
        for (int u = 0; u < static_cast<int>(adjacency.size()); ++u)
            for (const auto &edge : adjacency[u])
                if (u < edge.first) roadOut << u << '|' << edge.first << '|' << edge.second << '\n';
    }

    void load() {
        ifstream locationIn(LOCATION_FILE);
        string line;
        while (getline(locationIn, line)) if (!trim(line).empty()) places.push_back(trim(line));
        if (places.empty()) {
            places = {"Main Gate", "Academic Block", "Library", "Hostel", "Cafeteria", "Sports Ground"};
            adjacency.resize(places.size());
            addRoadByName("Main Gate", "Academic Block", 4);
            addRoadByName("Academic Block", "Library", 2);
            addRoadByName("Academic Block", "Hostel", 5);
            addRoadByName("Library", "Cafeteria", 3);
            addRoadByName("Hostel", "Cafeteria", 2);
            addRoadByName("Cafeteria", "Sports Ground", 4);
            addRoadByName("Main Gate", "Sports Ground", 10);
            save();
            return;
        }
        adjacency.resize(places.size());
        ifstream roadIn(ROAD_FILE);
        while (getline(roadIn, line)) {
            vector<string> f = split(line);
            if (f.size() != 3) continue;
            try {
                int u = stoi(f[0]), v = stoi(f[1]), w = stoi(f[2]);
                if (u >= 0 && v >= 0 && u < placeCount() && v < placeCount() && u != v && w > 0) {
                    adjacency[u].push_back({v, w});
                    adjacency[v].push_back({u, w});
                }
            } catch (const exception &) {
                continue;
            }
        }
    }
};

// ============================================================
// 9. COMPLAINT MANAGER: LINKED LIST + HASH TABLE + CRUD
// ============================================================

class ComplaintManager {
    ComplaintNode *head = nullptr;
    ComplaintNode *tail = nullptr;
    int nextID = 1001;
    unordered_map<int, ComplaintNode*> idIndex;

    bool duplicateExists(const Complaint &candidate, int ignoredID = -1) const {
        string category = normalize(candidate.category);
        string location = normalize(candidate.location);
        string description = normalize(candidate.description);
        for (ComplaintNode *current = head; current; current = current->next) {
            const Complaint &existing = current->data;
            if (existing.id == ignoredID || existing.status == "Resolved") continue;
            if (normalize(existing.category) == category &&
                normalize(existing.location) == location &&
                normalize(existing.description) == description) return true;
        }
        return false;
    }

    void append(const Complaint &complaint) {
        if (idIndex.count(complaint.id)) return;
        ComplaintNode *node = new ComplaintNode(complaint);
        if (!head) head = tail = node;
        else {
            tail->next = node;
            tail = node;
        }
        idIndex[complaint.id] = node;
        nextID = max(nextID, complaint.id + 1);
    }

    void displayComplaint(const Complaint &c) const {
        cout << "\n==================================================\n";
        cout << "Complaint ID: " << c.id << '\n';
        cout << "Student: " << c.studentName << " | Student ID: " << c.studentID << '\n';
        cout << "Category: " << c.category << " | Location: " << c.location << '\n';
        cout << "Description: " << c.description << '\n';
        cout << "Reported date: " << c.date << " | Created: " << c.createdAt << '\n';
        cout << "Status: " << c.status << " | Last updated: " << c.lastUpdated << '\n';
        cout << "Severity: " << c.severity << "/10 | People affected: " << c.affectedPeople << '\n';
        cout << "Days pending: " << c.daysPending << " | Priority score: " << c.priority << '\n';
        cout << "Assigned staff: " << c.assignedStaff << " | Staff ID: "
             << (c.staffID.empty() ? "Not assigned" : c.staffID) << '\n';
        cout << "Emergency flag: " << (c.isEmergency() ? "YES" : "No") << '\n';
        cout << "==================================================\n";
    }

public:
    ComplaintManager() = default;
    ComplaintManager(const ComplaintManager &) = delete;
    ComplaintManager &operator=(const ComplaintManager &) = delete;

    ~ComplaintManager() {
        ComplaintNode *current = head;
        while (current) {
            ComplaintNode *next = current->next;
            delete current;
            current = next;
        }
    }

    int getNextID() const { return nextID; }

    Complaint *find(int id) {
        auto found = idIndex.find(id);
        if (found == idIndex.end()) return nullptr;
        return &found->second->data;
    }

    const Complaint *find(int id) const {
        auto found = idIndex.find(id);
        if (found == idIndex.end()) return nullptr;
        return &found->second->data;
    }

    bool registerComplaint(const Account &student, NotificationManager &notifications) {
        Complaint c;
        c.studentID = student.id;
        c.studentName = student.name;
        c.date = readLine("Reported date (DD-MM-YYYY): ");
        c.category = readLine("Category (Electrical/Water/Network/Cleaning/General): ");
        c.location = readLine("Campus location: ");
        c.description = readLine("Describe the issue: ");
        c.severity = readInt("Severity (1 = low, 10 = critical): ", 1, 10);
        c.affectedPeople = readInt("Number of people affected: ", 1, 1000000);
        c.daysPending = 0;
        c.status = "Pending";
        c.assignedStaff = "Unassigned";
        c.createdAt = currentDateTime();
        c.lastUpdated = c.createdAt;
        c.calculatePriority();

        if (duplicateExists(c)) {
            cout << "A similar active complaint already exists for this category and location.\n";
            cout << "No new complaint was created. You may track the existing complaint by ID.\n";
            return false;
        }

        c.id = nextID;
        append(c);
        notifications.add(student.id, "Your complaint #" + to_string(c.id) + " was registered.");
        notifications.add("ADMIN", "New complaint #" + to_string(c.id) + " was registered by " + student.name + ".");
        cout << "Complaint registered successfully.\nYour complaint ID is: " << c.id << '\n';
        cout << "Priority score: " << c.priority;
        if (c.isEmergency()) cout << " | EMERGENCY FLAGGED";
        cout << '\n';
        return true;
    }

    void displayAll() const {
        if (!head) {
            cout << "No complaints have been registered.\n";
            return;
        }
        for (ComplaintNode *current = head; current; current = current->next)
            displayComplaint(current->data);
    }

    void searchByID() const {
        int id = readInt("Enter complaint ID: ", 1, 2000000000);
        const Complaint *complaint = find(id);
        if (!complaint) cout << "Complaint ID not found.\n";
        else displayComplaint(*complaint);
    }

    void showStudentComplaints(const string &studentID) const {
        bool found = false;
        for (ComplaintNode *current = head; current; current = current->next) {
            if (normalize(current->data.studentID) == normalize(studentID)) {
                displayComplaint(current->data);
                found = true;
            }
        }
        if (!found) cout << "You have not registered any complaints yet.\n";
    }

    void filterByStatus() const {
        cout << "1. Pending\n2. In Progress\n3. Resolved\n";
        string status = statusFromChoice(readInt("Choose status: ", 1, 3));
        bool found = false;
        for (ComplaintNode *current = head; current; current = current->next) {
            if (current->data.status == status) {
                displayComplaint(current->data);
                found = true;
            }
        }
        if (!found) cout << "No complaints found with status " << status << ".\n";
    }

    void filterByCategory() const {
        string category = normalize(readLine("Enter category: "));
        bool found = false;
        for (ComplaintNode *current = head; current; current = current->next) {
            if (normalize(current->data.category) == category) {
                displayComplaint(current->data);
                found = true;
            }
        }
        if (!found) cout << "No complaints found in that category.\n";
    }

    void updateStatus(HistoryManager &history, NotificationManager &notifications) {
        int id = readInt("Complaint ID: ", 1, 2000000000);
        Complaint *c = find(id);
        if (!c) {
            cout << "Complaint not found.\n";
            return;
        }
        cout << "1. Pending\n2. In Progress\n3. Resolved\n";
        string nextStatus = statusFromChoice(readInt("Select new status: ", 1, 3));
        if (nextStatus == c->status) {
            cout << "The complaint already has this status.\n";
            return;
        }
        string oldStatus = c->status;
        c->status = nextStatus;
        c->lastUpdated = currentDateTime();
        history.record(id, oldStatus, nextStatus);
        notifications.add(c->studentID, "Complaint #" + to_string(id) + " status changed to " + nextStatus + ".");
        notifications.add("ADMIN", "Status of complaint #" + to_string(id) + " changed to " + nextStatus + ".");
        cout << "Status updated successfully.\n";
    }

    void undoStatus(HistoryManager &history, NotificationManager &notifications) {
        StatusAction action;
        if (!history.undoAction(action)) {
            cout << "There is no status action available to undo.\n";
            return;
        }
        Complaint *c = find(action.complaintID);
        if (!c || c->status != action.newStatus) {
            cout << "Undo cancelled: this complaint has changed since the recorded action.\n";
            return;
        }
        c->status = action.oldStatus;
        c->lastUpdated = currentDateTime();
        notifications.add(c->studentID, "Last status update for complaint #" + to_string(c->id) + " was undone.");
        cout << "Status restored to " << action.oldStatus << ".\n";
    }

    void updatePendingDays() {
        int id = readInt("Complaint ID: ", 1, 2000000000);
        Complaint *c = find(id);
        if (!c) {
            cout << "Complaint not found.\n";
            return;
        }
        if (c->status == "Resolved") {
            cout << "Resolved complaints do not need pending-day updates.\n";
            return;
        }
        c->daysPending = readInt("Days pending (0-36500): ", 0, 36500);
        c->calculatePriority();
        c->lastUpdated = currentDateTime();
        cout << "Pending days updated. New priority: " << c->priority << '\n';
    }

    void editComplaint() {
        int id = readInt("Complaint ID to edit: ", 1, 2000000000);
        Complaint *c = find(id);
        if (!c) {
            cout << "Complaint not found.\n";
            return;
        }
        cout << "1. Category\n2. Location\n3. Description\n4. Severity\n5. People affected\n";
        int choice = readInt("Choose field to edit: ", 1, 5);
        Complaint updated = *c;
        if (choice == 1) updated.category = readLine("New category: ");
        else if (choice == 2) updated.location = readLine("New location: ");
        else if (choice == 3) updated.description = readLine("New description: ");
        else if (choice == 4) updated.severity = readInt("New severity (1-10): ", 1, 10);
        else updated.affectedPeople = readInt("New number of affected people: ", 1, 1000000);
        if (duplicateExists(updated, id)) {
            cout << "Edit rejected because it would create an active duplicate.\n";
            return;
        }
        updated.calculatePriority();
        updated.lastUpdated = currentDateTime();
        *c = updated;
        cout << "Complaint details updated successfully.\n";
    }

    void assignStaff(StaffManager &staff, NotificationManager &notifications, bool automatic) {
        int id = readInt("Complaint ID: ", 1, 2000000000);
        Complaint *c = find(id);
        if (!c) {
            cout << "Complaint not found.\n";
            return;
        }
        if (c->status == "Resolved") {
            cout << "Resolved complaints cannot receive a new assignment.\n";
            return;
        }
        string displayName;
        string staffID;
        if (automatic) {
            displayName = staff.automaticAssign(c->category, staffID);
            if (displayName == "Unassigned") {
                cout << "No available matching staff member. Add or enable staff first.\n";
                return;
            }
        } else {
            staff.showStaff();
            staffID = readLine("Enter staff ID to assign: ");
            if (!staff.manualAssign(staffID, displayName)) {
                cout << "Assignment was not changed.\n";
                return;
            }
        }
        if (!c->staffID.empty()) staff.releaseAssignment(c->staffID);
        c->staffID = staffID;
        c->assignedStaff = displayName;
        c->lastUpdated = currentDateTime();
        notifications.add(c->studentID, "Complaint #" + to_string(id) + " assigned to " + displayName + ".");
        cout << "Complaint assigned to " << displayName << ".\n";
    }

    void showEmergencies() const {
        bool found = false;
        cout << "\n--- Active Emergency Complaints ---\n";
        for (ComplaintNode *current = head; current; current = current->next) {
            if (current->data.isEmergency() && current->data.status != "Resolved") {
                displayComplaint(current->data);
                found = true;
            }
        }
        if (!found) cout << "No active emergency-flagged complaints.\n";
    }

    void rebuildPriority(PriorityManager &priority) const {
        priority.clear();
        for (ComplaintNode *current = head; current; current = current->next)
            if (current->data.status != "Resolved")
                priority.insert(current->data.id, current->data.priority);
    }

    void analytics() const {
        int total = 0, pending = 0, progress = 0, resolved = 0, emergency = 0, assigned = 0;
        long long prioritySum = 0;
        unordered_map<string, int> categoryCounts;
        unordered_map<string, int> staffCounts;
        for (ComplaintNode *current = head; current; current = current->next) {
            const Complaint &c = current->data;
            ++total;
            prioritySum += c.priority;
            if (c.status == "Pending") ++pending;
            else if (c.status == "In Progress") ++progress;
            else if (c.status == "Resolved") ++resolved;
            if (c.isEmergency() && c.status != "Resolved") ++emergency;
            if (c.assignedStaff != "Unassigned") {
                ++assigned;
                ++staffCounts[c.assignedStaff];
            }
            ++categoryCounts[c.category];
        }
        cout << "\n========== CAMPUSFLOW ANALYTICS ==========\n";
        cout << "Total complaints: " << total << '\n';
        cout << "Pending: " << pending << '\n';
        cout << "In Progress: " << progress << '\n';
        cout << "Resolved: " << resolved << '\n';
        cout << "Active emergencies: " << emergency << '\n';
        cout << "Assigned complaints: " << assigned << '\n';
        cout << "Unassigned complaints: " << total - assigned << '\n';
        cout << fixed << setprecision(2);
        cout << "Average priority: " << (total ? static_cast<double>(prioritySum) / total : 0.0) << '\n';
        cout << "Category-wise complaints:\n";
        if (categoryCounts.empty()) cout << "  No data\n";
        for (const auto &entry : categoryCounts)
            cout << "  " << entry.first << ": " << entry.second << '\n';
        cout << "Staff-wise assignments:\n";
        if (staffCounts.empty()) cout << "  No assignments\n";
        for (const auto &entry : staffCounts)
            cout << "  " << entry.first << ": " << entry.second << '\n';
    }

    void exportCSV() const {
        ofstream out(REPORT_FILE);
        if (!out) {
            cout << "Could not create CSV report. Check folder permissions.\n";
            return;
        }
        out << "ID,Student ID,Student Name,Category,Location,Description,Date,Status,Staff,Severity,Affected People,Days Pending,Priority,Emergency\n";
        for (ComplaintNode *current = head; current; current = current->next) {
            const Complaint &c = current->data;
            out << c.id << ',' << csvEscape(c.studentID) << ',' << csvEscape(c.studentName) << ','
                << csvEscape(c.category) << ',' << csvEscape(c.location) << ','
                << csvEscape(c.description) << ',' << csvEscape(c.date) << ','
                << csvEscape(c.status) << ',' << csvEscape(c.assignedStaff) << ','
                << c.severity << ',' << c.affectedPeople << ',' << c.daysPending << ','
                << c.priority << ',' << (c.isEmergency() ? "Yes" : "No") << '\n';
        }
        cout << "Report exported to " << REPORT_FILE << ".\n";
    }

    void save() const {
        ofstream out(COMPLAINT_FILE);
        if (!out) throw runtime_error("Unable to save complaints file.");
        for (ComplaintNode *current = head; current; current = current->next) {
            const Complaint &c = current->data;
            out << c.id << '|' << safeField(c.studentID) << '|'
                << safeField(c.studentName) << '|' << safeField(c.category) << '|'
                << safeField(c.location) << '|' << safeField(c.description) << '|'
                << safeField(c.date) << '|' << safeField(c.createdAt) << '|'
                << safeField(c.status) << '|' << safeField(c.assignedStaff) << '|'
                << safeField(c.staffID) << '|' << safeField(c.lastUpdated) << '|'
                << c.severity << '|' << c.affectedPeople << '|' << c.daysPending << '\n';
        }
    }

    void load() {
        ifstream in(COMPLAINT_FILE);
        string line;
        while (getline(in, line)) {
            vector<string> f = split(line);
            if (f.size() != 15) continue;
            try {
                Complaint c;
                c.id = stoi(f[0]);
                c.studentID = f[1];
                c.studentName = f[2];
                c.category = f[3];
                c.location = f[4];
                c.description = f[5];
                c.date = f[6];
                c.createdAt = f[7];
                c.status = f[8];
                c.assignedStaff = f[9];
                c.staffID = f[10];
                c.lastUpdated = f[11];
                c.severity = stoi(f[12]);
                c.affectedPeople = stoi(f[13]);
                c.daysPending = stoi(f[14]);
                if (c.id <= 0 || !validStatus(c.status) || c.severity < 1 || c.severity > 10 ||
                    c.affectedPeople < 1 || c.daysPending < 0 || idIndex.count(c.id)) continue;
                c.calculatePriority();
                append(c);
            } catch (const exception &) {
                cout << "Warning: one damaged complaint record was skipped.\n";
            }
        }
    }

    void saveAll() const {
        save();
    }
};

// ============================================================
// 10. EMERGENCY MANAGEMENT WORKFLOW
// ============================================================

class EmergencyManager {
public:
    void review(ComplaintManager &complaints, StaffManager &staff,
                NotificationManager &notifications) {
        cout << "\n--- Emergency Management ---\n";
        complaints.showEmergencies();
        if (!readYesNo("Would you like to assign an emergency complaint now? (Y/N): ")) return;
        complaints.assignStaff(staff, notifications, true);
    }

    void explainRule() const {
        cout << "\nEmergency classification rule used in this project:\n";
        cout << "1. Severity is 9 or 10; OR\n";
        cout << "2. Severity is at least 7 AND at least 50 people are affected.\n";
        cout << "This is a project-defined demonstration rule, not an official campus policy.\n";
    }
};

// ============================================================
// 11. DATA STORAGE COORDINATOR
// ============================================================

class DataManager {
public:
    void saveAll(UserManager &users, ComplaintManager &complaints,
                 StaffManager &staff, NotificationManager &notifications,
                 HistoryManager &history, CampusGraph &graph) {
        users.save();
        complaints.save();
        staff.save();
        notifications.save();
        history.save();
        graph.save();
        cout << "All CampusFlow data saved successfully.\n";
    }
};

// ============================================================
// 12. STUDENT MENU
// ============================================================

void studentMenu(const Account &student, ComplaintManager &complaints,
                 NotificationManager &notifications) {
    while (true) {
        cout << "\n========== STUDENT DASHBOARD ==========\n";
        cout << "Welcome, " << student.name << " | ID: " << student.id << '\n';
        cout << "1. Register a complaint\n";
        cout << "2. View my complaints\n";
        cout << "3. Track complaint by ID\n";
        cout << "4. View my notifications\n";
        cout << "5. Mark notifications as read\n";
        cout << "6. Logout\n";
        int choice = readInt("Choose an option: ", 1, 6);
        if (choice == 1) complaints.registerComplaint(student, notifications);
        else if (choice == 2) complaints.showStudentComplaints(student.id);
        else if (choice == 3) {
            int id = readInt("Complaint ID: ", 1, 2000000000);
            const Complaint *c = complaints.find(id);
            if (!c || normalize(c->studentID) != normalize(student.id))
                cout << "No complaint with that ID belongs to your account.\n";
            else {
                cout << "Complaint status: " << c->status << '\n';
                cout << "Priority score: " << c->priority << '\n';
                cout << "Assigned staff: " << c->assignedStaff << '\n';
            }
        }
        else if (choice == 4) notifications.showFor(student.id);
        else if (choice == 5) notifications.markRead(student.id);
        else break;
    }
}

// ============================================================
// 13. ADMINISTRATOR MENUS
// ============================================================

void staffMenu(StaffManager &staff) {
    while (true) {
        cout << "\n--- Staff Management ---\n";
        cout << "1. View staff directory\n2. Add staff member\n";
        cout << "3. Change staff availability\n4. Back\n";
        int choice = readInt("Choose: ", 1, 4);
        if (choice == 1) staff.showStaff();
        else if (choice == 2) staff.addStaff();
        else if (choice == 3) staff.changeAvailability();
        else break;
    }
}

void campusMenu(CampusGraph &graph) {
    while (true) {
        cout << "\n--- Campus Navigation Management ---\n";
        cout << "1. Show locations\n2. Find route and traversals\n";
        cout << "3. Add campus location\n4. Add road between locations\n5. Back\n";
        int choice = readInt("Choose: ", 1, 5);
        if (choice == 1) graph.showPlaces();
        else if (choice == 2) graph.navigationMenu();
        else if (choice == 3) graph.addLocationInteractive();
        else if (choice == 4) graph.addRoadInteractive();
        else break;
    }
}

void adminMenu(const Account &admin, UserManager &users,
               ComplaintManager &complaints, PriorityManager &priority,
               HistoryManager &history, NotificationManager &notifications,
               StaffManager &staff, CampusGraph &graph,
               EmergencyManager &emergency, DataManager &data) {
    while (true) {
        cout << "\n========== ADMINISTRATOR DASHBOARD ==========\n";
        cout << "Welcome, " << admin.name << " | ID: " << admin.id << '\n';
        cout << " 1. View all complaints\n";
        cout << " 2. Search complaint by ID (hash table)\n";
        cout << " 3. Filter complaints by status\n";
        cout << " 4. Filter complaints by category\n";
        cout << " 5. Edit complaint details\n";
        cout << " 6. Show priority order (max-heap)\n";
        cout << " 7. Update complaint status\n";
        cout << " 8. Undo last status update (stack)\n";
        cout << " 9. View status history\n";
        cout << "10. Update days pending and priority\n";
        cout << "11. Emergency management\n";
        cout << "12. Assign staff automatically\n";
        cout << "13. Assign staff manually\n";
        cout << "14. Staff directory management\n";
        cout << "15. Campus navigation (graph)\n";
        cout << "16. View notifications\n";
        cout << "17. View notification queue\n";
        cout << "18. Clear pending notification queue\n";
        cout << "19. Analytics and reports\n";
        cout << "20. Export complaints to CSV\n";
        cout << "21. List registered users\n";
        cout << "22. Save all data\n";
        cout << "23. Logout\n";
        int choice = readInt("Choose an option: ", 1, 23);

        if (choice == 1) complaints.displayAll();
        else if (choice == 2) complaints.searchByID();
        else if (choice == 3) complaints.filterByStatus();
        else if (choice == 4) complaints.filterByCategory();
        else if (choice == 5) complaints.editComplaint();
        else if (choice == 6) {
            complaints.rebuildPriority(priority);
            priority.display();
        }
        else if (choice == 7) complaints.updateStatus(history, notifications);
        else if (choice == 8) complaints.undoStatus(history, notifications);
        else if (choice == 9) history.show();
        else if (choice == 10) complaints.updatePendingDays();
        else if (choice == 11) {
            emergency.explainRule();
            emergency.review(complaints, staff, notifications);
        }
        else if (choice == 12) complaints.assignStaff(staff, notifications, true);
        else if (choice == 13) complaints.assignStaff(staff, notifications, false);
        else if (choice == 14) staffMenu(staff);
        else if (choice == 15) campusMenu(graph);
        else if (choice == 16) notifications.showFor(admin.id, true);
        else if (choice == 17) notifications.showQueue();
        else if (choice == 18) notifications.clearQueue();
        else if (choice == 19) complaints.analytics();
        else if (choice == 20) complaints.exportCSV();
        else if (choice == 21) users.listUsers();
        else if (choice == 22) {
            try {
                data.saveAll(users, complaints, staff, notifications, history, graph);
            } catch (const exception &error) {
                cout << "Save failed: " << error.what() << '\n';
            }
        }
        else break;
    }
}

// ============================================================
// 14. MAIN APPLICATION AND MENU ROUTING
// ============================================================

int main() {
    try {
        UserManager users;
        ComplaintManager complaints;
        PriorityManager priority;
        HistoryManager history;
        NotificationManager notifications;
        StaffManager staff;
        CampusGraph graph;
        EmergencyManager emergency;
        DataManager data;

        complaints.load();
        notifications.load();
        history.load();
        users.createFirstAdministrator();

        cout << "\n====================================================\n";
        cout << "       CAMPUSFLOW - SMART CAMPUS SYSTEM\n";
        cout << "====================================================\n";
        cout << "One application | Shared records | Role-based menus\n";

        bool running = true;
        while (running) {
            cout << "\n=============== MAIN MENU ===============\n";
            cout << "1. Student login\n";
            cout << "2. Student registration\n";
            cout << "3. Administrator login\n";
            cout << "4. Exit and save\n";
            int choice = readInt("Choose an option: ", 1, 4);

            if (choice == 1) {
                Account student;
                if (users.loginStudent(student))
                    studentMenu(student, complaints, notifications);
            }
            else if (choice == 2) users.registerStudent();
            else if (choice == 3) {
                Account admin;
                if (users.loginAdmin(admin))
                    adminMenu(admin, users, complaints, priority, history,
                              notifications, staff, graph, emergency, data);
            }
            else {
                data.saveAll(users, complaints, staff, notifications, history, graph);
                cout << "Thank you for using CampusFlow. Goodbye!\n";
                running = false;
            }
        }
    } catch (const exception &error) {
        cerr << "A system error occurred: " << error.what() << '\n';
        cerr << "Please check the data files and try again.\n";
        return 1;
    }
    return 0;
}

