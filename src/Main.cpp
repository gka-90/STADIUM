#include "crow.h"
#include "Stadia.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static void serve_file(const std::string& path, crow::response& res) {
    auto set_content_type = [&](const std::string& p) {
        if (p.size() >= 5 && p.substr(p.size()-5) == ".html") res.set_header("Content-Type", "text/html");
        else if (p.size() >= 4 && p.substr(p.size()-4) == ".css") res.set_header("Content-Type", "text/css");
        else if (p.size() >= 3 && p.substr(p.size()-3) == ".js") res.set_header("Content-Type", "application/javascript");
        else if (p.size() >= 4 && p.substr(p.size()-4) == ".png") res.set_header("Content-Type", "image/png");
        else if (p.size() >= 4 && p.substr(p.size()-4) == ".jpg") res.set_header("Content-Type", "image/jpeg");
        else if (p.size() >= 5 && p.substr(p.size()-5) == ".jpeg") res.set_header("Content-Type", "image/jpeg");
        else if (p.size() >= 4 && p.substr(p.size()-4) == ".svg") res.set_header("Content-Type", "image/svg+xml");
        else if (p.size() >= 4 && p.substr(p.size()-4) == ".ico") res.set_header("Content-Type", "image/x-icon");
        else res.set_header("Content-Type", "text/plain");
    };

    std::string public_path = "public/" + path;
    std::ifstream file(public_path, std::ios::binary);
    if (!file.good()) {
        file = std::ifstream(path, std::ios::binary);
    }
    if (file.good()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.body = buffer.str();
        set_content_type(path);
        res.code = 200;
        res.end();
        return;
    }
    res.code = 404;
    res.body = "File not found";
    res.end();
}

int main() {
    crow::SimpleApp app;
    Stadium stadium;

    // Serve index
    CROW_ROUTE(app, "/")([](const crow::request&, crow::response& res){
        serve_file("index.html", res);
    });

    // API: stadium list
    CROW_ROUTE(app, "/stadiums")([&stadium](const crow::request&, crow::response& res) {
        auto names = stadium.list_stadiums();
        crow::json::wvalue obj;
        std::vector<crow::json::wvalue> arr;
        arr.reserve(names.size());
        for (const auto& s : names) arr.emplace_back(s);
        obj["stadiums"] = std::move(arr);

        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(obj.dump());
        res.end();
    });

    // API: create stadium
    CROW_ROUTE(app, "/create_stadium").methods(crow::HTTPMethod::Post)
    ([&stadium](const crow::request& req, crow::response& res) {
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400; res.write("Invalid JSON"); res.end(); return;
        }
        std::string name = body["name"].s();
        int rows = body["rows"].i();
        int cols = body["cols"].i();
        if (name.empty() || rows <= 0 || cols <= 0) {
            res.code = 400; res.write("Invalid input"); res.end(); return;
        }
        stadium.create_stadium(name, rows, cols);
        res.code = 200; res.write("Stadium created"); res.end();
    });

    // API: availability
    CROW_ROUTE(app, "/availability/<string>")
    ([&stadium](const crow::request&, crow::response& res, std::string stadium_name) {
        if (!stadium.load_stadium(stadium_name)) {
            res.code = 404; res.write("Stadium not found"); res.end(); return;
        }
        auto seats = stadium.get_seats();

        std::vector<crow::json::wvalue> outer;
        outer.reserve(seats.size());
        for (const auto& row : seats) {
            std::vector<crow::json::wvalue> inner;
            inner.reserve(row.size());
            for (char seat : row) {
                inner.emplace_back(std::string(1, seat));
            }
            outer.emplace_back(std::move(inner));
        }

        crow::json::wvalue payload(outer);
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(payload.dump());
        res.end();
    });

    // API: book
    CROW_ROUTE(app, "/book/<string>").methods(crow::HTTPMethod::Post)
    ([&stadium](const crow::request& req, crow::response& res, std::string stadium_name) {
        if (!stadium.load_stadium(stadium_name)) {
            res.code = 404; res.write("Stadium not found"); res.end(); return;
        }
        auto body = crow::json::load(req.body);
        if (!body) { res.code = 400; res.write("Invalid JSON"); res.end(); return; }

        int row = body["row"].i();
        int col = body["col"].i();
        std::string customer = body["customer"].s();

        if (stadium.book_ticket_with_undo(row, col, customer)) {
            res.code = 200; res.write("Booked"); res.end(); return;
        }
        res.code = 400; res.write("Booking failed"); res.end();
    });

    // API: cancel
    CROW_ROUTE(app, "/cancel/<string>").methods(crow::HTTPMethod::Post)
    ([&stadium](const crow::request& req, crow::response& res, std::string stadium_name) {
        if (!stadium.load_stadium(stadium_name)) {
            res.code = 404; res.write("Stadium not found"); res.end(); return;
        }
        auto body = crow::json::load(req.body);
        if (!body) { res.code = 400; res.write("Invalid JSON"); res.end(); return; }

        int row = body["row"].i();
        int col = body["col"].i();

        if (stadium.cancel_booking_with_undo(row, col)) {
            res.code = 200; res.write("Cancelled"); res.end(); return;
        }
        res.code = 400; res.write("Cancellation failed"); res.end();
    });

    // API: undo
    CROW_ROUTE(app, "/undo/<string>").methods(crow::HTTPMethod::Post)
    ([&stadium](const crow::request&, crow::response& res, std::string stadium_name) {
        if (!stadium.load_stadium(stadium_name)) {
            res.code = 404; res.write("Stadium not found"); res.end(); return;
        }
        stadium.undo_last_action();
        res.code = 200; res.write("Undo successful"); res.end();
    });

    app.port(8080).multithreaded().run();
    return 0;
}
