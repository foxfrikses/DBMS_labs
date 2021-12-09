#include "CreateCompanyRequestsWidget.h"
#include "ui_CreateCompanyRequestsWidget.h"

#include <QTableWidgetItem>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>

#include <unordered_map>
#include <memory>
#include <vector>

#include "CompanyModel.h"
#include "UserModel.h"

CreateCompanyRequestsWidget::CreateCompanyRequestsWidget(
  AuthenticatedUser user,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , ui(new Ui::CreateCompanyRequestsWidget)
{
  ui->setupUi(this);

  ui->companyRequestsTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->companyRequestsTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->companyRequestsTable->setColumnCount(6);
  ui->companyRequestsTable->setHorizontalHeaderLabels(
    {"Company name",
     "Requester",
     "Request date",
     "Status",
     "Status change date",
     "Status changer"}
  );

  Reload();
}

void CreateCompanyRequestsWidget::Reload()
{
  std::unordered_map<int, QString> userIdToUsername;

  try {
    requestList = CompanyModel::LoadCreateCompanyRequests(user);

    for (auto& req : requestList) {
      if (!userIdToUsername.count(req.requesterId)) {
        if (auto userData = UserModel::LoadById(req.requesterId)) {
          userIdToUsername.emplace(req.requesterId,
                                   userData->username);
      }
        else userIdToUsername.emplace(req.requesterId, "ERROR USER");
      }

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
    ui->companyRequestsTable->setItem(row, 1, new QTableWidgetItem(userIdToUsername[elem.requesterId]));
    ui->companyRequestsTable->setItem(row, 2, new QTableWidgetItem(elem.requestDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->companyRequestsTable->setItem(row, 3, new QTableWidgetItem(statusIdToStatusString[elem.status]));
    ui->companyRequestsTable->setItem(row, 4, new QTableWidgetItem(elem.statusChangeDate.toString("yyyy-MM-dd hh:ss:mm")));
    ui->companyRequestsTable->setItem(row, 5, new QTableWidgetItem(userIdToUsername[elem.statusChangerId]));
  }
}

CreateCompanyRequestsWidget::~CreateCompanyRequestsWidget()
{
  delete ui;
}

void CreateCompanyRequestsWidget::ShowTableContextMenu(const QPoint &p)
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

  if (requestList[index.row()].status == CreateCompanyRequestStatus::Posted ||
      requestList[index.row()].status == CreateCompanyRequestStatus::Denied) {
    actions.push_back(std::make_unique<QAction>("Accept request", ui->companyRequestsTable));
    connect(actions.back().get(), &QAction::triggered, [this, index] (bool) {
      try{
        CompanyModel::AcceptCreateCompanyRequest(requestList[index.row()].id, user);
        QMessageBox::information(this, "Info", "Request was accepted");
        Reload();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (requestList[index.row()].status == CreateCompanyRequestStatus::Posted) {
    actions.push_back(std::make_unique<QAction>("Deny request", ui->companyRequestsTable));
    connect(actions.back().get(), &QAction::triggered, [this, index] (bool) {
      try{
        CompanyModel::DenyCreateCompanyRequest(requestList[index.row()].id, user);
        QMessageBox::information(this, "Info", "Request was denied");
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






