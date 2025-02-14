#include "settingspopup.h"
#include "../settingstrings.h"
#include "colorsettingedit.h"
#include "fontsettingedit.h"
#include "iconsettingedit.h"
#include "integersettingedit.h"
#include "textsettingedit.h"
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

SettingsPopup::SettingsPopup() {
  populateWidgets();
  this->setWindowTitle("Room Booker Settings");
  QIcon windowIcon(":/resources/defaulticon.png");
  this->setWindowIcon(windowIcon);
}

void SettingsPopup::populateWidgets() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  QScrollArea *scrollArea = new QScrollArea(this);
  QWidget *scrollWidget = new QWidget(this);
  QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setAlignment(Qt::AlignTop);

  TextSettingEdit *apiEdit =
      new TextSettingEdit("API Address", API_ADDRESS_SETTING);
  layout->addWidget(apiEdit);

  IntegerSettingEdit *roomIdEdit =
      new IntegerSettingEdit("Room ID", ROOM_ID_SETTING, 100000);
  layout->addWidget(roomIdEdit);

  TextSettingEdit *unbookedNameEdit =
      new TextSettingEdit("Unbooked large text", UNBOOKED_NAME_TEXT_SETTING);
  layout->addWidget(unbookedNameEdit);

  TextSettingEdit *unbookedInfoEdit =
      new TextSettingEdit("Unbooked info text", UNBOOKED_INFO_TEXT_SETTING);
  layout->addWidget(unbookedInfoEdit);

  TextSettingEdit *unbookedStatusEdit =
      new TextSettingEdit("Unbooked status text", UNBOOKED_STATUS_TEXT_SETTING);
  layout->addWidget(unbookedStatusEdit);

  TextSettingEdit *bookedUsernamePrefixEdit = new TextSettingEdit(
      "Booked username prefix text", BOOKED_USERNAME_PREFIX_TEXT_SETTING);
  layout->addWidget(bookedUsernamePrefixEdit);

  IntegerSettingEdit *bookingNameMaxCharactersEdit = new IntegerSettingEdit(
      "Max booking name characters", BOOKING_NAME_MAX_CHARACTERS, 2000);
  layout->addWidget(bookingNameMaxCharactersEdit);
  IntegerSettingEdit *bookingUsernameMaxCharactersEdit = new IntegerSettingEdit(
      "Max booking username characters", BOOKING_USERNAME_MAX_CHARACTERS, 2000);
  layout->addWidget(bookingUsernameMaxCharactersEdit);
  IntegerSettingEdit *upcomingBookingMaxCharacters =
      new IntegerSettingEdit("Max upcoming booking name characters",
                             UPCOMING_BOOKING_NAME_MAX_CHARACTERS, 2000);
  layout->addWidget(upcomingBookingMaxCharacters);

  QWidget *colorSettingWidget = new QWidget();
  QHBoxLayout *colorSettingLayout = new QHBoxLayout();
  colorSettingLayout->setContentsMargins(0, 0, 0, 0);
  colorSettingWidget->setLayout(colorSettingLayout);
  layout->addWidget(colorSettingWidget);

  ColorSettingEdit *backgroundColorEdit =
      new ColorSettingEdit("Background", BACKGROUND_COLOR_SETTING);
  colorSettingLayout->addWidget(backgroundColorEdit);
  ColorSettingEdit *unbookedColorEdit =
      new ColorSettingEdit("Unbooked", UNBOOKED_COLOR_SETTING);
  colorSettingLayout->addWidget(unbookedColorEdit);
  ColorSettingEdit *bookedColorEdit =
      new ColorSettingEdit("Booked", BOOKED_COLOR_SETTING);
  colorSettingLayout->addWidget(bookedColorEdit);

  FontSettingEdit *nameFontEdit =
      new FontSettingEdit("Name font", BOOKNG_NAME_FONT_SETTING);
  layout->addWidget(nameFontEdit);
  FontSettingEdit *infoFontEdit =
      new FontSettingEdit("Info font", BOOKING_INFO_FONT_SETTING);
  layout->addWidget(infoFontEdit);
  FontSettingEdit *statusFontEdit =
      new FontSettingEdit("Status font", BOOKING_STATUS_FONT_SETTING);
  layout->addWidget(statusFontEdit);
  FontSettingEdit *upcomingBoldFontEdit = new FontSettingEdit(
      "Upcoming time font", UPCOMING_BOOKINGS_BOLD_FONT_SETTING);
  layout->addWidget(upcomingBoldFontEdit);
  FontSettingEdit *upcomingLightFontEdit = new FontSettingEdit(
      "Upcoming name font", UPCOMING_BOOKINGS_LIGHT_FONT_SETTING);
  layout->addWidget(upcomingLightFontEdit);

  IconSettingEdit *iconEdit =
      new IconSettingEdit("Select icon", ICON_FILE_PATH_SETTING);
  layout->addWidget(iconEdit);

  scrollLayout->addLayout(layout);
  scrollWidget->setLayout(scrollLayout);
  mainLayout->addWidget(scrollArea);

  QLabel *restartText = new QLabel("Settings stored automatically. Restart "
                                   "required for changes to take effect.");
  restartText->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(restartText);
  QLabel *creditsText =
      new QLabel("Room Booker v[VERSION_NUMBER] by Mervin van Brakel :)");
  creditsText->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(creditsText);

  mainLayout->addWidget(getBottomButtons());

  scrollArea->setWidget(scrollWidget);
  scrollArea->setWidgetResizable(true);

  setLayout(mainLayout);
}

QWidget *SettingsPopup::getBottomButtons() {
  QWidget *bottomButtons = new QWidget();
  QHBoxLayout *layout = new QHBoxLayout();
  bottomButtons->setLayout(layout);

  QPushButton *restartProgram = new QPushButton("Restart program");
  layout->addWidget(restartProgram);
  connect(restartProgram, &QPushButton::clicked, this,
          &SettingsPopup::restartProgram);

  QPushButton *closeProgram = new QPushButton("Close program");
  layout->addWidget(closeProgram);
  connect(closeProgram, &QPushButton::clicked, qApp, &QApplication::quit);

  layout->addStretch();

  QPushButton *exitSettings = new QPushButton("Exit settings");
  layout->addWidget(exitSettings);
  connect(exitSettings, &QPushButton::clicked, this, &SettingsPopup::close);

  return bottomButtons;
}

void SettingsPopup::restartProgram() {
  qApp->quit();
  QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
