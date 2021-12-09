#ifndef COMPANYPERMISSIONMODEL_H
#define COMPANYPERMISSIONMODEL_H

#include "Common.h"

#include "AuthenticatedUser.h"
#include "CompanyModel.h"

#include <vector>

namespace CompanyPermissionModel {
  enum class PermissionID
  {
    WorkWithOpenings = 1,
  };

  bool CanGrantOrRevokePermission(const AuthenticatedUser& admin, CompanyID, PermissionID);

  std::vector<PermissionID> LoadCompanyPermissions(UserID, CompanyID);

  QList<CompanyModel::CompanyData> LoadCompaniesForWhichPermissionExists(UserID, PermissionID);

  bool HasPermission(UserID, CompanyID, PermissionID);
  void GrantPermission(const AuthenticatedUser& admin, UserID, CompanyID, PermissionID);
  void RevokePermission(const AuthenticatedUser& admin, UserID, CompanyID, PermissionID);
}

#endif // COMPANYPERMISSIONMODEL_H
