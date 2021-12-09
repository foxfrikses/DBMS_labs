#include "OpeningsDialog.h"
#include "ui_OpeningsDialog.h"

#include "JobOpeningModel.h"
#include "UserModel.h"
#include "CompanyModel.h"
#include "CompanyPermissionModel.h"

#include "JobOpeningDialog.h"
#include "ApplicationDialog.h"

#include <QMessageBox>
#include <QAction>
#include <QMenu>

OpeningsDialog::~OpeningsDialog()
{
  delete ui;
}

OpeningsDialog::OpeningsDialog(
  AuthenticatedUser user,
  Mode mode,
  std::optional<CompanyID> companyId,
  QWidget *parent
)
  : QDialog(parent)
  , user(user)
  , companyId(companyId)
  , mode(mode)
  , ui(new Ui::OpeningsDialog)
{
  ui->setupUi(this);

  ui->openingsTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->openingsTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->openingsTable->setColumnCount(7); // without description
  ui->openingsTable->setHorizontalHeaderLabels(
    {"Job Title",
     "Company",
     "Create date",
     "Creator",
     "Status",
     "Status change date",
     "Status changer"}
  );

  Reload();
}

std::unique_ptr<OpeningsDialog> OpeningsDialog::CreateCompanyOpenOpeningsWidget(
  AuthenticatedUser user,
  CompanyID companyId,
  QWidget *parent
)
{
  std::unique_ptr<OpeningsDialog> widget(new OpeningsDialog(user, Mode::companyOpenOpenings, companyId, parent));
  return widget;
}

std::unique_ptr<OpeningsDialog> OpeningsDialog::CreateUserOpeningsWidget(
  AuthenticatedUser user,
  QWidget *parent
)
{
  std::unique_ptr<OpeningsDialog> widget(new OpeningsDialog(user, Mode::userOpenings, std::nullopt, parent));
  return widget;
}

std::unique_ptr<OpeningsDialog> OpeningsDialog::CreateOpenOpeningsWidget(
  AuthenticatedUser user,
  QWidget *parent
)
{
  std::unique_ptr<OpeningsDialog> widget(new OpeningsDialog(user, Mode::openOpenings, std::nullopt, parent));
  return widget;
}

#define ErrorReturn(str) \
  do { \
    QMessageBox::critical(this, "Error", str); \
    return; \
  } while (false)

