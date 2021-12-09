#ifndef USERPERMISSIONMODEL_H
#define USERPERMISSIONMODEL_H

#include "Common.h"

#include "AuthenticatedUser.h"

#include <vector>

namespace UserPermissionModel {
  enum class PermissionID
  {
    AcceptCompanyRequest = 1,
  };

  bool CanGrantOrRevokePermission(UserID adminId, PermissionID);

  bool HasPermission(UserID, PermissionID);
  void GrantPermission(const AuthenticatedUser& admin, UserID, PermissionID);
  void RevokePermission(const AuthenticatedUser& admin, UserID, PermissionID);
}

#endif // USERPERMISSIONMODEL_H
