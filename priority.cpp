#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
using namespace std;
class Complaint {
public:
int id;
int severity;
int affectedPeople;
int daysPending;
int priority;
Complaint(int i, int s, int a, int d) {
id = i;
severity = s;
affectedPeople = a;
daysPending = d;
priority = 0;
}
void calculatePriority() {
priority = severity * 20 + min(affectedPeople, 100) + min(daysPending, 30);
}
};
struct HeapItem {
int id;
int priority;
};
class PriorityManager {
private:
vector<HeapItem> heap;
void heapifyUp(int i) {
while (i > 0) {
int parent = (i - 1) / 2;
if (heap[i].priority > heap[parent].priority) {
swap(heap[i], heap[parent]);
i = parent;
} else {
break;
}
}
}
void heapifyDown(int i) {
int n = heap.size();
while (true) {
int largest = i;
int left = 2 * i + 1;
int right = 2 * i + 2;
if (left < n && heap[left].priority > heap[largest].priority) {
largest = left;
}
if (right < n && heap[right].priority > heap[largest].priority) {
largest = right;
}
if (largest == i)
break;
swap(heap[i], heap[largest]);
i = largest;
}
}
public:
void clear() {
heap.clear();
}
void insert(int id, int priority) {
HeapItem item;
item.id = id;
item.priority = priority;
heap.push_back(item);
heapifyUp(heap.size() - 1);
}
bool empty() {
return heap.empty();
}
HeapItem top() {
return heap[0];
}
void removeTop() {
if (heap.empty())
return;
if (heap.size() == 1) {
heap.pop_back();
return;
}
heap[0] = heap.back();
heap.pop_back();
heapifyDown(0);
}
void showPriorityOrder() {
if (heap.empty()) {
cout << "\nNo complaints available.\n";
return;
}
cout << "\n===== COMPLAINT PRIORITY ORDER =====\n";
int rank = 1;
while (!heap.empty()) {
HeapItem item = top();
cout << rank++ << ". Complaint ID: " << item.id << " | Priority: " << item.priority << endl;
removeTop();
}
}
};
void loadPriorities(Complaint complaints[], int n, PriorityManager& pm) {
pm.clear();
for (int i = 0; i < n; i++) {
complaints[i].calculatePriority();
pm.insert(complaints[i].id, complaints[i].priority);
}
}
int main() {
Complaint complaints[] = {
Complaint(1001, 8, 40, 5),
Complaint(1002, 5, 20, 2),
Complaint(1003, 9, 60, 10)
};
int n = 3;
PriorityManager pm;
loadPriorities(complaints, n, pm);
pm.showPriorityOrder();
return 0;
}
