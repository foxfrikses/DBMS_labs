#include "EditUserInfoWidget.h"
#include "ui_EditUserInfoWidget.h"

#include <QMessageBox>

EditUserInfoWidget::EditUserInfoWidget(
  UserID userId,
  QWidget *parent
)
  : QWidget(parent)
  , userData(UserModel::LoadById(userId))
  , ui(new Ui::EditUserInfoWidget)
{
  ui->setupUi(this);

  if (!userData) {
    ui->saveChangesButton->setDisabled(true);
    return;
  }

  ui->usernameEdit->setText(userData->username);
  ui->nameEdit->setText(userData->name);
  auto textDate = userData->registrationDate.toString("yyyy-MM-dd hh:ss:mm");
  ui->registrationDateEdit->setText(textDate);
}

EditUserInfoWidget::~EditUserInfoWidget()
{
  delete ui;
}

void EditUserInfoWidget::on_saveChangesButton_released()
{
  #define WarningReturn(str) \
    do { \
      QMessageBox::warning(this, "Worning", str); \
      return; \
    } while (false)

  #define ErrorReturn(str) \
    do { \
      QMessageBox::critical(this, "Error", str); \
      return; \
    } while (false)

  if (ui->usernameEdit->text() == userData->username &&
      ui->nameEdit->text() == userData->name &&
      !ui->changePasswordGroupBox->isChecked()) {
    return;
  }

  if (ui->changePasswordGroupBox->isChecked()) {
    if (ui->newPasswordEdit->text() != ui->confirmNewPasswordEdit->text()) {
      WarningReturn("Passwords don't match");
    }
  }

  if (ui->usernameEdit->text() != userData->username ||
      ui->nameEdit->text() != userData->name )
  {
    auto newUserData = std::make_unique<UserModel::UserData>(*userData);
    newUserData->name = ui->nameEdit->text();
    newUserData->username = ui->usernameEdit->text();
    try {
      UserModel::UpdateUserData(*newUserData, ui->currentPasswordEdit->text());
      userData = std::move(newUserData);
      QMessageBox::information(this, "Info", "User data updated");
    } catch (std::exception& ex) {
      ErrorReturn(ex.what());
    }
  }

  if (ui->changePasswordGroupBox->isChecked()) {
    try {
      UserModel::UpdatePassword(userData->id,
                                ui->currentPasswordEdit->text(),
                                ui->newPasswordEdit->text());
      QMessageBox::information(this, "Info", "Password updated");
    } catch (std::exception& ex) {
      ErrorReturn(ex.what());
    }
    ui->changePasswordGroupBox->setChecked(false);
    ui->newPasswordEdit->setText("");
    ui->confirmNewPasswordEdit->setText("");
  }

  ui->currentPasswordEdit->setText("");
}

