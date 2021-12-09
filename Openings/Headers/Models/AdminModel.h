#ifndef ADMINMODEL_H
#define ADMINMODEL_H

#include "Common.h"

#include <vector>

namespace AdminModel {
  bool CanDealWithAdminRights();

  bool HasAdminRight(UserID);
  bool GrantAdminRight(UserID);
  bool RevokeAdminRight(UserID);
}

#endif // ADMINMODEL_H
