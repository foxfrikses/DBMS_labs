#ifndef APPLICATIONDIALOG_H
#define APPLICATIONDIALOG_H

#include <QDialog>

#include <memory>
#include <variant>

#include "Common.h"
#include "UserModel.h"

#include "AuthenticatedUser.h"

#include <QTemporaryFile>

QT_BEGIN_NAMESPACE
namespace Ui { class ApplicationDialog; }
QT_END_NAMESPACE

class ApplicationDialog final
  : public QDialog
{
  Q_OBJECT

  AuthenticatedUser user;
  std::variant<ApplicationID, JobOpeningID> applicationOrOpeningId;
  QString resumeFilename;
  QByteArray resume;
  QTemporaryFile file;

public:
  ApplicationDialog(AuthenticatedUser user, ApplicationID, QWidget *parent = nullptr); // to view application
  ApplicationDialog(AuthenticatedUser user, JobOpeningID, QWidget *parent = nullptr); // to create application

  ~ApplicationDialog();

private slots:
  void OkReleased();
  void ViewOpeningReleased();
  void ViewResumeReleased();
  void SelectResumeReleased();

private:
  void Reload();

private:
  Ui::ApplicationDialog *ui;
};

#endif // APPLICATIONDIALOG_H
