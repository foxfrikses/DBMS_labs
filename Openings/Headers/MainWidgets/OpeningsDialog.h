#ifndef OPENINGSDIALOG_H
#define OPENINGSDIALOG_H

#include <QDialog>

#include "Common.h"
#include "AuthenticatedUser.h"
#include "JobOpeningModel.h"

#include <optional>

QT_BEGIN_NAMESPACE
namespace Ui { class OpeningsDialog; }
QT_END_NAMESPACE

class OpeningsDialog final
    : public QDialog
{
  Q_OBJECT

  AuthenticatedUser user;
  std::optional<CompanyID> companyId;

  QList<JobOpeningModel::JobOpeningData> jobOpeningList;

  enum class Mode {
    userOpenings,
    companyOpenOpenings,
    openOpenings
  } mode;

  OpeningsDialog(AuthenticatedUser, Mode, std::optional<CompanyID>, QWidget *parent = nullptr);

public:
  static std::unique_ptr<OpeningsDialog> CreateCompanyOpenOpeningsWidget(AuthenticatedUser, CompanyID id, QWidget *parent = nullptr);
  static std::unique_ptr<OpeningsDialog> CreateUserOpeningsWidget(AuthenticatedUser, QWidget *parent = nullptr);
  static std::unique_ptr<OpeningsDialog> CreateOpenOpeningsWidget(AuthenticatedUser, QWidget *parent = nullptr);

  ~OpeningsDialog();

private:
  void Reload();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::OpeningsDialog  *ui;
};

#endif // OPENINGSDIALOG_H
