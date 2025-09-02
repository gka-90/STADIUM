#include "Bookings.h"

// Constructor: Sets up a booking with row, column, and customer name
Booking::Booking(int row, int col, const string& customer_name) {
    _row = row;
    _col = col;
    _customer_name = customer_name;
}

// Returns the row number of the booked seat
int Booking::get_row() const {
    return _row;
}

// Returns the column number of the booked seat
int Booking::get_col() const {
    return _col;
}

// Returns the customer's name for the booking
string Booking::get_customer_name() const {
    return _customer_name;
}