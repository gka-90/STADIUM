#ifndef ACTION_H
#define ACTION_H
#include <string>
using namespace std;

// This class keeps track of actions like booking or canceling a seat
class Action {
public:
    // Constructor: Creates an action with type (B for book, C for cancel), row, column, and customer name
    Action(char type, int row, int col, const string& customer_name);
    
    // Returns the type of action (B or C)
    char get_type() const;
    
    // Returns the row number of the seat
    int get_row() const;
    
    // Returns the column number of the seat
    int get_col() const;
    
    // Returns the customer's name
    string get_customer_name() const;

private:
    char _type; // Stores action type (B or C)
    int _row; // Stores seat row
    int _col; // Stores seat column
    string _customer_name; // Stores customer name
};

#endif