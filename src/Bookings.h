#ifndef BOOKING_H
#define BOOKING_H
#include <string>
using namespace std;

// This class stores information about a booked seat
class Booking {
public:
    // Constructor: Creates a booking with row, column, and customer name
    Booking(int row, int col, const string& customer_name);
    
    // Returns the row number of the booked seat
    int get_row() const;
    
    // Returns the column number of the booked seat
    int get_col() const;
    
    // Returns the customer's name for the booking
    string get_customer_name() const;

private:
    int _row; // Stores seat row
    int _col; // Stores seat column
    string _customer_name; // Stores customer name
};

#endif