void OpeningsDialog::Reload()
{
  std::optional<JobOpeningModel::JobOpeningStatus> status;
  std::optional<CompanyID> companyId;
  std::optional<UserID> creatorId;

  switch (mode) {
    case Mode::companyOpenOpenings:
      companyId = this->companyId;
      status = JobOpeningModel::JobOpeningStatus::Posted;
      break;

    case Mode::openOpenings:
      status = JobOpeningModel::JobOpeningStatus::Posted;
      break;

    case Mode::userOpenings:
      creatorId = this->user.GetUserID();
      break;
  }

  try {
  }
  catch (std::exception& ex) {
    ErrorReturn(ex.what());
  }

  std::unordered_map<int, QString> companyIdToName;
  std::unordered_map<int, QString> userIdToUsername;
  try {
    jobOpeningList = JobOpeningModel::LoadJobOpenings(status, companyId, creatorId);

    for (auto& jo : jobOpeningList) {
      if (!userIdToUsername.count(jo.creatorId)) {
        if (auto userData = UserModel::LoadById(jo.creatorId)) {
          userIdToUsername.emplace(jo.creatorId, userData->username);
        }
        else userIdToUsername.emplace(jo.creatorId, "ERROR USER");
      }

      if (!userIdToUsername.count(jo.statusChangerId)) {
        if (auto userData = UserModel::LoadById(jo.statusChangerId)) {
          userIdToUsername.emplace(jo.statusChangerId, userData->username);
        }
        else userIdToUsername.emplace(jo.statusChangerId, "ERROR USER");
      }

      if (!companyIdToName.count(jo.companyId)) {
        if (auto companyData = CompanyModel::LoadCompanyDataById(jo.companyId)) {
          companyIdToName.emplace(jo.companyId, companyData->companyName);
        }
        else companyIdToName.emplace(jo.companyId, "ERROR COMPANY");
      }
    }
  }
  catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
    jobOpeningList.clear();
    ui->openingsTable->setRowCount(0);
    return;
  }

  static std::unordered_map<JobOpeningModel::JobOpeningStatus, QString> statusIdToStatusString {
    {JobOpeningModel::JobOpeningStatus::Closed, "Closed"},
    {JobOpeningModel::JobOpeningStatus::Posted, "Open"},
  };

  ui->openingsTable->setRowCount(jobOpeningList.size());

  for (int row = 0; row < ui->openingsTable->rowCount(); ++row) {
    auto& elem = jobOpeningList[row];

    ui->openingsTable->setItem(row, 0, new QTableWidgetItem(elem.title));
    ui->openingsTable->setItem(row, 1, new QTableWidgetItem(companyIdToName[elem.companyId]));
    ui->openingsTable->setItem(row, 2, new QTableWidgetItem(elem.createDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->openingsTable->setItem(row, 3, new QTableWidgetItem(userIdToUsername[elem.creatorId]));
    ui->openingsTable->setItem(row, 4, new QTableWidgetItem(statusIdToStatusString[elem.status]));
    ui->openingsTable->setItem(row, 5, new QTableWidgetItem(elem.statusChangeDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->openingsTable->setItem(row, 6, new QTableWidgetItem(userIdToUsername[elem.statusChangerId]));
  }
}

void OpeningsDialog::ShowTableContextMenu(const QPoint &p)
{
  auto item = ui->openingsTable->itemAt(p);
  if (!item) {
    return;
  }

  auto index = ui->openingsTable->indexFromItem(item);
  if (index.row() < 0 || index.row() >= jobOpeningList.size()) {
    return;
  }

  auto& selectedOpening = jobOpeningList[index.row()];

  std::vector<std::unique_ptr<QAction>> actions;

  if (selectedOpening.status == JobOpeningModel::JobOpeningStatus::Posted &&
      CompanyPermissionModel::HasPermission(user.GetUserID(),
                                            selectedOpening.companyId,
                                            CompanyPermissionModel::PermissionID::WorkWithOpenings)) {
    actions.push_back(std::make_unique<QAction>("Close opening", ui->openingsTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedOpening] (bool) {
      try {
        JobOpeningModel::CloseJobOpening(selectedOpening.id, user);
        QMessageBox::information(this, "Info", "Job opening closed");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (selectedOpening.status == JobOpeningModel::JobOpeningStatus::Posted &&
      selectedOpening.creatorId == user.GetUserID()) {
    actions.push_back(std::make_unique<QAction>("Edit opening", ui->openingsTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedOpening] (bool) {
      try {
        auto widget = JobOpeningDialog::EditJobOpeningWidget(user, selectedOpening.id, this);
        widget->exec();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  {
    actions.push_back(std::make_unique<QAction>("View more", ui->openingsTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedOpening] (bool) {
      try {
        auto widget = JobOpeningDialog::ViewJobOpeningWidget(user, selectedOpening.id, this);
        widget->exec();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  {
    actions.push_back(std::make_unique<QAction>("Apply", ui->openingsTable));
    connect(actions.back().get(), &QAction::triggered, [this, &selectedOpening] (bool) {
      try {
        auto widget = std::make_unique<ApplicationDialog>(user, selectedOpening.id, this);
        widget->exec();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (actions.empty()) {
    return;
  }

  QMenu menu(ui->openingsTable);
  menu.setWindowModality(Qt::WindowModality::WindowModal);
  for (auto& action : actions) {
    menu.addAction( action.get() );
  }
  menu.exec(p);
}

