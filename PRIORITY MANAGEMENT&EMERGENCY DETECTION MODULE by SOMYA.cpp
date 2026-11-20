/*THIS MODULE HAS BEEN BUILT BY (ME)SOMYA DHOUNDIYAL (TEAM MEMBER-2) TO DEMONSTRATE THE WORKING OF
PRIORITY MANAGEMENT SYSTEM AND EMERGENCY DETECTION SYSTEM IN THE CAMPUSFLOW PROJECT DEVELOPED IN C++
USING THE CONCEPTS OF OOPS AND DATA STRUCTURES*/

//File Location-C:\Users\HP\Desktop\Somya Dhoundiyal\PBL Module

#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdexcept>
using namespace std;

// Complaint data structure
struct Complaint {
    int id;
    string category;
    string location;
    int severity;
    int affectedPeople;
    int daysPending;
    int priority;
    string status;

    Complaint() {
        id = 0;
        severity = 1;
        affectedPeople = 1;
        daysPending = 0;
        priority = 0;
        status = "Pending";
    }

    void calculatePriority() {
        priority = severity * 20
                 + min(affectedPeople, 100)
                 + min(daysPending, 30);
    }

    bool isEmergency() const {
        return severity >= 9 ||
              (severity >= 7 && affectedPeople >= 50);
    }
};

// Heap item
struct HeapItem {
    int complaintID;
    int priority;
};

// Priority Management using Max-Heap
class PriorityManager {
    vector<HeapItem> heap;

    bool higher(const HeapItem &a, const HeapItem &b) const {
        if (a.priority != b.priority)
            return a.priority > b.priority;

        return a.complaintID < b.complaintID;
    }

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;

            if (!higher(heap[index], heap[parent]))
                break;

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

            if (left < count && higher(heap[left], heap[best]))
                best = left;

            if (right < count && higher(heap[right], heap[best]))
                best = right;

            if (best == index)
                break;

            swap(heap[index], heap[best]);
            index = best;
        }
    }

public:
    void clear() {
        heap.clear();
    }

    bool empty() const {
        return heap.empty();
    }

    int size() const {
        return static_cast<int>(heap.size());
    }

    void insert(int id, int priority) {
        if (id <= 0 || priority < 0)
            throw invalid_argument("Invalid complaint data.");

        heap.push_back({id, priority});
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    HeapItem top() const {
        if (heap.empty())
            throw underflow_error("Priority heap is empty.");

        return heap.front();
    }

    HeapItem removeTop() {
        if (heap.empty())
            throw underflow_error("Priority heap is empty.");

        HeapItem result = heap.front();

        heap.front() = heap.back();
        heap.pop_back();

        if (!heap.empty())
            heapifyDown(0);

        return result;
    }

    void display() const {
        if (heap.empty()) {
            cout << "\nNo active complaints.\n";
            return;
        }

        PriorityManager copy = *this;

        cout << "\n--- PRIORITY ORDER ---\n";

        int position = 1;

        while (!copy.empty()) {
            HeapItem item = copy.removeTop();

            cout << position++ << ". Complaint ID: "
                 << item.complaintID
                 << " | Priority: " << item.priority << '\n';
        }
    }
};

// Emergency Management
class EmergencyManager {
public:
    void explainRule() const {
        cout << "\n--- EMERGENCY DETECTION RULE ---\n";
        cout << "1. Severity is 9 or 10 OR\n";
        cout << "2. Severity is at least 7 AND\n";
        cout << "   at least 50 people are affected.\n";
    }

    void showEmergencies(const vector<Complaint> &complaints) const {
        bool found = false;

        cout << "\n--- ACTIVE EMERGENCY COMPLAINTS ---\n";

        for (const Complaint &c : complaints) {
            if (c.isEmergency() && c.status != "Resolved") {
                cout << "\nComplaint ID: " << c.id;
                cout << "\nCategory: " << c.category;
                cout << "\nLocation: " << c.location;
                cout << "\nSeverity: " << c.severity;
                cout << "\nPeople affected: " << c.affectedPeople;
                cout << "\nPriority: " << c.priority << '\n';

                found = true;
            }
        }

        if (!found)
            cout << "No active emergency complaints found.\n";
    }
};

