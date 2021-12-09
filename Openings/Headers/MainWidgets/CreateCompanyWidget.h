#ifndef CREATECOMPANYWIDGET_H
#define CREATECOMPANYWIDGET_H

#include <QWidget>

#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CreateCompanyWidget; }
QT_END_NAMESPACE

class CreateCompanyWidget final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;

public:
  CreateCompanyWidget(AuthenticatedUser, QWidget *parent = nullptr);
  ~CreateCompanyWidget();

private slots:
  void on_sendRequestButton_released();

private:
  Ui::CreateCompanyWidget  *ui;
};

#endif // CREATECOMPANYWIDGET_H
