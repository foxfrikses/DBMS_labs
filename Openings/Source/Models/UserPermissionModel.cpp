#include "UserPermissionModel.h"

#include <QSqlQuery>

#include "AdminModel.h"

namespace UserPermissionModel {
  bool CanGrantOrRevokePermission(
    UserID userId,
    PermissionID permission [[maybe_unused]]
  )
  {
    return AdminModel::HasAdminRight(userId);
  }

  bool HasPermission(
    UserID userId,
    PermissionID permission
  )
  {
    QSqlQuery query;
    query.prepare("SELECT * "
                  "FROM openings_user_to_user_permission "
                  "WHERE id_user=:id_user "
                  "AND id_permission=:id_permission");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_permission", int(permission));
    if (!query.exec()) {
      throw std::runtime_error("Error while checking user permission");
    }

    return query.next();
  }

  void GrantPermission(
    const AuthenticatedUser& granter,
    UserID userId,
    PermissionID permission
  )
  {
    if (!CanGrantOrRevokePermission(granter.GetUserID(), permission)) {
      throw std::runtime_error("No right to grant user permission");
    }

    QSqlQuery query;
    query.prepare("INSERT INTO openings_user_to_user_permission "
                  "(id_user, id_permission) "
                  "VALUES (:id_user, :id_permission) "
                  "ON CONFLICT (id_user, id_permission) DO NOTHING");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_permission", int(permission));
    if (!query.exec()) {
      throw std::runtime_error("Error while granting user permission");
    }
  }

  void RevokePermission(
    const AuthenticatedUser& revoker,
    UserID userId,
    PermissionID permission
  )
  {
    if (!CanGrantOrRevokePermission(revoker.GetUserID(), permission)) {
      throw std::runtime_error("No right to revoke user permission");
    }

    QSqlQuery query;
    query.prepare("DELETE "
                  "FROM openings_user_to_user_permission "
                  "WHERE id_user=:id_user "
                  "AND id_permission=:id_permission ");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_permission", int(permission));
    if (!query.exec()) {
      throw std::runtime_error("Error while revoking user permission");
    }
  }
}
