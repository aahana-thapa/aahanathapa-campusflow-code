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