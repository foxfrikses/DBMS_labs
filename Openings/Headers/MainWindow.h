#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow final
    : public QMainWindow
{
  Q_OBJECT

  AuthenticatedUserPtr userPtr;

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  bool IsLoggedIn() const;

private:
  void Clear();
  bool Login();
  void Logout();

private:
  Ui::MainWindow *ui;

private slots:
  void on_logoutButton_released();
  void on_editInfoButton_released();
  void on_createCompanyButton_released();
  void on_createCompanyRequestsButton_released();
  void on_myCreateCompanyRequestsButton_released();
  void on_myOpeningsApplicationsButton_released();
  void on_myApplicationsButton_released();
  void on_openingsButton_released();
  void on_myOpeningsButton_released();
  void on_companyListButton_released();
  void on_userListButton_released();
  void on_exitButton_released();
  void on_createOpening_released();

private:
  enum class Mode {
    None,
    EditInfo,
    CreateCompany,
    CreateCompanyRequests,
    MyCreateCompanyRequests,
    MyOpeningsApplications,
    MyApplications,
    Openings,
    MyOpenings,
    CompanyList,
    UserList,
  } currentMode;

  void SetMode(Mode);
};
#endif // MAINWINDOW_H
