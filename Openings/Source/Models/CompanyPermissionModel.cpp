#include "CompanyPermissionModel.h"

#include <QSqlQuery>

#include "CompanyModel.h"

namespace CompanyPermissionModel {

  bool CanGrantOrRevokePermission(
    UserID userId,
    CompanyID companyId,
    PermissionID permission [[maybe_unused]]
  )
  {
    auto company = CompanyModel::LoadCompanyDataById(companyId);
    if (!company) {
      throw std::runtime_error("No company with such id");
    }

    return company->companyAdmin == userId;
  }

  bool HasPermission(
    UserID userId,
    CompanyID companyId,
    PermissionID permission
  )
  {
    QSqlQuery query;
    query.prepare("SELECT * "
                  "FROM openings_user_to_company_permission "
                  "WHERE id_user=:id_user "
                  "AND id_company=:id_company "
                  "AND id_permission=:id_permission");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_company", int(companyId));
    query.bindValue(":id_permission", int(permission));
    if (!query.exec()) {
      throw std::runtime_error("Error while checking user permission");
    }

    return query.next();
  }

  void GrantPermission(
    const AuthenticatedUser& granter,
    UserID userId,
    CompanyID companyId,
    PermissionID permission
  )
  {
    if (!CanGrantOrRevokePermission(granter.GetUserID(), companyId, permission)) {
      throw std::runtime_error("No right to grant company permission");
    }

    QSqlQuery query;
    query.prepare("INSERT INTO openings_user_to_company_permission "
                  "(id_user, id_permission, id_company) "
                  "VALUES (:id_user, :id_permission, :id_company) "
                  "ON CONFLICT (id_user, id_permission, id_company) DO NOTHING");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_permission", int(permission));
    query.bindValue(":id_company", int(companyId));
    if (!query.exec()) {
      throw std::runtime_error("Error while granting company permission");
    }
  }

  QList<CompanyModel::CompanyData> LoadCompaniesForWhichPermissionExists(
    UserID user,
    PermissionID permissionId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " C.id, " // 0
                  " C.name, " // 1
                  " C.id_company_admin " // 2
                  "FROM openings_company C "
                  "JOIN openings_user_to_company_permission UCP "
                  "ON UCP.id_company=C.id "
                  "WHERE UCP.id_user=:id_user "
                  "AND UCP.id_permission=:id_permission ");
    query.bindValue(":id_user", int(user));
    query.bindValue(":id_permission", int(permissionId));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading company data");
    }

    QList<CompanyModel::CompanyData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = CompanyID(query.value(0).toInt());
      data.companyName = query.value(1).toString();
      data.companyAdmin = UserID(query.value(2).toInt());
    }
    return dataList;
  }

  std::vector<PermissionID> LoadCompanyPermissions(
    UserID userId,
    CompanyID companyId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT id_permission "
                  "FROM openings_user_to_company_permission "
                  "WHERE id_user=:id_user "
                  "AND id_company=:id_company ");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_company", int(companyId));
    if (!query.exec()) {
      throw std::runtime_error("Error while checking user permission");
    }

    std::vector<PermissionID> permissions;
    while (query.next()) {
      permissions.push_back(PermissionID(query.value(0).toInt()));
    }
    return permissions;
  }

  void RevokePermission(
    const AuthenticatedUser& revoker,
    UserID userId,
    CompanyID companyId,
    PermissionID permission
  )
  {
    if (!CanGrantOrRevokePermission(revoker.GetUserID(), companyId, permission)) {
      throw std::runtime_error("No right to revoke company permission");
    }

    QSqlQuery query;
    query.prepare("DELETE "
                  "FROM openings_user_to_company_permission "
                  "WHERE id_user=:id_user "
                  "AND id_company=:id_company "
                  "AND id_permission=:id_permission ");
    query.bindValue(":id_user", int(userId));
    query.bindValue(":id_company", int(companyId));
    query.bindValue(":id_permission", int(permission));
    if (!query.exec()) {
      throw std::runtime_error("Error while revoking user permission");
    }
  }
}
