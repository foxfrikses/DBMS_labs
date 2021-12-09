#ifndef APPLICATIONSDIALOG_H
#define APPLICATIONSDIALOG_H

#include <QWidget>

#include "ApplicationModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ApplicationsDialog; }
QT_END_NAMESPACE

class ApplicationsDialog final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;
  QList<ApplicationModel::ApplicationData> applicationList;

  enum class Mode {
    userApplications,
    userOpeningsApplications,
  } mode;

  ApplicationsDialog(AuthenticatedUser, Mode, QWidget *parent = nullptr);

public:
  static std::unique_ptr<ApplicationsDialog> CreateUserApplicationsWidget(AuthenticatedUser, QWidget *parent = nullptr);
  static std::unique_ptr<ApplicationsDialog> CreateUserOpeningsApplicationsWidget(AuthenticatedUser, QWidget *parent = nullptr);

  ~ApplicationsDialog();

private:
  void Reload();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::ApplicationsDialog *ui;
};

#endif // APPLICATIONSDIALOG_H
