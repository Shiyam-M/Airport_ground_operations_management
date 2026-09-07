// sqlite3_min.h
// -----------------------------------------------------------------------------
// Minimal hand-written subset of the SQLite3 C API declarations.
//
// WHY THIS FILE EXISTS:
// This project links directly against the system SQLite3 shared library
// (libsqlite3.so / libsqlite3.so.0), which is present on virtually every
// Linux distribution because it is a dependency of many base packages.
// Some minimal/offline build environments do not have the "libsqlite3-dev"
// package (which ships the official sqlite3.h) installed, and have no
// package-manager/internet access available to install it.
//
// The SQLite C API/ABI is extremely stable (it has not broken across major
// versions in over a decade), so it is safe to declare only the subset of
// functions AirportDatabase actually uses and link against the real,
// official SQLite shared library at build time. This gives a REAL
// SQLite-backed persistence layer (not a mock), while remaining portable.
//
// If "libsqlite3-dev" IS available on your machine, you may instead
// `#include <sqlite3.h>` and everything in this project continues to work
// unmodified, because the declarations below are copied verbatim from the
// official SQLite C API.
// -----------------------------------------------------------------------------
#ifndef AGOMS_SQLITE3_MIN_H
#define AGOMS_SQLITE3_MIN_H

extern "C" {

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef long long sqlite3_int64;

int sqlite3_open(const char* filename, sqlite3** ppDb);
int sqlite3_close(sqlite3* db);

int sqlite3_exec(sqlite3* db, const char* sql,
                  int (*callback)(void*, int, char**, char**),
                  void* arg, char** errmsg);

int sqlite3_prepare_v2(sqlite3* db, const char* zSql, int nByte,
                        sqlite3_stmt** ppStmt, const char** pzTail);

int sqlite3_step(sqlite3_stmt* stmt);
int sqlite3_finalize(sqlite3_stmt* stmt);
int sqlite3_reset(sqlite3_stmt* stmt);

int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int, void (*)(void*));
int sqlite3_bind_int(sqlite3_stmt*, int, int);
int sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_null(sqlite3_stmt*, int);

const unsigned char* sqlite3_column_text(sqlite3_stmt*, int iCol);
int sqlite3_column_int(sqlite3_stmt*, int iCol);
sqlite3_int64 sqlite3_column_int64(sqlite3_stmt*, int iCol);
double sqlite3_column_double(sqlite3_stmt*, int iCol);
int sqlite3_column_type(sqlite3_stmt*, int iCol);
int sqlite3_column_count(sqlite3_stmt*);

const char* sqlite3_errmsg(sqlite3*);
void sqlite3_free(void*);
sqlite3_int64 sqlite3_last_insert_rowid(sqlite3*);
int sqlite3_changes(sqlite3*);

#define SQLITE_OK           0
#define SQLITE_ROW        100
#define SQLITE_DONE       101
#define SQLITE_NULL         5
#define SQLITE_TRANSIENT ((void(*)(void*))-1)

} // extern "C"

#endif // AGOMS_SQLITE3_MIN_H
