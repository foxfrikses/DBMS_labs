#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog final
    : public QDialog
{
  Q_OBJECT

  AuthenticatedUserPtr userPtr;

public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  AuthenticatedUserPtr Login() ;

private slots:
  void on_registerButton_released();

private:
  Ui::LoginDialog  *ui;
};
#endif // LOGINDIALOG_H
