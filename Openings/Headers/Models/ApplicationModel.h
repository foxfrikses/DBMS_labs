#ifndef APPLICATIONMODEL_H
#define APPLICATIONMODEL_H

#include "Common.h"
#include "AuthenticatedUser.h"

#include <QDateTime>
#include <QList>

#include <optional>
#include <memory>

namespace ApplicationModel {
  enum class ApplicationStatusID {
    Posted = 1,
    Cancelled = 2,
    Accepted = 3,
    Denied = 4,
  };

  struct ApplicationData {
    ApplicationID id;
    JobOpeningID openingId;
    UserResumeID resumeId;
    QDateTime applicationDate;
    ApplicationStatusID status;
    QDateTime statusChangeDate;
    UserID statusChangerID;
  };

  struct PostApplicationData {
    JobOpeningID openingId;
    UserResumeID resumeId;
  };

  void PostApplication(const PostApplicationData&, AuthenticatedUser);

  bool CanCancel(const ApplicationData&, AuthenticatedUser);
  bool CanAccept(const ApplicationData&, AuthenticatedUser);
  bool CanDeny(const ApplicationData&, AuthenticatedUser);

  void CancelApplication(ApplicationID, AuthenticatedUser);
  void AcceptApplication(ApplicationID, AuthenticatedUser);
  void DenyApplication(ApplicationID, AuthenticatedUser);

  std::unique_ptr<ApplicationData> LoadApplicationByid(ApplicationID, AuthenticatedUser);

  QList<ApplicationData> LoadApplicationsCreatedBy(AuthenticatedUser, std::optional<ApplicationStatusID>);
  QList<ApplicationData> LoadApplicationsForOpeningsCreatedBy(AuthenticatedUser, std::optional<ApplicationStatusID>);
}

#endif // APPLICATIONMODEL_H
