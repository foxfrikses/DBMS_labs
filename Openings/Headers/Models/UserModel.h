#ifndef USERMODEL_H
#define USERMODEL_H

#include "Common.h"

#include <QString>
#include <QCryptographicHash>
#include <QDateTime>
#include <QList>

#include <memory>

namespace UserModel {
  struct InsertUserData {
    QString username;
    QString name;
  };

  struct UserData {
    UserID id;
    QString username;
    QString name;
    QDateTime registrationDate;
  };

  std::unique_ptr<UserData> InsertUser(const InsertUserData&, QString password);
  std::unique_ptr<UserData> LoadById(UserID);
  std::unique_ptr<UserData> LoadByUsername(QString);
  QList<UserData> LoadUsers();
  void UpdateUserData(const UserData&, QString password);
  void DeleteUser(UserID, QString password);
  bool VerifyPassword(UserID, QString password);
  void UpdatePassword(UserID, QString oldPassword, QString newPassword);
}


#endif // USERMODEL_H
