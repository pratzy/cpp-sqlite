//
// Created by Prateek Agarwal on 13/6/20.
//
#pragma once

#include "handle.h"
#include <iostream>
#include <sqlite3.h>
#include <string>

#ifdef _DEBUG
#define VERIFY ASSERT
#define VERIFY_(result, expression) ASSERT(result == expression)
#else
#define VERIFY(expression) (expression)
#define VERIFY_(result, expression) (expression)
#endif

namespace sqlite {

struct Exception {
  int result = 0;
  std::string message;

  explicit Exception(sqlite3 *const connection)
    : result(std::move(sqlite3_extended_errcode(connection))), message(std::move(sqlite3_errmsg(connection))) {}
};

class Connection {
  struct ConnectionHandleTraits : HandleTraits<sqlite3 *> {
    static void close(Type value) noexcept {
      VERIFY_(SQLITE_OK, sqlite3_close(value));
      std::cout << "Connection closed" << std::endl;
    }
  };

  using ConnectionHandle = Handle<ConnectionHandleTraits>;
  ConnectionHandle m_handle;

  template <typename F, typename C>
  void internal_open(F open, const C *const filename) {
    Connection temp;
    if (SQLITE_OK != open(filename, temp.m_handle.set())) {
      temp.throw_last_error();
    }
    swap(m_handle, temp.m_handle);
  }

public:
  Connection() noexcept = default;

  template <typename C>
  explicit Connection(const C *const filename) {
    open(filename);
  }

  static Connection memory() { return Connection(":memory:"); }

  static Connection wide_memory() { return Connection(L":memory:"); }

  explicit operator bool() const noexcept { return static_cast<bool>(m_handle); }

  sqlite3 *get_abi() const noexcept { return m_handle.get(); }

  void throw_last_error() const { throw Exception(get_abi()); }

  void open(const char *const filename) {
    internal_open(sqlite3_open, filename);
    std::cout << "Connection open: " << filename << std::endl;
  }

  void open(const wchar_t *const filename) {
    internal_open(sqlite3_open16, filename);
    std::cout << "Connection open: " << filename << std::endl;
  }
};

template <typename T>
struct Reader {
  int get_int(const int column = 0) const noexcept {
    return sqlite3_column_int(static_cast<const T *>(this)->get_abi(), column);
  }

  char const *get_string(const int column = 0) const noexcept {
    return reinterpret_cast<const char *>(sqlite3_column_text(static_cast<const T *>(this)->get_abi(), column));
  }

  wchar_t const *get_wide_string(const int column = 0) const noexcept {
    return reinterpret_cast<const wchar_t *>(sqlite3_column_text16(static_cast<const T *>(this)->get_abi(), column));
  }

  int get_string_length(const int column = 0) const noexcept {
    return sqlite3_column_bytes(static_cast<T const *>(this)->get_abi(), column);
  }

  int get_wide_string_length(const int column = 0) const noexcept {
    return sqlite3_column_bytes16(static_cast<T const *>(this)->get_abi(), column) / sizeof(wchar_t);
  }
};

class Row : public Reader<Row> {
  sqlite3_stmt *m_statement = nullptr;

public:
  sqlite3_stmt *get_abi() const noexcept { return m_statement; }

  Row(sqlite3_stmt *const statement) noexcept : m_statement(statement) {}
};

class Statement : public Reader<Statement> {
  struct StatementHandleTraits : HandleTraits<sqlite3_stmt *> {
    static void close(Type value) noexcept { VERIFY_(SQLITE_OK, sqlite3_finalize(value)); }
  };

  using StatementHandle = Handle<StatementHandleTraits>;
  StatementHandle m_handle;

  template <typename F, typename C, typename... Values>
  void _prepare(const Connection &connection, F prepare, const C *const text, Values &&... values) {
    assert(connection);
    auto result = prepare(connection.get_abi(), text, -1, m_handle.set(), nullptr);
    if (SQLITE_OK != result) {
      connection.throw_last_error();
    }
    bind_all(std::forward<Values>(values)...);
  }

