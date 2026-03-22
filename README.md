# Stadium Booking System

##
This project is a **Campus Stadium Booking System** built with **C++ (Crow framework + SQLite)** on the backend and a **HTML/JavaScript/Bootstrap** frontend.

The system allows:
- **Admins** to create new stadiums with defined rows and columns of seats.
- **Users** to view availability, book seats, cancel bookings, and undo their last action.
- **Data persistence** using SQLite, so stadiums and bookings are stored even after restarting.

It’s a complete example of combining a **modern C++ web API** with a simple browser-based interface.

---

## Technologies Used
- **C++17**
  - **Crow**: Lightweight web framework providing routing, JSON, and HTTP server.
  - **SQLite3**: Embedded SQL database storing stadiums, bookings, and undo actions.
- **HTML5 / CSS3 / Bootstrap 5**: UI layout, responsive forms, and seat grid.
- **JavaScript (Vanilla)**: Fetch API for asynchronous requests (book/cancel/undo).
- **Threads + Asio**: Crow uses Asio networking under the hood to handle concurrent requests.

---

## Database (SQLite3)
Three tables are used:
- `stadiums (name, rows, cols)` → Defines stadiums.
- `bookings (stadium, row, col, customer)` → Stores each reserved seat.
- `actions (stadium, type, row, col, customer)` → Records each booking/cancellation for the undo feature.

Indexes are created on `bookings` and `actions` for faster lookups.

---

## Web API (Crow)
Crow provides simple routes such as:
- `GET /stadiums` → List all stadiums.
- `POST /create_stadium` → Create a new stadium (JSON body).
- `GET /availability/<stadium>` → Return seat grid as JSON (`"A"` for available, `"B"` for booked).
- `POST /book/<stadium>` → Book a seat (row, col, customer).
- `POST /cancel/<stadium>` → Cancel a booking.
- `POST /undo/<stadium>` → Undo the last action.

Crow handles JSON serialization/deserialization, runs a multithreaded HTTP server, and uses Asio for networking.

---

## Frontend (HTML + JavaScript)
- **`index.html`** (in `/public`) is served at `/`.
- Admin can create stadiums via a form.
- A dropdown + load button shows seat availability as a grid:
  - Green = Available (`A`)
  - Red = Booked (`B`)
- Booking/cancel forms send `fetch()` requests to the backend.
- Undo button reverts the last action.
- A status bar (Bootstrap-styled) shows messages without using intrusive alerts.

---

## Usage
1. **Build & Run**:
   cd build
   cmake ...
   ./stadium_server

2. **Open UI**: [http://localhost:8080](http://localhost:8080) in a browser.
3. **Try it out**:
   - Create a stadium.
   - Load its availability.
   - Book, cancel, or undo actions.

---

##  Project Structure
```
Stadium_Booking/
│── src/
│   ├── Main.cpp         # Crow server & routes
│   ├── Stadia.cpp/h     # Database logic (SQLite)
│   ├── Bookings.cpp/h   # Booking model
│   ├── Actions.cpp/h    # Action (for undo)
│   └── crow.h           # Crow framework (amalgamated header)
│
│── public/
│   └── index.html       # Frontend UI
│
│── CMakeLists.txt       # Build system
│── README.md            # Documentation (this file)
```

---

## License & Copyright
© 2025 Gideon Kobea.
All rights reserved.

This project is shared **for viewing and educational purposes only**.
- You may **view, run, and study** the code.
- You may **not copy, redistribute, or use** it in commercial or academic submissions without explicit permission.
