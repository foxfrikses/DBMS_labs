#include "Common.h"

CompanyID::CompanyID(int id)
  : id(id)
{}

CompanyID::operator int() const
{
  return id;
}

UserID::UserID(int id)
  : id(id)
{}

UserID::operator int() const
{
  return id;
}

CreateCompanyRequestID::CreateCompanyRequestID(int id)
  : id(id)
{}

CreateCompanyRequestID::operator int() const
{
  return id;
}

JobOpeningID::JobOpeningID(int id)
  : id(id)
{}

JobOpeningID::operator int() const
{
  return id;
}

ApplicationID::ApplicationID(int id)
  : id(id)
{}

ApplicationID::operator int() const
{
  return id;
}

UserResumeID::UserResumeID(int id)
  : id(id)
{}

UserResumeID::operator int() const
{
  return id;
}

