#include "ApplicationModel.h"

#include "UserResumeModel.h"
#include "CompanyPermissionModel.h"

#include "JobOpeningModel.h"

#include <QSqlQuery>
#include <QSqlError>

namespace ApplicationModel {
  void EnsureIsCreatorOfResume(UserResumeID resumeId, UserID userId) {
    auto resume = UserResumeModel::LoadUserResume(resumeId);
    if (!resume) {
      throw std::runtime_error("Cannot load application with specified id");
    }
    if (resume->userId != userId) {
      throw std::runtime_error("It's not your resume");
    }
  };

  void EnsureCanManageOpening(JobOpeningID OpeningId, UserID userId) {
    auto opening = JobOpeningModel::LoadJobOpeningById(OpeningId);
    if (!opening) {
      throw std::runtime_error("Cannot load job opening with specified id");
    }

    if( !CompanyPermissionModel::HasPermission(userId,
                                               opening->companyId,
                                               CompanyPermissionModel::PermissionID::WorkWithOpenings)) {
      throw std::runtime_error("You cannot manage this opening's application");
    }
  };

  void PostApplication(
    const PostApplicationData& data,
    AuthenticatedUser user
  )
  {
    EnsureIsCreatorOfResume(data.resumeId, user.GetUserID());

    QSqlQuery query;
    query.prepare("INSERT INTO openings_job_opening_application "
                  "(id_resume, "
                  " id_opening, "
                  " id_status_changer) "
                  "VALUES "
                  "(:id_resume, "
                  " :id_opening, "
                  " :id_status_changer) ");
    query.bindValue(":id_resume", int(data.resumeId));
    query.bindValue(":id_opening", int(data.openingId));
    query.bindValue(":id_status_changer", int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while inserting an application.\n" +
                               query.lastError().text().toStdString());
    }
  }

  void EnsureCanCancelApplication(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    EnsureIsCreatorOfResume(data.resumeId, user.GetUserID());

    switch (data.status) {
      case ApplicationModel::ApplicationStatusID::Cancelled:
        throw std::runtime_error("Already cancelled");

      case ApplicationModel::ApplicationStatusID::Accepted: [[fallthrough]];
      case ApplicationModel::ApplicationStatusID::Denied:
        throw std::runtime_error("Cannot cancell already proceeded application");

      case ApplicationModel::ApplicationStatusID::Posted:
        break;
    }
  }

  void EnsureCanAcceptApplication(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    EnsureCanManageOpening(data.openingId, user.GetUserID());

    switch (data.status) {
      case ApplicationModel::ApplicationStatusID::Cancelled:
        throw std::runtime_error("Cannot accept already cancelled application");

      case ApplicationModel::ApplicationStatusID::Accepted:
        throw std::runtime_error("Already accepted");

      case ApplicationModel::ApplicationStatusID::Denied: [[fallthrough]];
      case ApplicationModel::ApplicationStatusID::Posted:
        break;
    }
  }

  void EnsureCanDenyApplication(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    EnsureCanManageOpening(data.openingId, user.GetUserID());

    switch (data.status) {
      case ApplicationModel::ApplicationStatusID::Cancelled:
        throw std::runtime_error("Cannot deny already cancelled application");

      case ApplicationModel::ApplicationStatusID::Accepted:
        throw std::runtime_error("Cannot deny already accepted application");

      case ApplicationModel::ApplicationStatusID::Denied:
        throw std::runtime_error("Already denied");

      case ApplicationModel::ApplicationStatusID::Posted:
        break;
    }
  }

  void CancelApplication(
    ApplicationID id,
    AuthenticatedUser user
  )
  {
    auto application = LoadApplicationByid(id, user);
    if (!application) {
      throw std::runtime_error("Cannot load application with specified id");
    }
    EnsureCanCancelApplication(*application, user);

    QSqlQuery query;
    query.prepare("UPDATE openings_job_opening_application "
                  "SET "
                  " id_status_changer=:id_user, "
                  " status_change_date=CURRENT_TIMESTAMP, "
                  " application_status=2 "
                  "WHERE "
                  " id=:id");
    query.bindValue(":id", int(id));
    query.bindValue(":id_user", int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while cancelling an application.\n" +
                               query.lastError().text().toStdString());
    }
  }

  void AcceptApplication(
    ApplicationID id,
    AuthenticatedUser user
  )
  {
    auto application = LoadApplicationByid(id, user);
    if (!application) {
      throw std::runtime_error("Cannot load application with specified id");
    }
    EnsureCanAcceptApplication(*application, user);

    QSqlQuery query;
    query.prepare("UPDATE openings_job_opening_application "
                  "SET "
                  " id_status_changer=:id_user, "
                  " status_change_date=CURRENT_TIMESTAMP, "
                  " application_status=3 "
                  "WHERE "
                  " id=:id");
    query.bindValue(":id", int(id));
    query.bindValue(":id_user", int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while accepting an application.\n" +
                               query.lastError().text().toStdString());
    }
  }
  void DenyApplication(
    ApplicationID id,
    AuthenticatedUser user
  )
  {
    auto application = LoadApplicationByid(id, user);
    if (!application) {
      throw std::runtime_error("Cannot load application with specified id");
    }
    EnsureCanDenyApplication(*application, user);

    QSqlQuery query;
    query.prepare("UPDATE openings_job_opening_application "
                  "SET "
                  " id_status_changer=:id_user, "
                  " status_change_date=CURRENT_TIMESTAMP, "
                  " application_status=4 "
                  "WHERE "
                  " id=:id");
    query.bindValue(":id", int(id));
    query.bindValue(":id_user", int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while denying an application.\n" +
                               query.lastError().text().toStdString());
    }
  }

  bool CanCancel(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    try {EnsureCanCancelApplication(data, user);}
    catch (...) {return false;}
    return true;
  }

  bool CanAccept(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    try {EnsureCanAcceptApplication(data, user);}
    catch (...) {return false;}
    return true;
  }

  bool CanDeny(
    const ApplicationData& data,
    AuthenticatedUser user
  )
  {
    try {EnsureCanDenyApplication(data, user);}
    catch (...) {return false;}
    return true;
  }

  std::unique_ptr<ApplicationData> LoadApplicationByid(
    ApplicationID id,
    AuthenticatedUser user
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " id_resume, " // 1
                  " id_opening, " // 2
                  " application_date, " // 3
                  " application_status, " // 4
                  " status_change_date, " // 5
                  " id_status_changer " // 6
                  "FROM openings_job_opening_application "
                  "WHERE id=:id");
    query.bindValue(":id", int(id));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading an application.\n" +
                               query.lastError().text().toStdString());
    }

    if (!query.next()) {
      return nullptr;
    }

    auto ptr = std::make_unique<ApplicationData>();

    ptr->id = ApplicationID(query.value(0).toInt());
    ptr->resumeId = UserResumeID(query.value(1).toInt());
    ptr->openingId = JobOpeningID(query.value(2).toInt());
    ptr->applicationDate = query.value(3).toDateTime();
    ptr->status = ApplicationStatusID(query.value(4).toInt());
    ptr->statusChangeDate = query.value(5).toDateTime();
    ptr->statusChangerID = UserID(query.value(6).toInt());

    std::string error_str;
    try {
      EnsureIsCreatorOfResume(ptr->resumeId, user.GetUserID());
      return ptr;
    }
    catch (std::exception& ex) {
      error_str += ex.what();
    }

    try {
      EnsureCanManageOpening(ptr->openingId, user.GetUserID());
      return ptr;
    }
    catch (std::exception& ex) {
      if (!error_str.empty()) {
        error_str += '\n';
      }
      error_str += ex.what();
      throw std::runtime_error(error_str);
    }
  }

  QList<ApplicationData> LoadApplicationsCreatedBy(
    AuthenticatedUser user,
    std::optional<ApplicationStatusID> status
  )
  {
    QString queryStr("SELECT "
                     " A.id, " // 0
                     " A.id_resume, " // 1
                     " A.id_opening, " // 2
                     " A.application_date, " // 3
                     " A.application_status, " // 4
                     " A.status_change_date, " // 5
                     " A.id_status_changer " // 6
                     "FROM openings_job_opening_application as A "
                     "JOIN openings_user_resume as R ON "
                     "  R.id=A.id_resume "
                     "WHERE R.id_user=:id_user ");
    if (status.has_value()) {
      queryStr += "AND A.application_status=:application_status";
    }

    QSqlQuery query;
    query.prepare(queryStr);
    query.bindValue(":id_user", int(user.GetUserID()));
    if (status.has_value()) {
      query.bindValue(":application_status", int(status.value()));
    }
    if (!query.exec()) {
      throw std::runtime_error("Error while loading applications.\n" +
                               query.lastError().text().toStdString());
    }

    QList<ApplicationData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = ApplicationID(query.value(0).toInt());
      data.resumeId = UserResumeID(query.value(1).toInt());
      data.openingId = JobOpeningID(query.value(2).toInt());
      data.applicationDate = query.value(3).toDateTime();
      data.status = ApplicationStatusID(query.value(4).toInt());
      data.statusChangeDate = query.value(5).toDateTime();
      data.statusChangerID = UserID(query.value(6).toInt());
    }
    return dataList;
  }

  QList<ApplicationData> LoadApplicationsForOpeningsCreatedBy(
    AuthenticatedUser user,
    std::optional<ApplicationStatusID> status
  )
  {
    QString queryStr("SELECT "
                     " A.id, " // 0
                     " A.id_resume, " // 1
                     " A.id_opening, " // 2
                     " A.application_date, " // 3
                     " A.application_status, " // 4
                     " A.status_change_date, " // 5
                     " A.id_status_changer " // 6
                     "FROM openings_job_opening_application as A "
                     "JOIN openings_job_opening as O "
                     "ON O.id=A.id_opening "
                     "WHERE O.id_creator=:id_user ");
    if (status.has_value()) {
      queryStr += "AND A.application_status=:application_status";
    }

    QSqlQuery query;
    query.prepare(queryStr);
    query.bindValue(":id_user", int(user.GetUserID()));
    if (status.has_value()) {
      query.bindValue(":application_status", int(status.value()));
    }
    if (!query.exec()) {
      throw std::runtime_error("Error while loading applications.\n" +
                               query.lastError().text().toStdString());
    }

    QList<ApplicationData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = ApplicationID(query.value(0).toInt());
      data.resumeId = UserResumeID(query.value(1).toInt());
      data.openingId = JobOpeningID(query.value(2).toInt());
      data.applicationDate = query.value(3).toDateTime();
      data.status = ApplicationStatusID(query.value(4).toInt());
      data.statusChangeDate = query.value(5).toDateTime();
      data.statusChangerID = UserID(query.value(6).toInt());
    }
    return dataList;
  }
}
