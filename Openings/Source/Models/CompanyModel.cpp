#include "CompanyModel.h"

#include "UserPermissionModel.h"

#include <QSqlQuery>

namespace CompanyModel {
  void RequestCreateCompany(
    QString companyName,
    const AuthenticatedUser& requester
  )
  {
    QSqlQuery query;
    query.prepare("SELECT id "
                  "FROM openings_create_company_request "
                  "WHERE company_name=:company_name "
                  "AND id_requester=:id_requester "
                  "AND request_status=:request_status");
    query.bindValue(":company_name", companyName);
    query.bindValue(":id_requester", int(requester.GetUserID()));
    query.bindValue(":request_status", int(CreateCompanyRequestStatus::Posted));

    if (!query.exec()) {
      throw std::runtime_error("Error while checking if create company request exists");
    }
    if (query.next()) {
      throw std::runtime_error("You have already created a query. Your query is in process");
    }

    query.prepare("INSERT INTO openings_create_company_request "
                  "(company_name, id_requester, id_status_changer) "
                  "VALUES(:company_name, :id_requester, :id_requester)");
    query.bindValue(":company_name", companyName);
    query.bindValue(":id_requester", int(requester.GetUserID()));

    if (!query.exec()) {
      throw std::runtime_error("Error while inserting create company query");
    }
  }

  void CancelCreateCompanyRequest(
    CreateCompanyRequestID createCompanyReqId,
    const AuthenticatedUser& requester
  )
  {
    auto loaded = LoadCreateCompanyRequestData(createCompanyReqId);
    if (!loaded) {
      throw std::runtime_error("There is no create company request with such id");
    }
    if (loaded->requesterId != requester.GetUserID()) {
      throw std::runtime_error("You cannot cancel create company requests which are not yours");
    }
    switch (loaded->status) {
      case CreateCompanyRequestStatus::Cancelled:
        throw std::runtime_error("Already cancelled");

      case CreateCompanyRequestStatus::Denied:
        [[fallthrough]];
      case CreateCompanyRequestStatus::Accepted:
        throw std::runtime_error("Cannot cancel already proceeded request");

      default:
        break;
    }

    QSqlQuery query;
    query.prepare("UPDATE openings_create_company_request "
                  "SET request_status=2, "
                  "    status_change_date=CURRENT_TIMESTAMP, "
                  "    id_status_changer=:id_requester "
                  "WHERE id=:id "
                  "AND id_requester=:id_requester "
                  "AND request_status=1");
    query.bindValue(":id", int(createCompanyReqId));
    query.bindValue(":id_requester", int(requester.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while cancelling a create company request");
    }
  }

  void EnsureCanChangeCreateCompanyRequestStatus(
    const AuthenticatedUser& admin
  )
  {
    if( !UserPermissionModel::HasPermission(admin.GetUserID(),
                                            UserPermissionModel::PermissionID::AcceptCompanyRequest) ) {
      throw std::runtime_error("You have no permission to accept company requests");
    }
  }

  void AcceptCreateCompanyRequest(
    CreateCompanyRequestID createCompanyReqId,
    const AuthenticatedUser& admin
  )
  {
    QSqlDatabase::database().transaction();
    auto loaded = LoadCreateCompanyRequestData(createCompanyReqId);
    if (!loaded) {
      throw std::runtime_error("There is no create company request with such id");
    }
    EnsureCanChangeCreateCompanyRequestStatus(admin);
    switch (loaded->status) {
      case CreateCompanyRequestStatus::Accepted:
        throw std::runtime_error("Already accepted");

      case CreateCompanyRequestStatus::Cancelled:
        throw std::runtime_error("Cannot accept already cancelled company create query");

      default:
        break;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO openings_company "
                  " (name, id_company_admin) "
                  "VALUES(:name, :id_company_admin)" );
    query.bindValue(":name", loaded->companyName);
    query.bindValue(":id_company_admin", int(loaded->requesterId));
    if (!query.exec()) {
      throw std::runtime_error("Error while inserting a company");
    }

    query.prepare("UPDATE openings_create_company_request "
                  "SET request_status=4, "
                  "    status_change_date=CURRENT_TIMESTAMP, "
                  "    id_status_changer=:id_status_changer "
                  "WHERE id=:id "
                  "AND request_status IN (1, 3)");
    query.bindValue(":id", int(createCompanyReqId));
    query.bindValue(":id_status_changer", int(admin.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while accepting a create company request");
    }
    QSqlDatabase::database().commit();
  }

  void DenyCreateCompanyRequest(
    CreateCompanyRequestID createCompanyReqId,
    const AuthenticatedUser& admin
  )
  {
    auto loaded = LoadCreateCompanyRequestData(createCompanyReqId);
    if (!loaded) {
      throw std::runtime_error("There is no create company request with such id");
    }
    EnsureCanChangeCreateCompanyRequestStatus(admin);
    switch (loaded->status) {
      case CreateCompanyRequestStatus::Accepted:
        throw std::runtime_error("Cannot deny already accepted company create query");

      case CreateCompanyRequestStatus::Denied:
        throw std::runtime_error("Already denied");

      case CreateCompanyRequestStatus::Cancelled:
        throw std::runtime_error("Cannot deny already cancelled company create query");

      default:
        break;
    }

    QSqlQuery query;
    query.prepare("UPDATE openings_create_company_request "
                  "SET request_status=3, "
                  "    status_change_date=CURRENT_TIMESTAMP, "
                  "    id_status_changer=:id_status_changer "
                  "WHERE id=:id "
                  "AND request_status=1");
    query.bindValue(":id", int(createCompanyReqId));
    query.bindValue(":id_status_changer", int(admin.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while denying a create company request");
    }
  }

  std::unique_ptr<CreateCompanyRequestData> LoadCreateCompanyRequestData(
    CreateCompanyRequestID createCompanyReqId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " company_name, " // 1
                  " id_requester, " // 2
                  " request_date, " // 3
                  " request_status, " // 4
                  " status_change_date, " // 5
                  " id_status_changer " // 6
                  "FROM openings_create_company_request "
                  "WHERE id=:id" );
    query.bindValue(":id", int(createCompanyReqId));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading create company request data");
    }

    std::unique_ptr<CreateCompanyRequestData> ptr;
    if (query.next()) {
      ptr = std::make_unique<CreateCompanyRequestData>();

      ptr->id = CreateCompanyRequestID(query.value(0).toInt());
      ptr->companyName = query.value(1).toString();
      ptr->requesterId = UserID(query.value(2).toInt());
      ptr->requestDate = query.value(3).toDateTime();
      ptr->status = CreateCompanyRequestStatus(query.value(4).toInt());
      ptr->statusChangeDate = query.value(5).toDateTime();
      ptr->statusChangerId = UserID(query.value(6).toInt());
    }
    return ptr;
  }

  std::unique_ptr<CompanyData> LoadCompanyDataById(
    CompanyID companyId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " name, " // 1
                  " id_company_admin " // 2
                  "FROM openings_company "
                  "WHERE id=?" );
    query.addBindValue(int(companyId));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading company data");
    }

