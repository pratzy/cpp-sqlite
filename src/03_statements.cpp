//
// Created by Prateek Agarwal on 13/6/20.
//

#include "sqlite.h"
#include <cstdio>

int main() {

  try {
    // Create a connection
    auto connection = sqlite::Connection::Memory();

    //    std::string param2{"World"};
    //    sqlite::Statement statement {connection, "select ?1 union all select ?2", "hello", param2};

    //    statement.Prepare(connection, "select ?1 union all select ?2", "hello", param2);
    //    statement.Bind(1, std::string("Hello")); // Calls rvalue ref
    //    statement.Bind(2, param2); // Calls lvalue ref

    //    statement.BindAll(std::string("Hello"), param2); // Using variadic templates

    //    for (const auto &row : statement) {
    //      fprintf(stdout, "%s\n", statement.GetString(0));
    //    }

    sqlite::Execute(connection, "CREATE TABLE Users (Name)");
    sqlite::Execute(connection, "insert into Users values (?)", "Joe");
    sqlite::Execute(connection, "insert into Users values (?)", "Beth");
    for (auto row : sqlite::Statement(connection, "select Name from Users")) {
      fprintf(stderr, "%s\n", row.GetString());
    }
  }

  catch (const sqlite::Exception &e) {
    fprintf(stderr, "%s (%d)\n", e.Message.c_str(), e.Result);
  }
}
