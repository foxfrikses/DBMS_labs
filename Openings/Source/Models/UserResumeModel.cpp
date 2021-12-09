#include "UserResumeModel.h"

#include <QSqlQuery>
#include <QSqlError>

namespace UserResumeModel {
  UserResumeID InsertUserResume(
    const InsertUserResumeData& data,
    AuthenticatedUser user
  )
  {
    QSqlQuery query;
    query.prepare("INSERT INTO openings_user_resume "
                  "(filename, blob, id_user) "
                  "VALUES (:filename, :blob, :id_user) ");
    query.bindValue(":filename", data.filename);
    query.bindValue(":blob", data.blob);
    query.bindValue(":id_user", int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while inserting user resume into the database" +
                               query.lastError().text().toStdString());
    }

    return UserResumeID(query.lastInsertId().toInt());
  }

  std::unique_ptr<UserResumeData> LoadUserResume(
    UserResumeID id
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  "  id, " // 0
                  "  filename, " // 1
                  "  blob, " // 2
                  "  id_user " // 3
                  "FROM openings_user_resume "
                  "WHERE id=:id");
    query.bindValue(":id", int(id));
    if (!query.exec()) {
      throw std::runtime_error("Error while inserting user resume into the database");
    }

    std::unique_ptr<UserResumeData> ptr;
    if (query.next()) {
      ptr = std::make_unique<UserResumeData>();

      ptr->id = UserResumeID(query.value(0).toInt());
      ptr->filename = query.value(1).toString();
      ptr->blob = query.value(2).toByteArray();
      ptr->userId = UserID(query.value(3).toInt());
    }
    return ptr;
  }
}
