#include "Actions.h"

// Constructor: Sets up an action with type, row, column, and customer name
Action::Action(char type, int row, int col, const string& customer_name) {
    _type = type;
    _row = row;
    _col = col;
    _customer_name = customer_name;
}

// Returns the type of action (B for book, C for cancel)
char Action::get_type() const {
    return _type;
}

// Returns the row number of the seat
int Action::get_row() const {
    return _row;
}

// Returns the column number of the seat
int Action::get_col() const {
    return _col;
}

// Returns the customer's name
string Action::get_customer_name() const {
    return _customer_name;
}