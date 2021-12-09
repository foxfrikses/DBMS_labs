#include "CreateCompanyWidget.h"
#include "ui_CreateCompanyWidget.h"

#include <QMessageBox>

#include "CompanyModel.h"

CreateCompanyWidget::CreateCompanyWidget(
  AuthenticatedUser user,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , ui(new Ui::CreateCompanyWidget)
{
  ui->setupUi(this);
}

CreateCompanyWidget::~CreateCompanyWidget()
{
  delete ui;
}

void CreateCompanyWidget::on_sendRequestButton_released()
{
  if (ui->companyNameEdit->text().isEmpty()) {
    return;
  }

  try {
    CompanyModel::RequestCreateCompany(ui->companyNameEdit->text(),
                                       user);
    QMessageBox::information(this, "Info", "Request created");
    ui->companyNameEdit->setText("");
  } catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
  }
}

