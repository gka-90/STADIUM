#include "Stadia.h"
#include <sqlite3.h>
#include <iostream>

Stadium::Stadium() {
    sqlite3_open("stadium.db", &_db);
    // Tables
    sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS stadiums (name TEXT PRIMARY KEY, rows INT, cols INT);", NULL, NULL, NULL);
    sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS bookings (stadium TEXT, row INT, col INT, customer TEXT);", NULL, NULL, NULL);
    sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS actions (stadium TEXT, type CHAR, row INT, col INT, customer TEXT);", NULL, NULL, NULL);
    // Indices for performance
    sqlite3_exec(_db, "CREATE INDEX IF NOT EXISTS idx_bookings_main ON bookings (stadium, row, col);", NULL, NULL, NULL);
    sqlite3_exec(_db, "CREATE INDEX IF NOT EXISTS idx_actions_stadium ON actions (stadium);", NULL, NULL, NULL);
}

Stadium::~Stadium() {
    if (_db) sqlite3_close(_db);
}

void Stadium::create_stadium(const string& name, int rows, int cols) {
    string sql = "INSERT OR REPLACE INTO stadiums (name, rows, cols) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, rows);
    sqlite3_bind_int(stmt, 3, cols);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    _stadium_dims[name] = {rows, cols};
}

vector<string> Stadium::list_stadiums() {
    vector<string> stadiums;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(_db, "SELECT name FROM stadiums;", -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        stadiums.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    return stadiums;
}

bool Stadium::load_stadium(const string& stadium_name) {
    _current_stadium = stadium_name;
    sqlite3_stmt* stmt;
    string sql = "SELECT rows, cols FROM stadiums WHERE name = ?;";
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, stadium_name.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }
    _rows = sqlite3_column_int(stmt, 0);
    _cols = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);

    // Reset in-memory state
    _seats = vector<vector<char>>(_rows, vector<char>(_cols, 'A'));
    _booked_tickets.clear();
    while (!_action_history.empty()) _action_history.pop();

    // Load bookings
    sql = "SELECT row, col, customer FROM bookings WHERE stadium = ?;";
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, stadium_name.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row = sqlite3_column_int(stmt, 0);
        int col = sqlite3_column_int(stmt, 1);
        string customer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (row >= 0 && row < _rows && col >= 0 && col < _cols) {
            _seats[row][col] = 'B';
            _booked_tickets.emplace_back(row, col, customer);
        }
    }
    sqlite3_finalize(stmt);

    // Load actions (oldest to newest so the top of stack is the last action)
    sql = "SELECT type, row, col, customer FROM actions WHERE stadium = ? ORDER BY rowid ASC;";
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, stadium_name.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char type = *reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int row = sqlite3_column_int(stmt, 1);
        int col = sqlite3_column_int(stmt, 2);
        string customer = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        _action_history.push(Action(type, row, col, customer));
    }
    sqlite3_finalize(stmt);

    return true;
}

bool Stadium::book_ticket(int row, int col, const string& customer_name) {
    if (row >= _rows || col >= _cols || row < 0 || col < 0 || _seats[row][col] != 'A') {
        return false;
    }
    _seats[row][col] = 'B';
    _booked_tickets.emplace_back(row, col, customer_name);

    string sql = "INSERT INTO bookings (stadium, row, col, customer) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, _current_stadium.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, row);
    sqlite3_bind_int(stmt, 3, col);
    sqlite3_bind_text(stmt, 4, customer_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return true;
}

bool Stadium::cancel_booking(int row, int col) {
    for (auto it = _booked_tickets.begin(); it != _booked_tickets.end(); ++it) {
        if (it->get_row() == row && it->get_col() == col) {
            if (row >= 0 && row < _rows && col >= 0 && col < _cols) {
                _seats[row][col] = 'A';
            }
            string sql = "DELETE FROM bookings WHERE stadium = ? AND row = ? AND col = ?;";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, _current_stadium.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 2, row);
            sqlite3_bind_int(stmt, 3, col);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            _booked_tickets.erase(it);
            return true;
        }
    }
    return false;
}

vector<vector<char>> Stadium::get_seats() const {
    return _seats;
}

bool Stadium::book_ticket_with_undo(int row, int col, const string& customer_name) {
    if (book_ticket(row, col, customer_name)) {
        _action_history.push(Action('B', row, col, customer_name));
        string sql = "INSERT INTO actions (stadium, type, row, col, customer) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, _current_stadium.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, "B", 1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, row);
        sqlite3_bind_int(stmt, 4, col);
        sqlite3_bind_text(stmt, 5, customer_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return true;
    }
    return false;
}

bool Stadium::cancel_booking_with_undo(int row, int col) {
    for (const auto& booking : _booked_tickets) {
        if (booking.get_row() == row && booking.get_col() == col) {
            if (cancel_booking(row, col)) {
                _action_history.push(Action('C', row, col, booking.get_customer_name()));
                string sql = "INSERT INTO actions (stadium, type, row, col, customer) VALUES (?, ?, ?, ?, ?);";
                sqlite3_stmt* stmt;
                sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
                sqlite3_bind_text(stmt, 1, _current_stadium.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, "C", 1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 3, row);
                sqlite3_bind_int(stmt, 4, col);
                sqlite3_bind_text(stmt, 5, booking.get_customer_name().c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                return true;
            }
        }
    }
    return false;
}

void Stadium::undo_last_action() {
    if (_action_history.empty()) return;
    Action last_action = _action_history.top();
    _action_history.pop();

    // Remove this action record from DB
    string sql = "DELETE FROM actions WHERE stadium = ? AND type = ? AND row = ? AND col = ? AND customer = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, NULL);
    string typeStr(1, last_action.get_type());
    sqlite3_bind_text(stmt, 1, _current_stadium.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, typeStr.c_str(), 1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, last_action.get_row());
    sqlite3_bind_int(stmt, 4, last_action.get_col());
    sqlite3_bind_text(stmt, 5, last_action.get_customer_name().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (last_action.get_type() == 'B') {
        cancel_booking(last_action.get_row(), last_action.get_col());
    } else if (last_action.get_type() == 'C') {
        book_ticket(last_action.get_row(), last_action.get_col(), last_action.get_customer_name());
    }
}
