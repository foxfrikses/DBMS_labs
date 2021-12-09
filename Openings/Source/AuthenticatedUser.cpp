#include <AuthenticatedUser.h>

#include "UserModel.h"
#include "AdminModel.h"

UserID AuthenticatedUser::GetUserID() const {
  return id;
}

AuthenticatedUserPtr AuthenticatedUser::Login(
  QString username,
  QString password
)
{
  auto user = UserModel::LoadByUsername(username);
  if (!user) {
    throw std::runtime_error( "No user with username '" +
                              username.toStdString() + "'" );
  }
  if (!UserModel::VerifyPassword(user->id, password)) {
    throw std::runtime_error( "Incorrect password" );
  }
  auto ptr = std::unique_ptr<AuthenticatedUser>(new AuthenticatedUser);
  ptr->id = user->id;
  return AuthenticatedUserPtr(ptr.release());
}

bool AuthenticatedUser::Register(
  QString username,
  QString name,
  QString password
)
{
  UserModel::InsertUserData insertData;
  insertData.username = username;
  insertData.name = name;
  return UserModel::InsertUser(insertData, password) != nullptr;
}
