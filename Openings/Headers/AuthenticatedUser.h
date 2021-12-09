#ifndef AUTHENTICATEDUSER_H
#define AUTHENTICATEDUSER_H

#include <QString>

#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "Common.h"

class AuthenticatedUser;
using AuthenticatedUserPtr = std::unique_ptr<const AuthenticatedUser>;

class AuthenticatedUser
{
  UserID id;

private:
  AuthenticatedUser() = default;

public:
  static AuthenticatedUserPtr Login(QString username, QString password);
  static bool Register(QString username, QString name, QString password);

public:
  UserID GetUserID() const;
};

#endif // AUTHENTICATEDUSER_H
