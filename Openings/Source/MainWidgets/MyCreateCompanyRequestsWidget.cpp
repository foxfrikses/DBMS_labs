#include "MyCreateCompanyRequestsWidget.h"
#include "ui_MyCreateCompanyRequestsWidget.h"

#include "UserModel.h"
#include "CompanyModel.h"

#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <vector>

MyCreateCompanyRequestsWidget::MyCreateCompanyRequestsWidget(
  AuthenticatedUser user,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , ui(new Ui::MyCreateCompanyRequestsWidget)
{
  ui->setupUi(this);

  ui->companyRequestsTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->companyRequestsTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->companyRequestsTable->setColumnCount(5);
  ui->companyRequestsTable->setHorizontalHeaderLabels(
    {"Company name",
     "Request date",
     "Status",
     "Status change date",
     "Status changer"}
  );

  Reload();
}

void MyCreateCompanyRequestsWidget::Reload()
{
  std::unordered_map<int, QString> userIdToUsername;

  try {
    requestList = CompanyModel::LoadUserCreateCompanyRequests(user);

    for (auto& req : requestList) {
      if (!userIdToUsername.count(req.statusChangerId)) {
        if (auto userData = UserModel::LoadById(req.statusChangerId)) {
          userIdToUsername.emplace(req.statusChangerId,
                                   userData->username);
        }
        else userIdToUsername.emplace(req.statusChangerId, "ERROR USER");
      }
    }
  } catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
    requestList.clear();
    ui->companyRequestsTable->setRowCount(0);
    return;
  }

  ui->companyRequestsTable->setRowCount(requestList.size());

  static std::unordered_map<CreateCompanyRequestStatus, QString> statusIdToStatusString {
    {CreateCompanyRequestStatus::Accepted, "Accepted"},
    {CreateCompanyRequestStatus::Cancelled, "Cancelled"},
    {CreateCompanyRequestStatus::Denied, "Denied"},
    {CreateCompanyRequestStatus::Posted, "Posted"},
  };

  for (int row = 0; row < ui->companyRequestsTable->rowCount(); ++row) {
    auto& elem = requestList[row];

    ui->companyRequestsTable->setItem(row, 0, new QTableWidgetItem(elem.companyName));
    ui->companyRequestsTable->setItem(row, 1, new QTableWidgetItem(elem.requestDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->companyRequestsTable->setItem(row, 2, new QTableWidgetItem(statusIdToStatusString[elem.status]));
    ui->companyRequestsTable->setItem(row, 3, new QTableWidgetItem(elem.statusChangeDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->companyRequestsTable->setItem(row, 4, new QTableWidgetItem(userIdToUsername[elem.statusChangerId]));
  }
}

void MyCreateCompanyRequestsWidget::ShowTableContextMenu(const QPoint &p)
{
  auto item = ui->companyRequestsTable->itemAt(p);
  if (!item) {
    return;
  }

  auto index = ui->companyRequestsTable->indexFromItem(item);
  if (index.row() < 0 || index.row() >= requestList.size()) {
    return;
  }

  std::vector<std::unique_ptr<QAction>> actions;

  if (requestList[index.row()].status == CreateCompanyRequestStatus::Posted) {
    actions.push_back(std::make_unique<QAction>("Cancel request", ui->companyRequestsTable));
    connect(actions.back().get(), &QAction::triggered, [this, index] (bool) {
      try{
        CompanyModel::CancelCreateCompanyRequest(requestList[index.row()].id, user);
        QMessageBox::information(this, "Info", "Request was cancelled");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (actions.empty()) {
    return;
  }

  QMenu menu(ui->companyRequestsTable);
  menu.setWindowModality(Qt::WindowModality::WindowModal);
  for (auto& action : actions) {
    menu.addAction( action.get() );
  }
  menu.exec(p);
}


MyCreateCompanyRequestsWidget::~MyCreateCompanyRequestsWidget()
{
  delete ui;
}