// Input validation
int readInt(string message, int minimum, int maximum) {
    int value;

    while (true) {
        cout << message;

        if (cin >> value) {
            if (value >= minimum && value <= maximum)
                return value;

            cout << "Enter a value between "
                 << minimum << " and " << maximum << ".\n";
        }
        else {
            cout << "Invalid input. Enter an integer.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Add complaint
void addComplaint(vector<Complaint> &complaints) {
    Complaint c;

    c.id = readInt("Enter complaint ID: ", 1, 1000000);

    for (const Complaint &existing : complaints) {
        if (existing.id == c.id) {
            cout << "Complaint ID already exists.\n";
            return;
        }
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Enter category: ";
    getline(cin, c.category);

    cout << "Enter campus location: ";
    getline(cin, c.location);

    if (c.category.empty() || c.location.empty()) {
        cout << "Category and location cannot be empty.\n";
        return;
    }

    c.severity = readInt("Enter severity (1-10): ", 1, 10);

    c.affectedPeople = readInt(
        "Enter number of affected people: ", 1, 1000000);

    c.daysPending = readInt("Enter days pending: ", 0, 1000000);

    c.status = "Pending";
    c.calculatePriority();

    complaints.push_back(c);

    cout << "\nComplaint added successfully!\n";
    cout << "Priority score: " << c.priority << '\n';

    if (c.isEmergency())
        cout << "EMERGENCY DETECTED!\n";
    else
        cout << "Normal complaint.\n";
}

// Rebuild priority heap
void rebuildPriority(const vector<Complaint> &complaints,
                     PriorityManager &priority) {
    priority.clear();

    for (const Complaint &c : complaints) {
        if (c.status != "Resolved")
            priority.insert(c.id, c.priority);
    }
}

// Display all complaints
void displayAll(const vector<Complaint> &complaints) {
    if (complaints.empty()) {
        cout << "\nNo complaints available.\n";
        return;
    }

    cout << "\n--- ALL COMPLAINTS ---\n";

    for (const Complaint &c : complaints) {
        cout << "\nID: " << c.id;
        cout << " | Category: " << c.category;
        cout << " | Severity: " << c.severity;
        cout << " | Affected People: " << c.affectedPeople;
        cout << " | Days Pending: " << c.daysPending;
        cout << " | Priority: " << c.priority;
        cout << " | Status: " << c.status << '\n';
    }
}

// Main function
int main() {
    vector<Complaint> complaints;

    PriorityManager priority;
    EmergencyManager emergency;

    int choice;

    try {
        while (true) {
            cout << "\n========== CAMPUSFLOW MODULE ==========\n";
            cout << "1. Add Complaint\n";
            cout << "2. Display All Complaints\n";
            cout << "3. Show Priority Order\n";
            cout << "4. Show Emergency Complaints\n";
            cout << "5. View Highest Priority Complaint\n";
            cout << "6. Remove Highest Priority from Heap\n";
            cout << "7. Explain Emergency Rules\n";
            cout << "8. Exit\n";

            choice = readInt("Enter choice: ", 1, 8);

            try {
                if (choice == 1) {
                    addComplaint(complaints);
                }
                else if (choice == 2) {
                    displayAll(complaints);
                }
                else if (choice == 3) {
                    rebuildPriority(complaints, priority);
                    priority.display();
                }
                else if (choice == 4) {
                    emergency.showEmergencies(complaints);
                }
                else if (choice == 5) {
                    rebuildPriority(complaints, priority);

                    HeapItem item = priority.top();

                    cout << "\nHighest Priority Complaint: "
                         << item.complaintID
                         << " | Priority: " << item.priority << '\n';
                }
                else if (choice == 6) {
                    rebuildPriority(complaints, priority);

                    HeapItem item = priority.removeTop();

                    cout << "\nRemoved from heap: Complaint #"
                         << item.complaintID
                         << " | Priority: " << item.priority << '\n';

                    cout << "Original complaint record is still stored.\n";
                }
                else if (choice == 7) {
                    emergency.explainRule();
                }
                else if (choice == 8) {
                    cout << "Exiting module.\n";
                    break;
                }
            }
            catch (const underflow_error &e) {
                cout << "Heap Error: " << e.what() << '\n';
            }
            catch (const invalid_argument &e) {
                cout << "Invalid Data: " << e.what() << '\n';
            }
            catch (const exception &e) {
                cout << "Error: " << e.what() << '\n';
            }
        }
    }
    catch (const exception &e) {
        cout << "Application Error: " << e.what() << '\n';
    }

    return 0;
}
