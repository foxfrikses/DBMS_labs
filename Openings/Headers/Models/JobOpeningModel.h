#ifndef JOBOPENINGMODEL_H
#define JOBOPENINGMODEL_H

#include "Common.h"

#include <QString>
#include <QCryptographicHash>
#include <QDateTime>

#include "AuthenticatedUser.h"

#include <QList>

#include <memory>

namespace JobOpeningModel {
  enum class JobOpeningStatus {
    Posted = 1,
    Closed = 2,
  };

  struct JobOpeningData {
    JobOpeningID id;
    QString title;
    QString description;
    CompanyID companyId;
    QDateTime createDate;
    UserID creatorId;
    JobOpeningStatus status;
    QDateTime statusChangeDate;
    UserID statusChangerId;
  };

  struct JobOpeningCreateData {
    QString title;
    QString description;
    CompanyID companyId;
  };

  struct JobOpeningUpdateData {
    JobOpeningID id;
    QString title;
    QString description;
  };

  QList<JobOpeningData> LoadJobOpenings(std::optional<JobOpeningStatus> status,
                                        std::optional<CompanyID> company,
                                        std::optional<UserID> creator);

  std::unique_ptr<JobOpeningData> LoadJobOpeningById(JobOpeningID);

  void CreateJobOpening(const JobOpeningCreateData&, const AuthenticatedUser& requester);
  void UpdateJobOpening(const JobOpeningUpdateData&, const AuthenticatedUser& requester);
  void CloseJobOpening(JobOpeningID, const AuthenticatedUser& requester);
}

#endif // JOBOPENINGMODEL_H
