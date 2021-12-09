#ifndef USERRESUMEMODEL_H
#define USERRESUMEMODEL_H

#include <QByteArray>
#include <QString>

#include <memory>

#include "Common.h"
#include "AuthenticatedUser.h"

namespace UserResumeModel {
  struct UserResumeData {
    UserResumeID id;
    UserID userId;
    QString filename;
    QByteArray blob;
  };

  struct InsertUserResumeData {
    QString filename;
    QByteArray blob;
  };

  UserResumeID InsertUserResume(const InsertUserResumeData&, AuthenticatedUser);
  std::unique_ptr<UserResumeData> LoadUserResume(UserResumeID);
}

#endif // USERRESUMEMODEL_H
