#ifndef JOBOPENINGDIALOG_H
#define JOBOPENINGDIALOG_H

#include <QDialog>

#include <memory>

#include "Common.h"
#include "UserModel.h"

#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class JobOpeningDialog; }
QT_END_NAMESPACE

class JobOpeningDialog final
    : public QDialog
{
  Q_OBJECT

  AuthenticatedUser user;
  std::optional<JobOpeningID> id;
  std::optional<CompanyID> companyId;

  enum Mode {
    create,
    edit,
    view
  } mode;

  JobOpeningDialog(AuthenticatedUser user, Mode, std::optional<JobOpeningID>, QWidget *parent = nullptr);

public:
  static std::unique_ptr<JobOpeningDialog> CreateJobOpeningWidget(AuthenticatedUser user, QWidget *parent = nullptr);
  static std::unique_ptr<JobOpeningDialog> EditJobOpeningWidget(AuthenticatedUser user, JobOpeningID, QWidget *parent = nullptr);
  static std::unique_ptr<JobOpeningDialog> ViewJobOpeningWidget(AuthenticatedUser user, JobOpeningID, QWidget *parent = nullptr);

  ~JobOpeningDialog();

private slots:
  void OkReleased();
  void CompanySelected(int index);

private:
  void Reload();

private:
  Ui::JobOpeningDialog *ui;
};

#endif // JOBOPENINGDIALOG_H
