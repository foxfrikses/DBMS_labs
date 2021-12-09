#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QFile>
#include <QFileDialog>

#include <optional>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  {
    auto fileName = QFileDialog::getOpenFileName(nullptr,
                                                 "Open Openings Settings File",
                                                 "/home",
                                                 "JSON (*.json)");
    if (fileName.isEmpty()) {
      return 0;
    }

    /*
    {
      "host" : "",
      "databaseName" : "",
      "username" : "",
      "password" : "",
      "port" : ""
    }
    */

    QFile settingsFile(fileName);
    if (!settingsFile.open(QIODevice::ReadOnly)) {
      QMessageBox::critical( nullptr, "Error", "Error while opening settings file" );
      return -1;
    }
    QByteArray settingsBlob = settingsFile.readAll();

    auto settingsJson = QJsonDocument::fromJson(settingsBlob);
    if (settingsJson.isNull() ||
        settingsJson.isEmpty() ||
        !settingsJson.isObject() ) {
      QMessageBox::critical( nullptr, "Error", "Incorrect format of settings file" );
      return -1;
    }

    auto settingsObject = settingsJson.object();

    auto host = settingsObject["host"];
    auto databaseName = settingsObject["databaseName"];
    auto username = settingsObject["username"];
    auto password = settingsObject["password"];
    auto port = settingsObject["port"];

    if (host.isNull() || !host.isString() ||
        databaseName.isNull() || !databaseName.isString() ||
        username.isNull() || !username.isString() ||
        password.isNull() || !password.isString() ||
        port.isNull() || !port.isString()) {
      QMessageBox::critical( nullptr, "Error", "Incorrect format of settings object" );
      return -1;
    }

    auto db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host.toString());
    db.setDatabaseName(databaseName.toString());
    db.setUserName(username.toString());
    db.setPort(port.toString().toInt());
    db.setPassword(password.toString());

    //db.setHostName("localhost");
    //db.setDatabaseName("openings_db");
    //db.setUserName("openings_app");
    //db.setPort(5432);
    //db.setPassword("password");

    if (!db.open()){
      QMessageBox::critical( nullptr,
                             "Error while connection to the database.",
                             db.lastError().text() );
      return -1;
    }
  }

  try {
    MainWindow w;
    if (w.IsLoggedIn()) {
      w.show();
    }
    return a.exec();
  }
  catch (std::exception& ex) {
     QMessageBox::critical( nullptr,
                           "Error. Openings failed.",
                            ex.what() );
     return -1;
  }
  catch (...) {
     QMessageBox::critical( nullptr,
                           "Error. Openings failed.",
                           "Undefined exception type." );
     return -2;
  }
}
