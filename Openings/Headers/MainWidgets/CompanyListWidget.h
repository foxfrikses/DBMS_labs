#ifndef COMPANYLISTWIDGET_H
#define COMPANYLISTWIDGET_H

#include <QWidget>

#include "AuthenticatedUser.h"
#include "CompanyModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CompanyListWidget; }
QT_END_NAMESPACE

class CompanyListWidget final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;
  QList<CompanyModel::CompanyData> companyList;

public:
  CompanyListWidget(AuthenticatedUser, QWidget *parent = nullptr);
  ~CompanyListWidget();

  void Reload();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::CompanyListWidget  *ui;
};

#endif // COMPANYLISTWIDGET_H
