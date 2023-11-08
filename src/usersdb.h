#ifndef USERSDB_H
#define USERSDB_H

#include <sqlitelib.h>
#include <memory>

class UsersDb
{
public:
    UsersDb();

    void ImportFile(const std::string &filename);

private:
    std::unique_ptr<sqlitelib::Sqlite> m_db;
};

#endif // USERSDB_H
