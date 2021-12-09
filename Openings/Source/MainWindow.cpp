#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "LoginDialog.h"
#include "LogoutDialog.h"

#include "EditUserInfoWidget.h"
#include "CreateCompanyWidget.h"
#include "CompanyListWidget.h"
#include "CreateCompanyRequestsWidget.h"
#include "ApplicationsDialog.h"
#include "MyCreateCompanyRequestsWidget.h"
#include "OpeningsDialog.h"
#include "UserListWidget.h"
#include "JobOpeningDialog.h"

#include "UserPermissionModel.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>

#include <stdexcept>

bool MainWindow::IsLoggedIn() const
{
  return userPtr != nullptr;
}

bool MainWindow::Login()
{
  auto login = std::make_shared<LoginDialog>(this);
  login->setModal(true);

  do {
    login->show();
    login->exec();
    if (!login->result()) {
      return false;
    }

    userPtr = login->Login();
  } while(!userPtr);

  ui->createCompanyRequestsButton->setHidden(
    !UserPermissionModel::HasPermission(userPtr->GetUserID(),
                                        UserPermissionModel::PermissionID::AcceptCompanyRequest)
  );

  return true;
}

void MainWindow::Logout()
{
  Clear();

  hide();

  if (!Login()) {
    close();
    return;
  }

  show();
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  if( !Login() ) {
    close();
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}

void ClearLayout(QLayout* layout, bool deleteWidgets = true)
{
  while (QLayoutItem* item = layout->takeAt(0))
  {
    auto widget = item->widget();

    if (auto childLayout = item->layout()) {
       ClearLayout(childLayout, deleteWidgets);
    }
    delete item;

    if (deleteWidgets) {
      delete widget;
    }
  }
}

void MainWindow::Clear()
{
  SetMode(Mode::None);
  userPtr.reset();
}

void MainWindow::SetMode(
  Mode mode
)
{
  if (!userPtr && mode != Mode::None) {
    return;
  }

  if (mode == currentMode) {
    return;
  }

  std::unique_ptr<QWidget> widget;

  switch (mode) {
    case Mode::EditInfo:
      widget = std::make_unique<EditUserInfoWidget>(userPtr->GetUserID());
      break;

    case Mode::CreateCompany:
      widget = std::make_unique<CreateCompanyWidget>(*userPtr);
      break;

    case Mode::CreateCompanyRequests:
      widget = std::make_unique<CreateCompanyRequestsWidget>(*userPtr);
      break;

    case Mode::MyCreateCompanyRequests:
      widget = std::make_unique<MyCreateCompanyRequestsWidget>(*userPtr);
      break;

    case Mode::MyOpeningsApplications:
      widget = ApplicationsDialog::CreateUserOpeningsApplicationsWidget(*userPtr);
      break;

    case Mode::MyApplications:
      widget = ApplicationsDialog::CreateUserApplicationsWidget(*userPtr);
      break;

    case Mode::Openings:
      widget = OpeningsDialog::CreateOpenOpeningsWidget(*userPtr);
      break;

    case Mode::MyOpenings:
      widget = OpeningsDialog::CreateUserOpeningsWidget(*userPtr);
      break;

    case Mode::CompanyList:
      widget = std::make_unique<CompanyListWidget>(*userPtr);
      break;

    case Mode::UserList:
      widget = std::make_unique<UserListWidget>(*userPtr);
      break;

    case Mode::None:
      break;

    default:
      QMessageBox::critical( nullptr, "Error", "Unknown command");
      return SetMode(Mode::None);
  };

  auto layout = ui->activeWidgetFrame->layout();
  ClearLayout(layout);
  if (widget) {
    layout->addWidget(widget.release());
  }
  currentMode = mode;
}

void MainWindow::on_exitButton_released()
{
  close();
}

void MainWindow::on_logoutButton_released()
{
  if (!userPtr) {
    return;
  }

  LogoutDialog d(this);
  d.setModal(true);
  d.show();
  d.exec();
  if (d.result() == 0) {
    return;
  }

  Logout();
}

void MainWindow::on_editInfoButton_released()
{
  SetMode(Mode::EditInfo);
}

void MainWindow::on_createCompanyButton_released()
{
  SetMode(Mode::CreateCompany);
}

void MainWindow::on_createCompanyRequestsButton_released()
{
  SetMode(Mode::CreateCompanyRequests);
}

void MainWindow::on_myCreateCompanyRequestsButton_released()
{
  SetMode(Mode::MyCreateCompanyRequests);
}

void MainWindow::on_myOpeningsApplicationsButton_released()
{
  SetMode(Mode::MyOpeningsApplications);
}

void MainWindow::on_myApplicationsButton_released()
{
  SetMode(Mode::MyApplications);
}

void MainWindow::on_openingsButton_released()
{
  SetMode(Mode::Openings);
}

void MainWindow::on_myOpeningsButton_released()
{
  SetMode(Mode::MyOpenings);
}

void MainWindow::on_companyListButton_released()
{
  SetMode(Mode::CompanyList);
}

void MainWindow::on_userListButton_released()
{
  SetMode(Mode::UserList);
}

void MainWindow::on_createOpening_released()
{
  auto widget = JobOpeningDialog::CreateJobOpeningWidget(*userPtr, this);
  widget->exec();
}

