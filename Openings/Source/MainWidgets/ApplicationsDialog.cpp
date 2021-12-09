#include "ApplicationsDialog.h"
#include "ui_ApplicationsDialog.h"

#include "ApplicationDialog.h"

#include "JobOpeningModel.h"
#include "UserModel.h"
#include "CompanyModel.h"
#include "UserResumeModel.h"

#include <QMessageBox>
#include <QAction>
#include <QMenu>

ApplicationsDialog::ApplicationsDialog(
  AuthenticatedUser user,
  Mode mode,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , mode(mode)
  , ui(new Ui::ApplicationsDialog)
{
  ui->setupUi(this);

  ui->applicationTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->applicationTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->applicationTable->setColumnCount(7); // without description
  ui->applicationTable->setHorizontalHeaderLabels(
    {"Job Title",
     "Company",
     "Applicant",
     "Application date",
     "Status",
     "Status change date",
     "Status changer"}
  );

  Reload();
}

ApplicationsDialog::~ApplicationsDialog()
{
  delete ui;
}

std::unique_ptr<ApplicationsDialog> ApplicationsDialog::CreateUserApplicationsWidget(
  AuthenticatedUser user,
  QWidget *parent
)
{
  auto widget = std::unique_ptr<ApplicationsDialog>(new ApplicationsDialog(user, Mode::userApplications, parent));

  return widget;
}

std::unique_ptr<ApplicationsDialog> ApplicationsDialog::CreateUserOpeningsApplicationsWidget(
  AuthenticatedUser user,
  QWidget *parent)
{
  auto widget = std::unique_ptr<ApplicationsDialog>(new ApplicationsDialog(user, Mode::userOpeningsApplications, parent));

  return widget;
}

void ApplicationsDialog::Reload()
{
  struct Position {
    QString jobTitle;
    QString companyName;
  };

  std::unordered_map<JobOpeningID, Position> jobOpeningIdToPosition;
  std::unordered_map<UserResumeID, UserID> resumeIdToUserId;
  std::unordered_map<UserID, QString> userIdToUsername;

  try {
    switch (mode) {
      case Mode::userApplications:
        applicationList = ApplicationModel::LoadApplicationsCreatedBy(user, std::nullopt);
        break;

      case Mode::userOpeningsApplications:
        applicationList = ApplicationModel::LoadApplicationsForOpeningsCreatedBy(user, std::nullopt);
        break;
    }

    for (auto& application : applicationList) {
      if (!jobOpeningIdToPosition.count(application.openingId)) {
        if (auto opening = JobOpeningModel::LoadJobOpeningById(application.openingId)) {
          auto& position = jobOpeningIdToPosition[opening->id];

          if (!userIdToUsername.count(opening->creatorId)) {
            if (auto user = UserModel::LoadById(opening->creatorId)) {
              userIdToUsername.emplace(user->id, user->username);
            } else userIdToUsername.emplace(opening->creatorId, "ERROR USER");
          }

          if (auto company = CompanyModel::LoadCompanyDataById(opening->companyId)) {
            position.companyName = company->companyName;
          } else position.companyName = "ERROR COMPANY";
          position.jobTitle = opening->title;
        }
      }

      if (!resumeIdToUserId.count(application.resumeId)) {
        if (auto resume = UserResumeModel::LoadUserResume(application.resumeId)) {
          resumeIdToUserId.emplace(resume->id, resume->userId);

          if (!userIdToUsername.count(resume->userId)) {
            if (auto user = UserModel::LoadById(resume->userId)) {
              userIdToUsername.emplace(user->id, user->username);
            } else userIdToUsername.emplace(resume->userId, "ERROR USER");
          }
        }
      }
    }
  } catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
    applicationList.clear();
    ui->applicationTable->setRowCount(0);
    return;
  }

  std::unordered_map<ApplicationModel::ApplicationStatusID, QString> statusIdToString{
    {ApplicationModel::ApplicationStatusID::Accepted, "Accepted"},
    {ApplicationModel::ApplicationStatusID::Cancelled, "Cancelled"},
    {ApplicationModel::ApplicationStatusID::Denied, "Denied"},
    {ApplicationModel::ApplicationStatusID::Posted, "Posted"},
  };

  ui->applicationTable->setRowCount(applicationList.size());

  for (int row = 0; row < ui->applicationTable->rowCount(); ++row) {
    auto& elem = applicationList[row];

    ui->applicationTable->setItem(row, 0, new QTableWidgetItem(jobOpeningIdToPosition[elem.openingId].jobTitle));
    ui->applicationTable->setItem(row, 1, new QTableWidgetItem(jobOpeningIdToPosition[elem.openingId].companyName));
    ui->applicationTable->setItem(row, 2, new QTableWidgetItem(userIdToUsername[resumeIdToUserId[elem.resumeId]]));
    ui->applicationTable->setItem(row, 3, new QTableWidgetItem(elem.applicationDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->applicationTable->setItem(row, 4, new QTableWidgetItem(statusIdToString[elem.status]));
    ui->applicationTable->setItem(row, 5, new QTableWidgetItem(elem.statusChangeDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->applicationTable->setItem(row, 6, new QTableWidgetItem(userIdToUsername[elem.statusChangerID]));
  }
}

void ApplicationsDialog::ShowTableContextMenu(const QPoint &p)
{
  auto item = ui->applicationTable->itemAt(p);
  if (!item) {
    return;
  }

  auto index = ui->applicationTable->indexFromItem(item);
  if (index.row() < 0 || index.row() >= applicationList.size()) {
    return;
  }

  auto& selectedApplication = applicationList[index.row()];

  std::vector<std::unique_ptr<QAction>> actions;

  if (ApplicationModel::CanAccept(selectedApplication, user)) {
    actions.push_back(std::make_unique<QAction>("Accept application", ui->applicationTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedApplication] (bool) {
      try {
        ApplicationModel::AcceptApplication(selectedApplication.id, user);
        QMessageBox::information(this, "Info", "Application accepted");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (ApplicationModel::CanDeny(selectedApplication, user)) {
    actions.push_back(std::make_unique<QAction>("Deny application", ui->applicationTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedApplication] (bool) {
      try {
        ApplicationModel::DenyApplication(selectedApplication.id, user);
        QMessageBox::information(this, "Info", "Application denied");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (ApplicationModel::CanDeny(selectedApplication, user)) {
    actions.push_back(std::make_unique<QAction>("Close application", ui->applicationTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedApplication] (bool) {
      try {
        ApplicationModel::DenyApplication(selectedApplication.id, user);
        QMessageBox::information(this, "Info", "Application closed");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  {
    actions.push_back(std::make_unique<QAction>("View more", ui->applicationTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedApplication] (bool) {
      try {
        ApplicationDialog dialog(user, selectedApplication.id, this);
        dialog.exec();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (actions.empty()) {
    return;
  }

  QMenu menu(ui->applicationTable);
  menu.setWindowModality(Qt::WindowModality::WindowModal);
  for (auto& action : actions) {
    menu.addAction( action.get() );
  }
  menu.exec(p);
}