  void _bind(const int index) const noexcept {}

  template <typename First, typename... Rest>
  void _bind(const int index, First &&first, Rest &&... rest) const {
    bind(index, std::forward<First>(first));
    _bind(index + 1, std::forward<Rest>(rest)...);
  }

public:
  Statement() noexcept = default;

  template <typename C, typename... Values>
  Statement(const Connection &conn, const C *const text, Values &&... values) {
    prepare(conn, text, std::forward<Values>(values)...);
  }

  explicit operator bool() const noexcept { return static_cast<bool>(m_handle); }

  sqlite3_stmt *get_abi() const noexcept { return m_handle.get(); }

  void throw_last_error() const { throw Exception(sqlite3_db_handle(get_abi())); }

  template <typename... Values>
  void prepare(const Connection &connection, const char *const text, Values &&... values) {
    _prepare(connection, sqlite3_prepare_v2, text, std::forward<Values>(values)...);
  }

  template <typename... Values>
  void prepare(const Connection &connection, const wchar_t *const text, Values &&... values) {
    _prepare(connection, sqlite3_prepare16_v2, text, std::forward<Values>(values)...);
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

  bool step() const {
    const auto result = sqlite3_step(get_abi());

    if (result == SQLITE_ROW)
      return true;
    if (result == SQLITE_DONE)
      return false;
    throw_last_error();
  }

#pragma GCC diagnostic pop

  void execute() const { VERIFY(!step()); }

  void bind(const int index, const int value) {
    if (SQLITE_OK != sqlite3_bind_int(get_abi(), index, value)) {
      throw_last_error();
    }
  }

  void bind(const int index, const char *const value, const int size = -1) const {
    if (SQLITE_OK != sqlite3_bind_text(get_abi(), index, value, size, SQLITE_STATIC)) {
      throw_last_error();
    }
  }

  void bind(const int index, const wchar_t *const value, const int size = -1) const {
    if (SQLITE_OK != sqlite3_bind_text16(get_abi(), index, value, size, SQLITE_STATIC)) {
      throw_last_error();
    }
  }

  void bind(const int index, const std::string &value) const { bind(index, value.c_str(), value.size()); }

  void bind(const int index, const std::wstring &value) const {
    bind(index, value.c_str(), value.size() * sizeof(wchar_t));
  }

  void bind(const int index, const std::string &&value) const {
    if (SQLITE_OK != sqlite3_bind_text(get_abi(), index, value.c_str(), value.size(), SQLITE_TRANSIENT)) {
      throw_last_error();
    }
  }

  void bind(const int index, const std::wstring &&value) const {
    if (SQLITE_OK !=
        sqlite3_bind_text16(get_abi(), index, value.c_str(), value.size() * sizeof(wchar_t), SQLITE_TRANSIENT)) {
      throw_last_error();
    }
  }

  template <typename... Values>
  void bind_all(Values &&... values) const {
    _bind(1, std::forward<Values>(values)...);
  }
};

class RowIterator {
  const Statement *m_statement = nullptr;

public:
  RowIterator() noexcept = default;

  RowIterator(const Statement &statement) noexcept {
    if (statement.step()) {
      m_statement = &statement;
    }
  }

  RowIterator &operator++() noexcept {
    if (!m_statement->step())
      m_statement = nullptr;
    return *this;
  }

  bool operator!=(const RowIterator &other) const noexcept { return m_statement != other.m_statement; }

  bool operator==(const RowIterator &other) const noexcept { return m_statement == other.m_statement; }

  Row operator*() const noexcept { return Row(m_statement->get_abi()); }
};

inline RowIterator begin(const Statement &statement) noexcept { return RowIterator(statement); }

inline RowIterator end(const Statement &) noexcept { return RowIterator(); }

template <typename C, typename... Values>
void execute(const Connection &connection, const C *const text, Values &&... values) {
  Statement(connection, text, std::forward<Values>(values)...);
}

} // namespace sqlite
