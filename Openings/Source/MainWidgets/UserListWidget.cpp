#include "UserListWidget.h"
#include "ui_UserListWidget.h"

#include <QMessageBox>
#include <QAction>
#include <QMenu>

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "CompanyModel.h"
#include "CompanyPermissionModel.h"
#include "UserPermissionModel.h"

UserListWidget::UserListWidget(
  AuthenticatedUser user,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , ui(new Ui::UserListWidget)
{
  ui->setupUi(this);

  ui->userTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->userTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->userTable->setColumnCount(3);
  ui->userTable->setHorizontalHeaderLabels(
    {"Username",
     "Name",
     "Registration date"}
  );

  Reload();
}

void UserListWidget::Reload()
{
  try {
    userDataList = UserModel::LoadUsers();
  }
  catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
    userDataList .clear();
    ui->userTable->setRowCount(0);
    return;
  }

  ui->userTable->setRowCount(userDataList.size());

  for (int row = 0; row < ui->userTable->rowCount(); ++row) {
    auto& elem = userDataList[row];

    ui->userTable->setItem(row, 0, new QTableWidgetItem(elem.username));
    ui->userTable->setItem(row, 1, new QTableWidgetItem(elem.name));
    ui->userTable->setItem(row, 2, new QTableWidgetItem(elem.registrationDate.toString("yyyy-MM-dd hh:ss:mm")));
  }
}

void UserListWidget::ShowTableContextMenu(const QPoint &p)
{
  auto item = ui->userTable->itemAt(p);
  if (!item) {
    return;
  }

  auto index = ui->userTable->indexFromItem(item);
  if (index.row() < 0 || index.row() >= userDataList.size()) {
    return;
  }

  auto& selectedUser = userDataList[index.row()];

  std::vector<std::unique_ptr<QAction>> userPermissionsActions;
  if (UserPermissionModel::CanGrantOrRevokePermission(user.GetUserID(), UserPermissionModel::PermissionID::AcceptCompanyRequest)) {
    userPermissionsActions.push_back(std::make_unique<QAction>("View, accept and deny createCompanyRequest", ui->userTable));
    auto& action = userPermissionsActions.back();

    bool hasPermission = UserPermissionModel::HasPermission(selectedUser.id, UserPermissionModel::PermissionID::AcceptCompanyRequest);
    action->setCheckable(true);
    action->setChecked(hasPermission);

    auto selectedUserId = selectedUser.id;

    connect(action.get(), &QAction::triggered, [this, selectedUserId] (bool set) {
      try {
        if (set) {
          UserPermissionModel::GrantPermission(user, selectedUserId, UserPermissionModel::PermissionID::AcceptCompanyRequest);
          QMessageBox::information(this, "Info", "Permission granted");
        }
        else {
          UserPermissionModel::RevokePermission(user, selectedUserId, UserPermissionModel::PermissionID::AcceptCompanyRequest);
          QMessageBox::information(this, "Info", "Permission revoken");
        }
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  std::vector<std::unique_ptr<QAction>> workWithOpeningsActions;
  for (auto& company : CompanyModel::LoadCompaniesAdministratedBy(user.GetUserID())) {
    workWithOpeningsActions.push_back(std::make_unique<QAction>(company.companyName, ui->userTable));
    auto& action = workWithOpeningsActions.back();

    bool hasPermission = CompanyPermissionModel::HasPermission(selectedUser.id, company.id, CompanyPermissionModel::PermissionID::WorkWithOpenings);
    action->setCheckable(true);
    action->setChecked(hasPermission);

    auto companyId = company.id;
    auto selectedUserId = selectedUser.id;
    connect(action.get(), &QAction::triggered, [this, companyId, selectedUserId] (bool set) {
      try {
        if (set) {
          CompanyPermissionModel::GrantPermission(user, selectedUserId, companyId, CompanyPermissionModel::PermissionID::WorkWithOpenings);
          QMessageBox::information(this, "Info", "Permission granted");
        }
        else {
          CompanyPermissionModel::RevokePermission(user, selectedUserId, companyId, CompanyPermissionModel::PermissionID::WorkWithOpenings);
          QMessageBox::information(this, "Info", "Permission revoken");
        }
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (userPermissionsActions.empty() && workWithOpeningsActions.empty()) {
    return;
  }

  auto userPermissionsMenu = std::make_unique<QMenu>(ui->userTable);
  userPermissionsMenu->setTitle("User permissions");
  for (auto& action : userPermissionsActions) {
    userPermissionsMenu->addAction(action.get());
  }

  auto workWithOpeningsMenu = std::make_unique<QMenu>(ui->userTable);
  workWithOpeningsMenu->setTitle("Work with company openings permissions");
  for (auto& action : workWithOpeningsActions) {
    workWithOpeningsMenu->addAction(action.get());
  }

  QMenu menu(ui->userTable);
  menu.setWindowModality(Qt::WindowModality::WindowModal);

  if( !userPermissionsActions.empty() ) {
    menu.addMenu(userPermissionsMenu.get());
  }

  if( !workWithOpeningsActions.empty() ) {
    menu.addMenu(workWithOpeningsMenu.get());
  }

  menu.exec(p);
}

UserListWidget::~UserListWidget()
{
  delete ui;
}

