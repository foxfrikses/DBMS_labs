#include "AdminModel.h"

#include <QSqlQuery>
#include <QSqlDatabase>

#include <unordered_set>

namespace AdminModel {
  bool CanDealWithAdminRights() {
    QSqlQuery query;
    query.prepare( "SELECT privilege_type "
                   "FROM information_schema.role_table_grants "
                   "WHERE table_name='openings_admin' "
                   "AND grantee=:username " );
    query.bindValue(":username", QSqlDatabase::database().userName());
    if (!query.exec()) {
      return false;
    }

    std::unordered_set<QString> privs;
    while (query.next()) {
      privs.insert(query.value(0).toString());
    }
    return
        privs.count("SELECT") == 1 &&
        privs.count("INSERT") == 1 &&
        privs.count("DELETE") == 1 &&
        privs.count("UPDATE") == 1;
  }

  bool HasAdminRight(
    UserID userId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT id_user "
                  "FROM openings_admin "
                  "WHERE id_user = " + QString::number(userId));
    return query.exec() && query.next();
  }
  bool GrantAdminRight(
    UserID userId
  )
  {
    QSqlQuery query;
    query.prepare("INSERT INTO openings_admin (id_user) "
                  "VALUES (?) "
                  "ON CONFLICT (id_user) DO NOTHING");
    query.addBindValue(int(userId));
    return query.exec();
  }

  bool RevokeAdminRight(
    UserID userId
  )
  {
    QSqlQuery query;
    query.prepare("DELETE FROM openings_admin "
                  "WHERE id_user = ?");
    query.addBindValue(int(userId));
    return query.exec();
  }
}
