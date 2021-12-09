#include "LoginDialog.h"
#include "ui_LoginDialog.h"

#include "RegisterDialog.h"

#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LoginDialog)
{
  ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
  delete ui;
}

AuthenticatedUserPtr LoginDialog::Login()
{
  if (!userPtr) {
    try {
      userPtr = AuthenticatedUser::Login(ui->usernameLineEdit->text(),
                                         ui->passwordLineEdit->text());
    } catch (std::exception& ex) {
      QMessageBox::critical( this,
                             "Error while logging.",
                             ex.what() );
    }
  }

  return std::move(userPtr);
}

void LoginDialog::on_registerButton_released()
{
  RegisterDialog d(this);
  d.setModal(true);

  do {
    d.show();
    d.exec();
    if (!d.result()) {
      return;
    }
    userPtr = d.RegisterUser();
  } while(!userPtr);

  accept();
}

