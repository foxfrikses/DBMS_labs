#ifndef LOGOUTDIALOG_H
#define LOGOUTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class LogoutDialog; }
QT_END_NAMESPACE

class LogoutDialog final
    : public QDialog
{
  Q_OBJECT

public:
  LogoutDialog(QWidget *parent = nullptr);
  ~LogoutDialog();

private:
  Ui::LogoutDialog  *ui;
};
#endif // LOGOUTDIALOG_H
