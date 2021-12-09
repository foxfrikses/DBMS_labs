#include "JobOpeningDialog.h"
#include "ui_JobOpeningDialog.h"

#include <QMessageBox>
#include <QObject>

#include "JobOpeningModel.h"
#include "UserModel.h"
#include "CompanyModel.h"
#include "CompanyPermissionModel.h"

JobOpeningDialog::JobOpeningDialog(
  AuthenticatedUser user,
  Mode mode,
  std::optional<JobOpeningID> id,
  QWidget *parent
)
  : QDialog(parent)
  , user(user)
  , id(id)
  , mode(mode)
  , ui(new Ui::JobOpeningDialog)
{
  ui->setupUi(this);

  connect(ui->okButton, &QPushButton::released, this, &JobOpeningDialog::OkReleased);

  if (mode == Mode::create) {
    QList<CompanyModel::CompanyData> companies;
    try {
      companies = CompanyPermissionModel::LoadCompaniesForWhichPermissionExists(
        user.GetUserID(),
        CompanyPermissionModel::PermissionID::WorkWithOpenings
      );
    } catch (std::exception& ex) {
      QMessageBox::critical(this, "Error", ex.what());
    }

    for (auto& company : companies) {
      ui->selectedCompanyComboBox->addItem(company.companyName, int(company.id));
    }

    connect(ui->selectedCompanyComboBox, &QComboBox::activated, this, &JobOpeningDialog::CompanySelected);
  }

  Reload();
}

void JobOpeningDialog::CompanySelected(int index) {
  if (index < 0 || index >= ui->selectedCompanyComboBox->count()) {
    return;
  }

  companyId = CompanyID(ui->selectedCompanyComboBox->itemData(index).toInt());
}

JobOpeningDialog::~JobOpeningDialog()
{
  delete ui;
}

std::unique_ptr<JobOpeningDialog> JobOpeningDialog::CreateJobOpeningWidget(
  AuthenticatedUser user,
  QWidget *parent
)
{
  std::unique_ptr<JobOpeningDialog> widget(new JobOpeningDialog(user, Mode::create, std::nullopt, parent));

  widget->ui->nonEditableAttrs->hide();
  widget->ui->nonEditableAttrsLabels->hide();
  widget->ui->selectedCompanyEdit->hide();

  widget->ui->okButton->setText("Create");

  return widget;
}

std::unique_ptr<JobOpeningDialog> JobOpeningDialog::EditJobOpeningWidget(
  AuthenticatedUser user,
  JobOpeningID jobOpeningId,
  QWidget *parent
)
{
  std::unique_ptr<JobOpeningDialog> widget(new JobOpeningDialog(user, Mode::edit, jobOpeningId, parent));

  widget->ui->okButton->setText("Save");
  widget->ui->selectedCompanyComboBox->hide();

  return widget;
}

std::unique_ptr<JobOpeningDialog> JobOpeningDialog::ViewJobOpeningWidget(
  AuthenticatedUser user,
  JobOpeningID jobOpeningId,
  QWidget *parent
)
{
  std::unique_ptr<JobOpeningDialog> widget(new JobOpeningDialog(user, Mode::view, jobOpeningId, parent));

  widget->ui->selectedCompanyComboBox->hide();
  widget->ui->okButton->setText("Ok");

  widget->ui->descriptionEdit->setReadOnly(true);
  widget->ui->titleEdit->setReadOnly(true);

  return widget;
}

#define ErrorReturn(str) \
  do { \
    QMessageBox::critical(this, "Error", (str)); \
    return; \
  } while(false)

void JobOpeningDialog::OkReleased()
{
  if (mode == Mode::view) {
    close();
    return;
  }

  switch (mode) {
    case Mode::create:
    {
      if (!companyId.has_value()) {
        ErrorReturn("Company is not set");
      }

      JobOpeningModel::JobOpeningCreateData data;
      data.description = ui->descriptionEdit->toPlainText();
      data.title = ui->titleEdit->text();
      data.companyId = companyId.value();

      try {
        JobOpeningModel::CreateJobOpening(data, user);
        QMessageBox::information(this, "Info", "Job opening created");
        close();
        return;
      }
      catch (std::exception& ex) {
        ErrorReturn(ex.what());
      }
    }
      break;

    case Mode::edit:
    {
      if (!id.has_value()) {
        ErrorReturn("Job Opening Id is empty");
      }

      JobOpeningModel::JobOpeningUpdateData data;
      data.id = id.value();
      data.description = ui->descriptionEdit->toPlainText();
      data.title = ui->titleEdit->text();

      try {
        JobOpeningModel::UpdateJobOpening(data, user);
        QMessageBox::information(this, "Info", "Job opening updated");
        close();
        return;
      }
      catch (std::exception& ex) {
        ErrorReturn(ex.what());
      }
    }
      break;

    default:
      break;
  }
}

void JobOpeningDialog::Reload()
{
  if (!id.has_value()) {
    return;
  }

  std::unique_ptr<JobOpeningModel::JobOpeningData> opening;
  std::unique_ptr<CompanyModel::CompanyData> company;
  std::unique_ptr<UserModel::UserData> creator;
  std::unique_ptr<UserModel::UserData> statusChanger;
  try {
    opening = JobOpeningModel::LoadJobOpeningById(id.value());
    if (!opening) {
      ErrorReturn("No opening with such id");
    }

    company = CompanyModel::LoadCompanyDataById(opening->companyId);
    if (!company) {
      ErrorReturn("No company with such id");
    }

    creator = UserModel::LoadById(opening->creatorId);
    if (!company) {
      ErrorReturn("No creator with such id");
    }

    statusChanger = UserModel::LoadById(opening->statusChangerId);
    if (!company) {
      ErrorReturn("No statusChanger with such id");
    }
  }
  catch (std::exception& ex) {
    ErrorReturn(ex.what());
  }

  QString status;
  switch (opening->status) {
    case JobOpeningModel::JobOpeningStatus::Closed:
      status = "Closed";
      break;

    case JobOpeningModel::JobOpeningStatus::Posted:
      status = "Opened";
      break;

    default:
      status = "ERROR STATUS";
  }

  ui->titleEdit->setText(opening->title);
  ui->descriptionEdit->setPlainText(opening->description);
  ui->selectedCompanyEdit->setText(company->companyName);
  ui->createDateEdit->setText(opening->createDate.toString("yyyy-MM-dd hh:ss:mm"));
  ui->creatorEdit->setText(creator->username);
  ui->statusEdit->setText(status);
  ui->statusChangeDateEdit->setText(opening->createDate.toString("yyyy-MM-dd hh:ss:mm"));
  ui->statusChangerEdit->setText(statusChanger->name);
}
