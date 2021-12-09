#ifndef EDITUSERINFOWIDGET_H
#define EDITUSERINFOWIDGET_H

#include <QWidget>

#include "Common.h"
#include "UserModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EditUserInfoWidget; }
QT_END_NAMESPACE

class EditUserInfoWidget final
    : public QWidget
{
  Q_OBJECT

  std::unique_ptr<UserModel::UserData> userData;

public:
  EditUserInfoWidget(UserID userId, QWidget *parent = nullptr);
  ~EditUserInfoWidget();

private slots:
  void on_saveChangesButton_released();

private:
  Ui::EditUserInfoWidget  *ui;
};

#endif // EDITUSERINFOWIDGET_H
