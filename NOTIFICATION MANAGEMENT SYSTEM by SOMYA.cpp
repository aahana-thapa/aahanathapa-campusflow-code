/*THIS MODULE HAS BEEN BUILT BY (ME)SOMYA DHOUNDIYAL (TEAM MEMBER-2) TO DEMONSTRATE THE WORKING OF
NOTIFICATION MANAGEMENT SYSTEM IN THE CAMPUSFLOW PROJECT DEVELOPED IN C++
USING THE CONCEPTS OF OOPS AND DATA STRUCTURES*/

//File Location-C:\Users\HP\Desktop\Somya Dhoundiyal\PBL Module

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <limits>
#include <ctime>
#include <stdexcept>
using namespace std;

const string NOTIFICATION_FILE = "notifications.txt";

// Get current date and time
string currentDateTime() {
    time_t now = time(0);
    tm *local = localtime(&now);

    if (local == nullptr)
        throw runtime_error("Unable to get current date and time.");

    stringstream ss;
    ss << local->tm_mday << "/"
       << local->tm_mon + 1 << "/"
       << local->tm_year + 1900 << " "
       << local->tm_hour << ":"
       << local->tm_min << ":"
       << local->tm_sec;

    return ss.str();
}

// Replace delimiter from text fields
string safeField(string value) {
    for (char &ch : value) {
        if (ch == '|')
            ch = '/';
    }
    return value;
}

// Split a saved notification record
vector<string> split(const string &line) {
    vector<string> fields;
    string field;
    stringstream ss(line);

    while (getline(ss, field, '|'))
        fields.push_back(field);

    return fields;
}

// Notification data structure
struct Notification {
    string recipientID;
    string message;
    string time;
    bool read = false;
};

// Notification Management System
class NotificationManager {
    queue<Notification> messages;
    vector<Notification> archive;

public:
    // Add notification
    void add(const string &recipient, const string &message) {
        if (recipient.empty() || message.empty())
            throw invalid_argument("Recipient and message cannot be empty.");

        Notification n{recipient, message, currentDateTime(), false};

        messages.push(n);
        archive.push_back(n);

        cout << "Notification added successfully.\n";
    }

    // Send notification to everyone
    void notifyAll(const string &message) {
        add("ALL", message);
    }

    // Display notifications for a particular user
    void showFor(const string &userID,
                 bool administrator = false) const {
        if (userID.empty() && !administrator) {
            cout << "Invalid user ID.\n";
            return;
        }

        bool found = false;

        cout << "\n--- Notifications ---\n";

        for (const Notification &n : archive) {
            if (n.recipientID == userID ||
                n.recipientID == "ALL" ||
                administrator) {

                cout << (n.read ? "[Read] " : "[New] ")
                     << n.time << " | "
                     << n.message << '\n';

                found = true;
            }
        }

        if (!found)
            cout << "No notifications available.\n";
    }

    // Mark notifications as read
    void markRead(const string &userID) {
        if (userID.empty()) {
            cout << "Invalid user ID.\n";
            return;
        }

        bool changed = false;

        for (Notification &n : archive) {
            if (n.recipientID == userID ||
                n.recipientID == "ALL") {

                n.read = true;
                changed = true;
            }
        }

        if (changed)
            cout << "Your notifications have been marked as read.\n";
        else
            cout << "No notifications to mark as read.\n";
    }

    // Display pending notification queue
    void showQueue() const {
        if (messages.empty()) {
            cout << "Notification queue is empty.\n";
            return;
        }

        queue<Notification> copy = messages;

        cout << "\n--- Notification Queue (oldest first) ---\n";

        while (!copy.empty()) {
            const Notification &n = copy.front();

            cout << n.time << " | "
                 << n.recipientID << " | "
                 << n.message << '\n';

            copy.pop();
        }
    }

    // Clear pending queue
    void clearQueue() {
        while (!messages.empty())
            messages.pop();

        cout << "Pending notification queue cleared.\n";
        cout << "Archived notifications remain saved.\n";
    }

    // Save notification history to file
    void save() const {
        ofstream out(NOTIFICATION_FILE);

        if (!out)
            throw runtime_error("Unable to save notifications.");

        for (const Notification &n : archive) {
            out << safeField(n.recipientID) << '|'
                << safeField(n.time) << '|'
                << (n.read ? 1 : 0) << '|'
                << safeField(n.message) << '\n';
        }

        if (!out)
            throw runtime_error("Error while writing notification file.");

        cout << "Notifications saved successfully.\n";
    }

    // Load notifications from file
    void load() {
        ifstream in(NOTIFICATION_FILE);

        if (!in) {
            cout << "No previous notification file found.\n";
            return;
        }

        string line;
        int count = 0;

        while (getline(in, line)) {
            vector<string> f = split(line);

            if (f.size() != 4)
                continue;

            try {
                Notification n;

                n.recipientID = f[0];
                n.time = f[1];
                n.read = f[2] == "1";
                n.message = f[3];

                if (n.recipientID.empty() || n.message.empty())
                    continue;

                archive.push_back(n);

                if (!n.read)
                    messages.push(n);

                count++;
            }
            catch (const exception &) {
                continue;
            }
        }

        cout << count << " notifications loaded successfully.\n";
    }
};

// Input validation
int readInt(string message, int minimum, int maximum) {
    int choice;

    while (true) {
        cout << message;

        if (cin >> choice) {
            if (choice >= minimum && choice <= maximum)
                return choice;

            cout << "Enter a valid option between "
                 << minimum << " and " << maximum << ".\n";
        }
        else {
            cout << "Invalid input. Please enter a number.\n";

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// Read a complete line
string readLine(string message) {
    string value;

    cout << message;
    getline(cin, value);

    return value;
}

// Main function
int main() {
    NotificationManager notifications;

    try {
        notifications.load();

        while (true) {
            cout << "\n========== NOTIFICATION MANAGEMENT ==========\n";
            cout << "1. Add Notification\n";
            cout << "2. Notify Everyone\n";
            cout << "3. View User Notifications\n";
            cout << "4. Mark Notifications as Read\n";
            cout << "5. View Pending Notification Queue\n";
            cout << "6. Clear Pending Queue\n";
            cout << "7. Save Notifications\n";
            cout << "8. Exit\n";

            int choice = readInt("Enter your choice: ", 1, 8);

            try {
                if (choice == 1) {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    string id = readLine("Enter recipient ID: ");
                    string message = readLine("Enter message: ");

                    notifications.add(id, message);
                }

                else if (choice == 2) {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    string message = readLine("Enter message: ");

                    notifications.notifyAll(message);
                }

                else if (choice == 3) {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    string id = readLine("Enter user ID: ");

                    notifications.showFor(id);
                }

                else if (choice == 4) {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    string id = readLine("Enter user ID: ");

                    notifications.markRead(id);
                }

                else if (choice == 5) {
                    notifications.showQueue();
                }

                else if (choice == 6) {
                    notifications.clearQueue();
                }

                else if (choice == 7) {
                    notifications.save();
                }

                else if (choice == 8) {
                    notifications.save();

                    cout << "Exiting Notification Management System.\n";
                    break;
                }
            }
            catch (const invalid_argument &e) {
                cout << "Input Error: " << e.what() << '\n';
            }
            catch (const exception &e) {
                cout << "Operation Error: " << e.what() << '\n';
            }
        }
    }
    catch (const exception &e) {
        cerr << "System Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
