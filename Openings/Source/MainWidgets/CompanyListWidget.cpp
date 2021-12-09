#include "CompanyListWidget.h"
#include "ui_CompanyListWidget.h"

#include "OpeningsDialog.h"

#include <QMessageBox>
#include <QMenu>
#include <QAction>

#include "UserModel.h"

CompanyListWidget::CompanyListWidget(
  AuthenticatedUser user,
  QWidget *parent
)
  : QWidget(parent)
  , user(user)
  , ui(new Ui::CompanyListWidget)
{
  ui->setupUi(this);

  ui->companyTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->companyTable, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(ShowTableContextMenu(const QPoint &)));

  ui->companyTable->setColumnCount(2);
  ui->companyTable->setHorizontalHeaderLabels(
    {"Company name",
     "Admin"}
  );

  Reload();
}


void CompanyListWidget::ShowTableContextMenu(const QPoint &p)
{
  auto item = ui->companyTable->itemAt(p);
  if (!item) {
    return;
  }

  auto index = ui->companyTable->indexFromItem(item);
  if (index.row() < 0 || index.row() >= companyList.size()) {
    return;
  }

  auto& company = companyList[index.row()];

  std::vector<std::unique_ptr<QAction>> actions;

  {
    actions.push_back(std::make_unique<QAction>("View openings", ui->companyTable));
    connect(actions.back().get(), &QAction::triggered, [this, &company] (bool) {
      try {
        auto widget = OpeningsDialog::CreateCompanyOpenOpeningsWidget(user,company.id, this);
        widget->exec();
      }
      catch (std::exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
      }
    });
  }

  if (actions.empty()) {
    return;
  }

  QMenu menu(ui->companyTable);
  menu.setWindowModality(Qt::WindowModality::WindowModal);
  for (auto& action : actions) {
    menu.addAction( action.get() );
  }
  menu.exec(p);
}

void CompanyListWidget::Reload()
{
  std::unordered_map<int, QString> userIdToUsername;

  try {
    companyList = CompanyModel::LoadCompanies();

    for (auto& company : companyList ) {
      if (!userIdToUsername.count(company.companyAdmin)) {
        if (auto userData = UserModel::LoadById(company.companyAdmin)) {
          userIdToUsername.emplace(company.companyAdmin,
                                   userData->username);
        }
        else userIdToUsername.emplace(company.companyAdmin, "ERROR USER");
      }
    }
  } catch (std::exception& ex) {
    QMessageBox::critical(this, "Error", ex.what());
    companyList.clear();
    ui->companyTable->setRowCount(0);
    return;
  }

  ui->companyTable->setRowCount(companyList.size());

  for (int row = 0; row < ui->companyTable->rowCount(); ++row) {
    auto& elem = companyList[row];

    ui->companyTable->setItem(row, 0, new QTableWidgetItem(elem.companyName));
    ui->companyTable->setItem(row, 1, new QTableWidgetItem(userIdToUsername[elem.companyAdmin]));
  }
}

CompanyListWidget::~CompanyListWidget()
{
  delete ui;
}

