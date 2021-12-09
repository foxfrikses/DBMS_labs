#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterDialog; }
QT_END_NAMESPACE

class RegisterDialog final
    : public QDialog
{
  Q_OBJECT

public:
  RegisterDialog(QWidget *parent = nullptr);
  ~RegisterDialog();

  AuthenticatedUserPtr RegisterUser();

private:
  Ui::RegisterDialog  *ui;
};
#endif // REGISTERDIALOG_H
