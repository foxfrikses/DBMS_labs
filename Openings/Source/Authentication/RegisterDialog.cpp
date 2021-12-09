#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"

#include <QMessageBox>

#include "UserModel.h"
#include "AdminModel.h"

RegisterDialog::RegisterDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::RegisterDialog)
{
  ui->setupUi(this);

  if (!AdminModel::CanDealWithAdminRights()) {
    ui->isAdminCheckBox->setChecked(false);
    ui->isAdminCheckBox->hide();
  }
}

RegisterDialog::~RegisterDialog()
{
  delete ui;
}

AuthenticatedUserPtr RegisterDialog::RegisterUser()
{
  #define ErrorRet(str) \
  do { \
    QMessageBox::critical( this, "Error while registring a user.", str ); \
    return nullptr; \
  } while (false)

  if (ui->passwordLineEdit->text() !=
      ui->verifyPasswordEdit->text()) {
    ErrorRet( "Passwords doesn't match" );
  }

  if (ui->isAdminCheckBox->isChecked() &&
      !AdminModel::CanDealWithAdminRights()) {
    ErrorRet("No permission to create admin user");
  }

  if (!AuthenticatedUser::Register(ui->usernameLineEdit->text(),
                                   ui->nameEdit->text(),
                                   ui->passwordLineEdit->text())) {
    ErrorRet("Error while registration");
  }

  auto user = UserModel::LoadByUsername(ui->usernameLineEdit->text());
  if (!user) {
    ErrorRet("Error while loading data of registred user");
  }

  if (ui->isAdminCheckBox->isChecked() && !AdminModel::GrantAdminRight(user->id)) {
    UserModel::DeleteUser(user->id, ui->passwordLineEdit->text());
    ErrorRet("Error granting user admin rights");
  }

  auto authUser = AuthenticatedUser::Login(ui->usernameLineEdit->text(),
                                           ui->passwordLineEdit->text());
  if (!authUser) {
    ErrorRet("Error while logging");
  }

  return authUser;
}
