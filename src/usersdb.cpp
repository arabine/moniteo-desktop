#include "usersdb.h"

#include <sqlitelib.h>
using namespace sqlitelib;

#include "rapidcsv.h"

UsersDb::UsersDb()
{
    m_db = std::make_unique<Sqlite>("./moniteo.db");

    m_db->execute(R"(
  CREATE TABLE IF NOT EXISTS people (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    firstname TEXT,
    lastname TEXT,
    number INTEGER,
    license TEXT,
    category TEXT,
    sex TEXT,
    club TEXT,
    birthdate TEXT
  )
)");
}

void UsersDb::ImportFile(const std::string &filename)
{
    rapidcsv::Document doc(filename);

    std::vector<std::string> col = doc.GetColumn<std::string>("Nom");
    std::cout << "Read " << col.size() << " values." << std::endl;
}
