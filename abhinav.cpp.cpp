
// MODULE 7: CAMPUS GRAPH AND ROUTE FINDING
// Work: Represents campus locations as vertices and roads
// as weighted edges. Demonstrates BFS, DFS and Dijkstra's algorithm.

class CampusGraph {
private:
    vector<vector<pair<int,int>>> graph;

public:
    CampusGraph(int n) {
        graph.resize(n);
    }

    void addRoad(int u,int v,int weight) {
        if(u<0||v<0||u>=graph.size()||v>=graph.size()) {
            cout<<"Invalid location number. Road not added.\n";
            return;
        }

        graph[u].push_back({v,weight});
        graph[v].push_back({u,weight});
    }

    void BFS(int start) {
        if(start<0||start>=graph.size()) {
            cout<<"Invalid starting location.\n";
            return;
        }

        vector<bool> visited(graph.size(),false);
        queue<int> q;

        visited[start]=true;
        q.push(start);

        cout<<"BFS Traversal: ";

        while(!q.empty()) {
            int u=q.front();
            q.pop();

            cout<<u<<" ";

            for(auto road:graph[u]) {
                int v=road.first;

                if(!visited[v]) {
                    visited[v]=true;
                    q.push(v);
                }
            }
        }

        cout<<endl;
    }

    void DFSHelper(int u,vector<bool>& visited) {
        visited[u]=true;
        cout<<u<<" ";

        for(auto road:graph[u]) {
            int v=road.first;

            if(!visited[v])
                DFSHelper(v,visited);
        }
    }

    void DFS(int start) {
        if(start<0||start>=graph.size()) {
            cout<<"Invalid starting location.\n";
            return;
        }

        vector<bool> visited(graph.size(),false);

        cout<<"DFS Traversal: ";
        DFSHelper(start,visited);
        cout<<endl;
    }

    void shortestPaths(int start) {
        if(start<0||start>=graph.size()) {
            cout<<"Invalid starting location.\n";
            return;
        }

        const int INF=numeric_limits<int>::max();
        vector<int> dist(graph.size(),INF);

        priority_queue<
            pair<int,int>,
            vector<pair<int,int>>,
            greater<pair<int,int>>
        > pq;

        dist[start]=0;
        pq.push({0,start});

        while(!pq.empty()) {
            int distance=pq.top().first;
            int u=pq.top().second;
            pq.pop();

            if(distance!=dist[u])
                continue;

            for(auto road:graph[u]) {
                int v=road.first;
                int weight=road.second;

                if(dist[u]<=INF-weight&&
                   dist[u]+weight<dist[v]) {
                    dist[v]=dist[u]+weight;
                    pq.push({dist[v],v});
                }
            }
        }

        cout<<"Shortest Distances from Location "<<start<<":\n";

        for(int i=0;i<dist.size();i++) {
            cout<<"Location "<<i<<": ";

            if(dist[i]==INF)
                cout<<"Unreachable";
            else
                cout<<dist[i];

            cout<<endl;
        }
    }
};


// MAIN PROGRAM: INTEGRATES THE IMPLEMENTED MODULES

int main() {
    ComplaintManager manager;
    PriorityManager priorityManager;
    HistoryManager historyManager;

    int choice;

    do {
        cout<<"\n========== CAMPUSFLOW ==========\n";
        cout<<"1. Register Complaint\n";
        cout<<"2. Display All Complaints\n";
        cout<<"3. Search Complaint by ID\n";
        cout<<"4. Arrange Complaints by Priority\n";
        cout<<"5. Update Complaint Status\n";
        cout<<"6. Undo Last Status Change\n";
        cout<<"7. Demonstrate Campus Graph\n";
        cout<<"0. Exit\n";
        cout<<"Enter Your Choice: ";

        cin>>choice;

        if(cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(),'\n');
            cout<<"Please enter a valid menu number.\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(),'\n');

        if(choice==1) {
            manager.addComplaint();
        }

        else if(choice==2) {
            manager.showComplaints();
        }

        else if(choice==3) {
            int id;

            cout<<"Enter Complaint ID: ";
            cin>>id;
            cin.ignore(numeric_limits<streamsize>::max(),'\n');

            Complaint* c=manager.findComplaint(id);

            if(c!=NULL) {
                cout<<"\nComplaint Found!\n";
                cout<<"Student Name: "<<c->studentName<<endl;
                cout<<"Category: "<<c->category<<endl;
                cout<<"Location: "<<c->location<<endl;
                cout<<"Complaint Date: "<<c->date<<endl;
                cout<<"Status: "<<c->status<<endl;
                cout<<"Priority Score: "<<c->priority<<endl;
            } else {
                cout<<"Complaint ID not found.\n";
            }
        }

        else if(choice==4) {
            // Rebuild the heap using all current complaints.
            // Displaying priority order removes heap entries,
            // but the original complaints remain in the linked list.

            manager.loadPriorities(priorityManager);
            priorityManager.showPriorityOrder();
        }

        else if(choice==5) {
            int id;
            string status;

            cout<<"Enter Complaint ID: ";
            cin>>id;
            cin.ignore(numeric_limits<streamsize>::max(),'\n');

            cout<<"Enter New Status (Pending/In Progress/Resolved): ";
            getline(cin,status);

            historyManager.updateStatus(manager,id,status);
        }

        else if(choice==6) {
            historyManager.undo(manager);
        }

        else if(choice==7) {
            int n;

            cout<<"Enter Number of Campus Locations (1-100): ";
            cin>>n;

            if(cin.fail()||n<=0||n>100) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(),'\n');
                cout<<"Invalid number of locations.\n";
                continue;
            }

            CampusGraph campus(n);

            int roads;
            cout<<"Enter Number of Roads: ";
            cin>>roads;

            if(cin.fail()||roads<0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(),'\n');
                cout<<"Invalid number of roads.\n";
                continue;
            }

            for(int i=0;i<roads;i++) {
                int u,v,w;

                cout<<"\nEnter Starting Location Number: ";
                cin>>u;

                cout<<"Enter Ending Location Number: ";
                cin>>v;

                cout<<"Enter Road Distance: ";
                cin>>w;

                if(cin.fail()||w<0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(),'\n');
                    cout<<"Invalid road details. Road not added.\n";
                    continue;
                }

                campus.addRoad(u,v,w);
            }

            int start;

            cout<<"\nEnter Starting Location Number: ";
            cin>>start;
            cin.ignore(numeric_limits<streamsize>::max(),'\n');

            campus.BFS(start);
            campus.DFS(start);
            campus.shortestPaths(start);
        }

        else if(choice==0) {
            cout<<"Exiting CampusFlow. Goodbye!\n";
        }

        else {
            cout<<"Invalid choice. Please try again.\n";
        }

    } while(choice!=0);

    return 0;
}