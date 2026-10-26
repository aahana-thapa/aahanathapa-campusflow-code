#include <iostream>
#include <string>
#include <cctype>
#include <limits>
using namespace std;
class Complaint {
public:
int id;
string category;
string location;
string description;
Complaint(int i, string c, string l, string d) {
id = i;
category = c;
location = l;
description = d;
}
};
struct Node {
Complaint data;
Node* next;
Node(Complaint c) : data(c), next(NULL) {}
};
class DuplicateManager {
private:
Node* head;
int nextID;
string normalizeText(string text) {
string result;
bool previousSpace = true;
for (unsigned char ch : text) {
if (isspace(ch)) {
if (!previousSpace) {
result += ' ';
previousSpace = true;
}
} else {
result += static_cast<char>(tolower(ch));
previousSpace = false;
}
}
if (!result.empty() && result.back() == ' ')
result.pop_back();
return result;
}
bool isDuplicate(string category, string location, string description) {
Node* current = head;
while (current != NULL) {
if (normalizeText(current->data.category) == normalizeText(category) &&
normalizeText(current->data.location) == normalizeText(location) &&
normalizeText(current->data.description) == normalizeText(description)) {
return true;
}
current = current->next;
}
return false;
}
public:
DuplicateManager() {
head = NULL;
nextID = 1001;
}
~DuplicateManager() {
Node* current = head;
while (current != NULL) {
Node* temp = current;
current = current->next;
delete temp;
}
}
void addComplaint() {
string category, location, description;
cout << "\nEnter complaint category: ";
getline(cin, category);
cout << "Enter location: ";
getline(cin, location);
cout << "Enter complaint description: ";
getline(cin, description);
if (normalizeText(category).empty() ||
normalizeText(location).empty() ||
normalizeText(description).empty()) {
cout << "\nError: Fields cannot be empty.\n";
cout << "Complaint was not added.\n";
return;
}
if (isDuplicate(category, location, description)) {
cout << "\nError: Duplicate complaint detected!\n";
cout << "A complaint with the same category, location and description already exists.\n";
cout << "Complaint was not added.\n";
return;
}
Complaint c(nextID, category, location, description);
Node* newNode = new Node(c);
if (head == NULL) {
head = newNode;
} else {
Node* current = head;
while (current->next != NULL)
current = current->next;
current->next = newNode;
}
cout << "\nComplaint added successfully!\n";
cout << "Complaint ID: " << nextID << endl;
nextID++;
}
void displayComplaints() {
if (head == NULL) {
cout << "\nNo complaints available.\n";
return;
}
Node* current = head;
cout << "\n===== REGISTERED COMPLAINTS =====\n";
while (current != NULL) {
cout << "\nComplaint ID: " << current->data.id;
cout << "\nCategory: " << current->data.category;
cout << "\nLocation: " << current->data.location;
cout << "\nDescription: " << current->data.description << endl;
current = current->next;
}
}
};
int main() {
DuplicateManager manager;
int choice;
do {
cout << "\n===== DATA DUPLICATION MODULE =====\n";
cout << "1. Add Complaint\n";
cout << "2. Display Complaints\n";
cout << "3. Exit\n";
cout << "Enter your choice: ";
if (!(cin >> choice)) {
cout << "\nError: Please enter a valid number.\n";
cin.clear();
cin.ignore(numeric_limits<streamsize>::max(), '\n');
continue;
}
cin.ignore(numeric_limits<streamsize>::max(), '\n');
switch (choice) {
case 1:
manager.addComplaint();
break;
case 2:
manager.displayComplaints();
break;
case 3:
cout << "\nExiting Data Duplication Module.\n";
break;
default:
cout << "\nInvalid choice. Please try again.\n";
}
} while (choice != 3);
return 0;
}
