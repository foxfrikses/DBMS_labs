#ifndef MYCREATECOMPANYREQUESTSWIDGET_H
#define MYCREATECOMPANYREQUESTSWIDGET_H

#include <QWidget>


#include "AuthenticatedUser.h"

#include "CompanyModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyCreateCompanyRequestsWidget; }
QT_END_NAMESPACE

class MyCreateCompanyRequestsWidget final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;
  QList<CompanyModel::CreateCompanyRequestData> requestList;

public:
  MyCreateCompanyRequestsWidget(AuthenticatedUser, QWidget *parent = nullptr);
  ~MyCreateCompanyRequestsWidget();

  void Reload();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::MyCreateCompanyRequestsWidget  *ui;
};

#endif // MYCREATECOMPANYREQUESTSWIDGET_H
