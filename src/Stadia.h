#ifndef STADIUM_H
#define STADIUM_H
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <sqlite3.h>
#include "Bookings.h"
#include "Actions.h"
using namespace std;

// Manages stadiums, seats, bookings, and actions
class Stadium {
public:
    Stadium();
    ~Stadium();

    void create_stadium(const string& name, int rows, int cols);
    vector<string> list_stadiums();
    bool load_stadium(const string& stadium_name);
    bool book_ticket(int row, int col, const string& customer_name);
    bool cancel_booking(int row, int col);
    vector<vector<char>> get_seats() const;

    bool book_ticket_with_undo(int row, int col, const string& customer_name);
    bool cancel_booking_with_undo(int row, int col);
    void undo_last_action();

private:
    sqlite3* _db = nullptr;
    string _current_stadium;
    int _rows = 0;
    int _cols = 0;
    vector<vector<char>> _seats;
    vector<Booking> _booked_tickets;
    stack<Action> _action_history;
    map<string, pair<int, int>> _stadium_dims;
};

#endif