    std::unique_ptr<CompanyData> ptr;
    if (query.next()) {
      ptr = std::make_unique<CompanyData>();

      ptr->id = CompanyID(query.value(0).toInt());
      ptr->companyName = query.value(1).toString();
      ptr->companyAdmin = UserID(query.value(2).toInt());
    }
    return ptr;
  }

  std::unique_ptr<CompanyData> LoadCompanyDataByName(
    QString companyName
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " name, " // 1
                  " id_company_admin " // 2
                  "FROM openings_company "
                  "WHERE name=?" );
    query.addBindValue(companyName);
    if (!query.exec()) {
      throw std::runtime_error("Error while loading company data");
    }

    std::unique_ptr<CompanyData> ptr;
    if (query.next()) {
      ptr = std::make_unique<CompanyData>();

      ptr->id = CompanyID(query.value(0).toInt());
      ptr->companyName = query.value(1).toString();
      ptr->companyAdmin = UserID(query.value(2).toInt());
    }
    return ptr;
  }

  QList<CompanyData> LoadCompanies()
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " name, " // 1
                  " id_company_admin " // 2
                  "FROM openings_company ");
    if (!query.exec()) {
      throw std::runtime_error("Error while loading companies data");
    }

    QList<CompanyData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = CompanyID(query.value(0).toInt());
      data.companyName = query.value(1).toString();
      data.companyAdmin = UserID(query.value(2).toInt());
    }
    return dataList;
  }

  QList<CompanyData> LoadCompaniesAdministratedBy(
    UserID userId
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " name, " // 1
                  " id_company_admin " // 2
                  "FROM openings_company "
                  "WHERE id_company_admin=:id_company_admin ");
    query.bindValue(":id_company_admin", int(userId));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading companies data");
    }

    QList<CompanyData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = CompanyID(query.value(0).toInt());
      data.companyName = query.value(1).toString();
      data.companyAdmin = UserID(query.value(2).toInt());
    }
    return dataList;
  }

  QList<CreateCompanyRequestData> LoadCreateCompanyRequests(
    const AuthenticatedUser& admin
  )
  {
    EnsureCanChangeCreateCompanyRequestStatus(admin);

    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " company_name, " // 1
                  " id_requester, " // 2
                  " request_date, " // 3
                  " request_status, " // 4
                  " status_change_date, " // 5
                  " id_status_changer " // 6
                  "FROM openings_create_company_request");
    if (!query.exec()) {
      throw std::runtime_error("Error while loading create company request data list");
    }

    QList<CreateCompanyRequestData> list;
    while (query.next()) {
      auto& newItem = list.emplace_back();

      newItem.id = CreateCompanyRequestID(query.value(0).toInt());
      newItem.companyName = query.value(1).toString();
      newItem.requesterId = UserID(query.value(2).toInt());
      newItem.requestDate = query.value(3).toDateTime();
      newItem.status = CreateCompanyRequestStatus(query.value(4).toInt());
      newItem.statusChangeDate = query.value(5).toDateTime();
      newItem.statusChangerId = UserID(query.value(6).toInt());
    }
    return list;
  }

  QList<CreateCompanyRequestData> LoadUserCreateCompanyRequests(
    const AuthenticatedUser& user
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  " id, " // 0
                  " company_name, " // 1
                  " id_requester, " // 2
                  " request_date, " // 3
                  " request_status, " // 4
                  " status_change_date, " // 5
                  " id_status_changer " // 6
                  "FROM openings_create_company_request "
                  "WHERE id_requester=?");
    query.addBindValue(int(user.GetUserID()));
    if (!query.exec()) {
      throw std::runtime_error("Error while loading create company request data list");
    }

    QList<CreateCompanyRequestData> list;
    while (query.next()) {
      auto& newItem = list.emplace_back();

      newItem.id = CreateCompanyRequestID(query.value(0).toInt());
      newItem.companyName = query.value(1).toString();
      newItem.requesterId = UserID(query.value(2).toInt());
      newItem.requestDate = query.value(3).toDateTime();
      newItem.status = CreateCompanyRequestStatus(query.value(4).toInt());
      newItem.statusChangeDate = query.value(5).toDateTime();
      newItem.statusChangerId = UserID(query.value(6).toInt());
    }
    return list;
  }
}
