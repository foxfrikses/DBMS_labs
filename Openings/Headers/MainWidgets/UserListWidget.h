#ifndef USERLISTWIDGET_H
#define USERLISTWIDGET_H

#include <QWidget>

#include "UserModel.h"
#include "AuthenticatedUser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class UserListWidget; }
QT_END_NAMESPACE

class UserListWidget final
    : public QWidget
{
  Q_OBJECT

  AuthenticatedUser user;
  QList<UserModel::UserData> userDataList;

  void Reload();

public:
  UserListWidget(AuthenticatedUser, QWidget *parent = nullptr);
  ~UserListWidget();

private slots:
  void ShowTableContextMenu(const QPoint &p);

private:
  Ui::UserListWidget  *ui;
};

#endif // USERLISTWIDGET_H
