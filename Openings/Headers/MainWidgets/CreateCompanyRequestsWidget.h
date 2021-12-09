#ifndef CREATECOMPANYREQUESTSWIDGET_H
#define CREATECOMPANYREQUESTSWIDGET_H

#include <QWidget>

#include "AuthenticatedUser.h"

#include "CompanyModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CreateCompanyRequestsWidget; }
QT_END_NAMESPACE

class CreateCompanyRequestsWidget final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;
  QList<CompanyModel::CreateCompanyRequestData> requestList;

public:
  CreateCompanyRequestsWidget(AuthenticatedUser, QWidget *parent = nullptr);
  ~CreateCompanyRequestsWidget();

  void Reload();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::CreateCompanyRequestsWidget  *ui;
};

#endif // CREATECOMPANYREQUESTSWIDGET_H
