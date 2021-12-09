#ifndef COMMON_H
#define COMMON_H

#include <functional>

enum class CreateCompanyRequestStatus
{
  Posted = 1,
  Cancelled = 2,
  Denied = 3,
  Accepted = 4,
};

constexpr ssize_t USER_USERNAME_SIZE = 30;
constexpr ssize_t USER_NAME_SIZE = 255;

class CompanyID final
{
  int id = -1;
public:
  CompanyID() = default;
  explicit CompanyID(int id);
  operator int() const;
};

class UserID final
{
  int id = -1;
public:
  UserID() = default;
  explicit UserID(int id);
  operator int() const;
};

class CreateCompanyRequestID final
{
  int id = -1;
public:
  CreateCompanyRequestID() = default;
  explicit CreateCompanyRequestID(int id);
  operator int() const;
};

class JobOpeningID final
{
  int id = -1;
public:
  JobOpeningID() = default;
  explicit JobOpeningID(int id);
  operator int() const;
};

class UserResumeID final
{
  int id = -1;
public:
  UserResumeID() = default;
  explicit UserResumeID(int id);
  operator int() const;
};

class ApplicationID final
{
  int id = -1;
public:
  ApplicationID() = default;
  explicit ApplicationID(int id);
  operator int() const;
};

namespace std {
  template <> struct hash<CompanyID> {
    size_t operator()(const CompanyID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };

  template <> struct hash<UserID> {
    size_t operator()(const UserID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };

  template <> struct hash<CreateCompanyRequestID> {
    size_t operator()(const CreateCompanyRequestID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };

  template <> struct hash<JobOpeningID> {
    size_t operator()(const JobOpeningID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };

  template <> struct hash<UserResumeID> {
    size_t operator()(const UserResumeID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };

  template <> struct hash<ApplicationID> {
    size_t operator()(const ApplicationID& x) const {return intHash(x);}
  private:
    hash<int> intHash;
  };
}

#endif // COMMON_H
