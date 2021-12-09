#include "JobOpeningModel.h"

#include <QSqlQuery>
#include <QSqlDatabase>

#include "CompanyPermissionModel.h"

namespace JobOpeningModel {
  QList<JobOpeningData> LoadJobOpenings(
    std::optional<JobOpeningStatus> status,
    std::optional<CompanyID> company,
    std::optional<UserID> creator
  )
  {
    QString whereString;
    {
      bool isWhereAdded = false;
      auto addWhereOrAnd = [&whereString, &isWhereAdded]() {
        if (isWhereAdded) whereString += " AND";
        else {whereString += " WHERE"; isWhereAdded = true;}
      };

      if (status.has_value()) {
        addWhereOrAnd();
        whereString += " opening_status=" + QString::number(int(status.value()));
      }

      if (company.has_value()) {
        addWhereOrAnd();
        whereString += " id_company=" + QString::number(int(company.value()));
      }

      if (creator.has_value()) {
        addWhereOrAnd();
        whereString += " id_creator=" + QString::number(int(creator.value()));
      }
    }

    QSqlQuery query;
    query.prepare("SELECT "
                  "  id, " // 0
                  "  title, " // 1
                  "  description, " // 2
                  "  id_company, " // 3
                  "  create_date, " // 4
                  "  id_creator, " // 5
                  "  opening_status, " // 6
                  "  status_change_date, " // 7
                  "  id_status_changer " // 8
                  "FROM openings_job_opening" +
                  whereString);

    if (!query.exec()) {
      throw std::runtime_error("Error while loading job opening by id");
    }

    QList<JobOpeningData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = JobOpeningID(query.value(0).toInt());
      data.title = query.value(1).toString();
      data.description = query.value(2).toString();
      data.companyId = CompanyID(query.value(3).toInt());
      data.createDate = query.value(4).toDateTime();
      data.creatorId = UserID(query.value(5).toInt());
      data.status = JobOpeningStatus(query.value(6).toInt());
      data.statusChangeDate = query.value(7).toDateTime();
      data.statusChangerId = UserID(query.value(8).toInt());
    }
    return dataList;
  }

  std::unique_ptr<JobOpeningData> LoadJobOpeningById(
    JobOpeningID openingId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  "  id, " // 0
                  "  title, " // 1
                  "  description, " // 2
                  "  id_company, " // 3
                  "  create_date, " // 4
                  "  id_creator, " // 5
                  "  opening_status, " // 6
                  "  status_change_date, " // 7
                  "  id_status_changer " // 8
                  "FROM openings_job_opening "
                  "WHERE id=?");
    query.addBindValue(int(openingId));

    if (!query.exec()) {
      throw std::runtime_error("Error while loading job opening by id");
    }

    std::unique_ptr<JobOpeningData> ptr;
    if (query.next()) {
      ptr = std::make_unique<JobOpeningData>();

      ptr->id = JobOpeningID(query.value(0).toInt());
      ptr->title = query.value(1).toString();
      ptr->description = query.value(2).toString();
      ptr->companyId = CompanyID(query.value(3).toInt());
      ptr->createDate = query.value(4).toDateTime();
      ptr->creatorId = UserID(query.value(5).toInt());
      ptr->status = JobOpeningStatus(query.value(6).toInt());
      ptr->statusChangeDate = query.value(7).toDateTime();
      ptr->statusChangerId = UserID(query.value(8).toInt());
    }
    return ptr;
  }

  void EnsureCanWorkWithOpenings(
    UserID userId,
    CompanyID companyId
  )
  {
    if (!CompanyPermissionModel::HasPermission(userId, companyId, CompanyPermissionModel::PermissionID::WorkWithOpenings)) {
      throw std::runtime_error("You have not gat the right to work with openings of this company");
    }
  }

  void CreateJobOpening(
    const JobOpeningCreateData& data,
    const AuthenticatedUser& requester
  )
  {
    EnsureCanWorkWithOpenings(requester.GetUserID(), data.companyId);

    QSqlQuery query;
    query.prepare("INSERT INTO openings_job_opening "
                  "(title, description, id_company, id_creator, id_status_changer) "
                  "VALUES (:title, :description, :id_company, :id_requester, :id_requester)");
    query.bindValue(":title", data.title);
    query.bindValue(":description", data.description);
    query.bindValue(":id_company", int(data.companyId));
    query.bindValue(":id_requester", int(requester.GetUserID()));

    if (!query.exec()) {
      throw std::runtime_error("Error while creating job opening status");
    }
  }

  void UpdateJobOpening(
    const JobOpeningUpdateData& data,
    const AuthenticatedUser& requester
  )
  {
    auto opening = LoadJobOpeningById(data.id);
    if (!opening) {
      throw std::runtime_error("Opening with such id doesn't exist");
    }

    EnsureCanWorkWithOpenings(requester.GetUserID(), opening->companyId);

    QSqlQuery query;
    query.prepare("UPDATE openings_job_opening "
                  "SET "
                  "  title=:title, "
                  "  description=:description "
                  "WHERE id=:id");
    query.bindValue(":title", data.title);
    query.bindValue(":description", data.description);
    query.bindValue(":id", int(data.id));

    if (!query.exec()) {
      throw std::runtime_error("Error while updating job opening status");
    }

  }

  void CloseJobOpening(
    JobOpeningID openingId,
    const AuthenticatedUser& requester
  )
  {
    auto opening = LoadJobOpeningById(openingId);
    if (!opening) {
      throw std::runtime_error("Opening with such id doesn't exist");
    }
    EnsureCanWorkWithOpenings(requester.GetUserID(), opening->companyId);

    QSqlQuery query;
    query.prepare("UPDATE openings_job_opening "
                  "SET "
                  "  opening_status=2, "
                  "  status_change_date=CURRENT_TIMESTAMP, "
                  "  id_status_changer=:id_status_changer "
                  "WHERE "
                  "  id=:id_opening");
    query.bindValue(":id_status_changer", int(requester.GetUserID()));
    query.bindValue(":id_opening", int(openingId));

    if (!query.exec()) {
      throw std::runtime_error("Error while updating job opening status");
    }
  }
}
