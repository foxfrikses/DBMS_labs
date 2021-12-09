DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
CONFIG += c++20
QT += core gui widgets quick sql
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15 # to support filesystem path

SOURCES += \
    Source/MainWidgets/ApplicationDialog.cpp \
    Source/MainWidgets/ApplicationsDialog.cpp \
    Source/MainWidgets/JobOpeningDialog.cpp \
    Source/MainWidgets/OpeningsDialog.cpp \
    Source/Models/ApplicationModel.cpp \
    Source/Models/CompanyModel.cpp \
    Source/Models/CompanyPermissionModel.cpp \
    Source/Models/JobOpeningModel.cpp \
    Source/Models/UserPermissionModel.cpp \
    Source/Models/UserResumeModel.cpp \
    main.cpp \
    \
    Source/Common.cpp \
    Source/MainWindow.cpp \
    Source/AuthenticatedUser.cpp \
    \
    Source/MainWidgets/EditUserInfoWidget.cpp \
    Source/MainWidgets/CreateCompanyWidget.cpp \
    Source/MainWidgets/CompanyListWidget.cpp \
    Source/MainWidgets/CreateCompanyRequestsWidget.cpp \
    Source/MainWidgets/MyCreateCompanyRequestsWidget.cpp \
    Source/MainWidgets/UserListWidget.cpp \
    \
    Source/Models/UserModel.cpp \
    Source/Models/AdminModel.cpp \
    \
    Source/Authentication/LoginDialog.cpp \
    Source/Authentication/LogoutDialog.cpp \
    Source/Authentication/RegisterDialog.cpp

HEADERS += \
    Headers/Common.h \
    Headers/MainWidgets/ApplicationDialog.h \
    Headers/MainWidgets/ApplicationsDialog.h \
    Headers/MainWidgets/JobOpeningDialog.h \
    Headers/MainWidgets/OpeningsDialog.h \
    Headers/Models/ApplicationModel.h \
    Headers/Models/CompanyModel.h \
    Headers/Models/CompanyPermissionModel.h \
    Headers/Models/JobOpeningModel.h \
    Headers/Models/UserPermissionModel.h \
    Headers/Models/UserResumeModel.h \
    \
    Headers/MainWidgets/EditUserInfoWidget.h \
    Headers/MainWidgets/CreateCompanyWidget.h \
    Headers/MainWidgets/CompanyListWidget.h \
    Headers/MainWidgets/CreateCompanyRequestsWidget.h \
    Headers/MainWidgets/MyCreateCompanyRequestsWidget.h \
    Headers/MainWidgets/UserListWidget.h \
    \
    Headers/MainWindow.h \
    Headers/AuthenticatedUser.h \
    \
    Headers/Models/UserModel.h \
    Headers/Models/AdminModel.h \
    \
    Headers/Authentication/LogoutDialog.h \
    Headers/Authentication/LoginDialog.h \
    Headers/Authentication/RegisterDialog.h

FORMS += \
    Forms/MainWidgets/ApplicationDialog.ui \
    Forms/MainWidgets/ApplicationsDialog.ui \
    Forms/MainWidgets/EditUserInfoWidget.ui \
    Forms/MainWidgets/CreateCompanyWidget.ui \
    Forms/MainWidgets/CreateCompanyRequestsWidget.ui \
    Forms/MainWidgets/JobOpeningDialog.ui \
    Forms/MainWidgets/MyCreateCompanyRequestsWidget.ui \
    Forms/MainWidgets/CompanyListWidget.ui \
    Forms/MainWidgets/OpeningsDialog.ui \
    Forms/MainWidgets/UserListWidget.ui \
    \
    Forms/MainWindow.ui \
    \
    Forms/Authentication/LoginDialog.ui \
    Forms/Authentication/LogoutDialog.ui \
    Forms/Authentication/RegisterDialog.ui

INCLUDEPATH += \
    Headers \
    Headers/Authentication \
    Headers/MainWidgets \
    Headers/Models

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
