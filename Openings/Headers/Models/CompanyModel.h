#ifndef COMPANYMODEL_H
#define COMPANYMODEL_H

#include "Common.h"

#include <QString>
#include <QCryptographicHash>
#include <QDateTime>

#include "AuthenticatedUser.h"

#include <QList>

#include <memory>

namespace CompanyModel {
  struct CompanyData {
    CompanyID id;
    QString companyName;
    UserID companyAdmin;
  };

  std::unique_ptr<CompanyData> LoadCompanyDataById(CompanyID);
  std::unique_ptr<CompanyData> LoadCompanyDataByName(QString);
  QList<CompanyData> LoadCompanies();
  QList<CompanyData> LoadCompaniesAdministratedBy(UserID);

  struct CreateCompanyRequestData {
    CreateCompanyRequestID id;
    QString companyName;
    UserID requesterId;
    QDateTime requestDate;
    CreateCompanyRequestStatus status;
    QDateTime statusChangeDate;
    UserID statusChangerId;
  };

  void RequestCreateCompany(QString companyName, const AuthenticatedUser& requester);
  void CancelCreateCompanyRequest(CreateCompanyRequestID, const AuthenticatedUser& requester);
  void AcceptCreateCompanyRequest(CreateCompanyRequestID, const AuthenticatedUser& admin);
  void DenyCreateCompanyRequest(CreateCompanyRequestID, const AuthenticatedUser& admin);
  QList<CreateCompanyRequestData> LoadCreateCompanyRequests(const AuthenticatedUser& admin);
  QList<CreateCompanyRequestData> LoadUserCreateCompanyRequests(const AuthenticatedUser& user);

  std::unique_ptr<CreateCompanyRequestData> LoadCreateCompanyRequestData(CreateCompanyRequestID);
}

#endif // COMPANYMODEL_H
