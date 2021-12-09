#include "ApplicationDialog.h"
#include "ui_ApplicationDialog.h"

#include "JobOpeningDialog.h"

#include "ApplicationModel.h"
#include "CompanyModel.h"
#include "JobOpeningModel.h"
#include "UserResumeModel.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QDesktopServices>
#include <QUrl>

#include <filesystem>

ApplicationDialog::ApplicationDialog(
  AuthenticatedUser user,
  JobOpeningID openingId,
  QWidget *parent
)
  : QDialog(parent)
  , user(user)
  , applicationOrOpeningId(openingId)
  , ui(new Ui::ApplicationDialog)
{
  ui->setupUi(this);

  ui->okButton->setText("Create");
  ui->notEditableLables->hide();
  ui->notEditableWidgets->hide();

  connect(ui->okButton, &QPushButton::released, this, &ApplicationDialog::OkReleased);
  connect(ui->attachResumeButton, &QPushButton::released, this, &ApplicationDialog::SelectResumeReleased);
  connect(ui->viewResumeButton, &QPushButton::released, this, &ApplicationDialog::ViewResumeReleased);
  connect(ui->viewJobTitleButton, &QPushButton::released, this, &ApplicationDialog::ViewOpeningReleased);

  Reload();
}

ApplicationDialog::ApplicationDialog(
  AuthenticatedUser user,
  ApplicationID applicationId,
  QWidget *parent
)
  : QDialog(parent)
  , user(user)
  , applicationOrOpeningId(applicationId)
  , ui(new Ui::ApplicationDialog)
{
  ui->setupUi(this);

  ui->okButton->setText("Ok");
  ui->attachResumeButton->hide();

  connect(ui->okButton, &QPushButton::released, this, &ApplicationDialog::OkReleased);
  //connect(ui->attachResumeButton, &QPushButton::released, this, &ApplicationDialog::SelectResumeReleased);
  connect(ui->viewResumeButton, &QPushButton::released, this, &ApplicationDialog::ViewResumeReleased);
  connect(ui->viewJobTitleButton, &QPushButton::released, this, &ApplicationDialog::ViewOpeningReleased);

  Reload();
}

ApplicationDialog::~ApplicationDialog()
{
  delete ui;
}

void ApplicationDialog::OkReleased()
{
  if (std::holds_alternative<JobOpeningID>(applicationOrOpeningId)) {
    try {
      UserResumeModel::InsertUserResumeData rData;
      rData.blob = resume;
      rData.filename = resumeFilename;
      auto resumeId = UserResumeModel::InsertUserResume(rData, user);

      ApplicationModel::PostApplicationData data;
      data.openingId = std::get<JobOpeningID>(applicationOrOpeningId);
      data.resumeId = resumeId;
      ApplicationModel::PostApplication(data, user);

      QMessageBox::information(this, "Info", "Application posted");
    }
    catch (std::exception& ex) {
      QMessageBox::critical(this, "Error", ex.what());
      return;
    }
  }

  close();
}

void ApplicationDialog::ViewOpeningReleased()
{
  JobOpeningID id;

  if (std::holds_alternative<JobOpeningID>(applicationOrOpeningId)) {
    id = std::get<JobOpeningID>(applicationOrOpeningId);
  }
  else {
    auto applicationId = std::get<ApplicationID>(applicationOrOpeningId);
    try {
      auto application = ApplicationModel::LoadApplicationByid(applicationId, user);
      if (!application) {
        throw std::runtime_error("Application with such id is not found");
      }
      id = application->openingId;
    } catch (std::exception& ex) {
      QMessageBox::critical(this, "Error", ex.what());
      return;
    }
  }

  auto widget = JobOpeningDialog::ViewJobOpeningWidget(user, id, this);
  widget->exec();
}

void ApplicationDialog::ViewResumeReleased()
{
  if (resumeFilename.isEmpty() || resume.isEmpty()) {
    return;
  }

  QTemporaryFile file("XXXXXX" + resumeFilename);
  file.setAutoRemove(false);
  if( file.open() ) {
    file.resize(0);
    file.write(resume);
    file.close();
    auto filename = file.fileName();
    auto url = QUrl::fromLocalFile(filename);
    QDesktopServices::openUrl(url);
  }
}

void ApplicationDialog::SelectResumeReleased()
{
  auto fileName = QFileDialog::getOpenFileName(nullptr,
                                              "Open Openings Settings File",
                                              "/home");
  if (fileName.isEmpty()) {
     return;
  }

  QFile settingsFile(fileName);
  if (!settingsFile.open(QIODevice::ReadOnly)) {
    QMessageBox::critical( nullptr, "Error", "Error while opening settings file" );
    return;
  }
  resume = settingsFile.readAll();
  std::filesystem::path p(fileName.toStdString());
  resumeFilename = QString::fromStdString(p.filename().string());
  ui->resumeEdit->setText(resumeFilename);
}

void ApplicationDialog::Reload()
{
  try {
    JobOpeningID openingId;
    if (std::holds_alternative<ApplicationID>(applicationOrOpeningId)) {
      auto id = std::get<ApplicationID>(applicationOrOpeningId);
      auto application = ApplicationModel::LoadApplicationByid(id, user);
      if (!application) {
        throw std::runtime_error("Cannot load specified application");
      }

      ui->applicationDateEdit->setText(application->applicationDate.toString("yyyy-MM-dd hh:ss:mm"));
      switch (application->status) {
        case ApplicationModel::ApplicationStatusID::Accepted:
          ui->statusEdit->setText("Accepted"); break;
        case ApplicationModel::ApplicationStatusID::Cancelled:
          ui->statusEdit->setText("Cancelled"); break;
        case ApplicationModel::ApplicationStatusID::Denied:
          ui->statusEdit->setText("Denied"); break;
        case ApplicationModel::ApplicationStatusID::Posted:
          ui->statusEdit->setText("Posted"); break;
      }
      ui->statusChangeDateEdit->setText(application->statusChangeDate.toString("yyyy-MM-dd hh:ss:mm"));

      auto statusChangerData = UserModel::LoadById(application->statusChangerID);
      if (!statusChangerData) {
        throw std::runtime_error("Cannot load status changer by id");
      }
      ui->statusChangerEdit->setText(statusChangerData->username);

      auto resume = UserResumeModel::LoadUserResume(application->resumeId);
      if (!resume) {
        throw std::runtime_error("Cannot load specified resume");
      }

      this->resume = resume->blob;
      this->resumeFilename = resume->filename;
      ui->resumeEdit->setText(this->resumeFilename);

      auto applicantData = UserModel::LoadById(resume->userId);
      if (!applicantData) {
        throw std::runtime_error("Cannot load applicant by id");
      }
      ui->applicantEdit->setText(applicantData->username);

      openingId = application->openingId;
    }
    else {
      openingId = std::get<JobOpeningID>(applicationOrOpeningId);
    }

    auto opening = JobOpeningModel::LoadJobOpeningById(openingId);
    if (!opening) {
      throw std::runtime_error("Cannot load specified job opening");
    }
    ui->jobTitleEdit->setText(opening->title);

    auto company = CompanyModel::LoadCompanyDataById(opening->companyId);
    if (!company) {
      throw std::runtime_error("Cannot load specified company");
    }
    ui->companyEdit->setText(company->companyName);
  }
  catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
  }
}
