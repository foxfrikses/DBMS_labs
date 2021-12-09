#include "UserModel.h"

#include <QSqlQuery>

#include <QCryptographicHash>

namespace  {
  void EnsureUsernameSizeCorrect(
    QString username
  )
  {
    if (username.size() > USER_USERNAME_SIZE) {
      throw std::runtime_error("Username must be less then " +
                               std::to_string(USER_USERNAME_SIZE) +
                               " symbols");
    }
    if (username.isEmpty() ) {
      throw std::runtime_error("Username cannot be empty");
    }
  }

  void EnsureNameSizeCorrect(
    QString name
  )
  {
    if (name.size() > USER_NAME_SIZE) {
      throw std::runtime_error("Name must be less then " +
                               std::to_string(USER_NAME_SIZE) +
                               " symbols");
    }
  }

  void EnsureCorrectPassword(
    UserID userId,
    QString password
  )
  {
    if (!UserModel::VerifyPassword(userId, password)) {
      throw std::runtime_error("Incorrect password");
    }
  }

  auto GetDefaultHashAlg() {
    return QCryptographicHash::Sha256;
  };

  auto ComputePasswordHash(
    QString password,
    QCryptographicHash::Algorithm alg
  )
  {
    QCryptographicHash hasher(alg);
    auto wstr = password.toStdU32String();
    hasher.addData(reinterpret_cast<char*>(wstr.data()), wstr.length());
    return hasher.result();
  }
}

namespace UserModel {
  std::unique_ptr<UserData> InsertUser(
    const InsertUserData& insertUserData,
    QString password
  )
  {
    EnsureUsernameSizeCorrect(insertUserData.username);
    EnsureNameSizeCorrect(insertUserData.name);

    auto hashAlg = GetDefaultHashAlg();
    auto hash = ComputePasswordHash(password, hashAlg);

    QSqlQuery query;
    query.prepare("INSERT INTO openings_user "
                  "(username, name, password_hash, hash_alg) "
                  "VALUES (:username, :name, :password_hash, :hash_alg)");
    query.bindValue(":username", insertUserData.username);
    query.bindValue(":name", insertUserData.name);
    query.bindValue(":password_hash", hash);
    query.bindValue(":hash_alg", int(hashAlg));

    std::unique_ptr<UserData> data;
    if (query.exec()) {
      data = LoadByUsername(insertUserData.username);
    }
    return data;
  }

  QList<UserData> LoadUsers()
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  "id, " // 0
                  "username, " // 1
                  "name, " // 2
                  "registration_date " // 3
                  "FROM openings_user ");
    if( !query.exec() ) {
      throw std::runtime_error("Error while loading user data list");
    }

    QList<UserData> dataList;
    while (query.next()) {
      auto& data = dataList.emplace_back();

      data.id = UserID(query.value(0).toInt());
      data.username = query.value(1).toString();
      data.name = query.value(2).toString();
      data.registrationDate = query.value(3).toDateTime();
    }
    return dataList;
  }

  std::unique_ptr<UserData> LoadById(
    UserID id
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  "id, " // 0
                  "username, " // 1
                  "name, " // 2
                  "registration_date " // 3
                  "FROM openings_user "
                  "WHERE id = ?");
    query.addBindValue(int(id));
    if( !query.exec() ) {
      throw std::runtime_error("Error while loading user data by id");
    }

    std::unique_ptr<UserData> data;

    if (query.next()) {
        data = std::make_unique<UserData>();

      data->id = UserID(query.value(0).toInt());
      data->username = query.value(1).toString();
      data->name = query.value(2).toString();
      data->registrationDate = query.value(3).toDateTime();
    }

    return data;
  }

  std::unique_ptr<UserData> LoadByUsername(
    QString username
  )
  {
    EnsureUsernameSizeCorrect(username);

    QSqlQuery query;
    query.prepare("SELECT "
                  "id, " // 0
                  "username, " // 1
                  "name, " // 2
                  "registration_date " // 3
                  "FROM openings_user "
                  "WHERE username = :username");
    query.bindValue(":username", username);
    if( !query.exec() ) {
      throw std::runtime_error("Error while loading user data by username");
    }

    std::unique_ptr<UserData> data;

    if (query.next()) {
      data = std::make_unique<UserData>();

      data->id = UserID(query.value(0).toInt());
      data->username = query.value(1).toString();
      data->name = query.value(2).toString();
      data->registrationDate = query.value(3).toDateTime();
    }

    return data;
  }

  void UpdateUserData(
    const UserData& userData,
    QString password
  )
  {
    EnsureCorrectPassword(userData.id, password);

    if (auto loaded = LoadById(userData.id);
        !loaded) {
      throw std::runtime_error("User not found");
    }
    else if (loaded->registrationDate != userData.registrationDate) {
      throw std::runtime_error("Cannot change registration date");
    }

    QSqlQuery query;
    query.prepare("UPDATE openings_user "
                  "SET "
                  "username = :username, "
                  "name = :name "
                  "WHERE id = :id");
    query.bindValue(":username", userData.username);
    query.bindValue(":name", userData.name);
    query.bindValue(":id", int(userData.id));

    if (!query.exec()) {
      throw std::runtime_error("Error while updating user data");
    }
  }

  bool VerifyPassword(
    UserID userId,
    QString password
  )
  {
    QSqlQuery query;
    query.prepare("SELECT "
                  "password_hash, "
                  "hash_alg "
                  "FROM openings_user "
                  "WHERE id = ?");
    query.addBindValue(int(userId));

    std::unique_ptr<UserData> data;

    if (!query.exec() || !query.next()) {
      throw std::runtime_error("User not found");
    }

    auto storedHash = query.value(0).toByteArray();
    auto hashAlg = QCryptographicHash::Algorithm(query.value(1).toInt());
    auto hash = ComputePasswordHash(password, hashAlg);

    return hash == storedHash;
  }

  void UpdatePassword(
    UserID userId,
    QString oldPassword,
    QString newPassword
  )
  {
    EnsureCorrectPassword(userId, oldPassword);

    auto hashAlg = GetDefaultHashAlg();
    auto hash = ComputePasswordHash(newPassword, hashAlg);

    QSqlQuery query;
    query.prepare("UPDATE openings_user "
                  "SET "
                  "password_hash = :hash, "
                  "hash_alg = :hash_alg "
                  "WHERE id = :id");
    query.bindValue(":hash", hash);
    query.bindValue(":hash_alg", hashAlg);
    query.bindValue(":id", int(userId));

    if (!query.exec()) {
      throw std::runtime_error("Error while updating password");
    }
  }

  void DeleteUser(
    UserID userId,
    QString password
  )
  {
    EnsureCorrectPassword(userId, password);

    QSqlQuery query;
    query.prepare("DELETE "
                  "FROM openings_user "
                  "WHERE id = ?");
    query.addBindValue(int(userId));
    if (!query.exec()) {
      throw std::runtime_error("Error while deleting a user");
    }
  }
}


