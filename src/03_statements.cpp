//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"

int main() {

  try {
    // Create a connection
    auto connection = sqlite::Connection::memory();

    // Method 1
    std::string param2{"World"};
    sqlite::Statement statement{connection, "select ?1 union all select ?2", "hello", param2};

    // Method 2
    //    statement.prepare(connection, "select ?1 union all select ?2", "hello", param2);
    //    statement.bind(1, std::string("Hello")); // Calls rvalue ref
    //    statement.bind(2, param2);               // Calls lvalue ref

    // Method 3
    //    statement.bind_all(std::string("Hello"), param2); // Using variadic templates

    for (const auto &row : statement) {
      std::cout << statement.get_string(0) << std::endl;
    }

    // Method 4 - Direct execute

    //    sqlite::execute(connection, "CREATE TABLE Users (Name)");
    //    sqlite::execute(connection, "insert into Users values (?)", "Joe");
    //    sqlite::execute(connection, "insert into Users values (?)", "Beth");
    //    for (auto row : sqlite::Statement(connection, "select Name from Users")) {
    //      std::cout << statement.get_string() << std::endl;
    //    }
  }

  catch (const sqlite::Exception &e) {
    std::cerr << e.message.c_str() << "(" << e.result << ")" << std::endl;
  }
}
