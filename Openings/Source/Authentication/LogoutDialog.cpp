#include "LogoutDialog.h"
#include "ui_LogoutDialog.h"

LogoutDialog::LogoutDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::LogoutDialog)
{
  ui->setupUi(this);
}

LogoutDialog::~LogoutDialog()
{
  delete ui;
}

