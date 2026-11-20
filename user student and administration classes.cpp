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