#include "bookingdisplay.h"
#include "./settings/settingspopup.h"
#include "./widgets/clickableicon.h"
#include "settingstrings.h"
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <qevent.h>

BookingDisplay::BookingDisplay(int &argc, char **argv)
    : QApplication(argc, argv) {
  configureStoredColors();
  addFontsToDatabase();
  populateSettingsIfNeeded();

  m_bookingWidget = new BookingWidget();
  m_bookingName = new BookingName();
  m_bookingInfo = new BookingInfo();
  m_bookingStatus = new BookingStatus();
  m_dataFetchingHandler = new DataFetchingHandler();
  m_upcomingBookings = new UpcomingBookings();

  configureWidgetLayout();
  connectSignals();
  m_bookingWidget->show();

  m_dataFetchingHandler->getBookingData();
  this->exec();
}

void BookingDisplay::addFontsToDatabase() {
  QFontDatabase::addApplicationFont(":/resources/ZillaSlabLight.ttf");
  QFontDatabase::addApplicationFont(":/resources/ZillaSlabSemiBold.ttf");
}

void BookingDisplay::configureStoredColors() {
  QSettings settings;
  m_unbookedColor = settings.value(UNBOOKED_COLOR_SETTING).value<QColor>();
  m_bookedColor = settings.value(BOOKED_COLOR_SETTING).value<QColor>();
};

void BookingDisplay::populateSettingsIfNeeded() {
  QSettings settings;
  if (settings.contains(API_ADDRESS_SETTING)) {
    return;
  }

  settings.setValue(API_ADDRESS_SETTING, "http://127.0.0.1:37222");
  settings.setValue(ROOM_ID_SETTING, 1);

  settings.setValue(UNBOOKED_NAME_TEXT_SETTING, "Configure me please!");
  settings.setValue(UNBOOKED_INFO_TEXT_SETTING, "Bookable through Slack");
  settings.setValue(UNBOOKED_STATUS_TEXT_SETTING, "Free");
  settings.setValue(BOOKED_USERNAME_PREFIX_TEXT_SETTING, "Booked by ");

  settings.setValue(BOOKING_NAME_MAX_CHARACTERS, 43);
  settings.setValue(BOOKING_USERNAME_MAX_CHARACTERS, 35);
  settings.setValue(UPCOMING_BOOKING_NAME_MAX_CHARACTERS, 17);

  settings.setValue(BACKGROUND_COLOR_SETTING, QColor("#202030"));
  settings.setValue(UNBOOKED_COLOR_SETTING, QColor("#82D173"));
  settings.setValue(BOOKED_COLOR_SETTING, QColor("#2F4858"));

  settings.setValue(BOOKNG_NAME_FONT_SETTING, QFont("Zilla Slab SemiBold", 72));
  settings.setValue(BOOKING_INFO_FONT_SETTING, QFont("Zilla Slab Light", 30));
  settings.setValue(BOOKING_STATUS_FONT_SETTING, QFont("Zilla Slab Light", 50));
  settings.setValue(UPCOMING_BOOKINGS_BOLD_FONT_SETTING,
                    QFont("Zilla Slab SemiBold", 25));
  settings.setValue(UPCOMING_BOOKINGS_LIGHT_FONT_SETTING,
                    QFont("Zilla Slab Light", 25));

  settings.setValue(ICON_FILE_PATH_SETTING, ":/resources/defaulticon.png");

  settings.sync();
}

void BookingDisplay::configureWidgetLayout() {
  QVBoxLayout *bookingWidgetVerticalLayout = new QVBoxLayout();

  bookingWidgetVerticalLayout->setAlignment(Qt::AlignTop);
  bookingWidgetVerticalLayout->setContentsMargins(15, 0, 0, 0);
  m_bookingWidget->setLayout(bookingWidgetVerticalLayout);

  QWidget *topSectionHorizontalDividerWidget = new QWidget();
  QHBoxLayout *topSectionHorizontalDividerLayout =
      new QHBoxLayout(topSectionHorizontalDividerWidget);
  topSectionHorizontalDividerLayout->setContentsMargins(0, 0, 0, 0);
  bookingWidgetVerticalLayout->addWidget(topSectionHorizontalDividerWidget, 4);

  QWidget *topSectionLeftSideWidget = new QWidget();
  QVBoxLayout *topSectionLeftSideLayout =
      new QVBoxLayout(topSectionLeftSideWidget);
  topSectionLeftSideLayout->addWidget(m_bookingName);
  topSectionLeftSideLayout->addStretch();
  topSectionLeftSideLayout->addWidget(m_bookingInfo);
  topSectionHorizontalDividerLayout->addWidget(topSectionLeftSideWidget, 3);

  topSectionHorizontalDividerLayout->addWidget(m_upcomingBookings, 1);

  QWidget *bottomSectionWidget = new QWidget();
  QHBoxLayout *bottomSectionHorizontalLayout =
      new QHBoxLayout(bottomSectionWidget);
  bottomSectionHorizontalLayout->addWidget(m_bookingStatus);
  bookingWidgetVerticalLayout->addWidget(bottomSectionWidget, 1);
  bottomSectionHorizontalLayout->addStretch();
  bottomSectionHorizontalLayout->setContentsMargins(9, 0, 20, 0);

  ClickableIcon *clickableIcon = new ClickableIcon();
  bottomSectionHorizontalLayout->addWidget(clickableIcon);

  connect(clickableIcon, &ClickableIcon::clicked, this,
          &BookingDisplay::openSettingsWindow);
}

void BookingDisplay::connectSignals() {
  // Updating the current booking after a slight delay so the upcoming bookings
  // are animated first. This looks cooler I think.
  connect(m_dataFetchingHandler, &DataFetchingHandler::currentBookingChanged,
          this, [this](CurrentBookingData newCurrentBooking) {
            QTimer::singleShot(1600, this, [this, newCurrentBooking]() {
              updateCurrentBooking(newCurrentBooking);
            });
          });

  connect(m_dataFetchingHandler, &DataFetchingHandler::currentBookingChanged,
          m_upcomingBookings, &UpcomingBookings::updateCurrentBooking);
  connect(m_dataFetchingHandler, &DataFetchingHandler::upcomingBookingsChanged,
          m_upcomingBookings, &UpcomingBookings::updateUpcomingBookings);
}

void BookingDisplay::updateCurrentBooking(
    CurrentBookingData newCurrentBooking) {
  if (newCurrentBooking.state == CurrentBookingState::ERROR) {
    m_bookingWidget->changeStatusColor(QColor(0, 0, 0));
  } else if (newCurrentBooking.state == CurrentBookingState::UNBOOKED) {
    m_bookingWidget->changeStatusColor(m_unbookedColor);
  } else if (newCurrentBooking.state == CurrentBookingState::BOOKED) {
    m_bookingWidget->changeStatusColor(m_bookedColor);
  }

  m_bookingName->changeBookingName(newCurrentBooking.name);
  m_bookingStatus->changeBookingStatus(newCurrentBooking.status);
  m_bookingInfo->changeBookingInfo(newCurrentBooking.info);
}

void BookingDisplay::openSettingsWindow() {
  SettingsPopup settingsPopup;
  settingsPopup.exec();
}

bool BookingDisplay::notify(QObject *receiver, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      QApplication::quit();
      return true;
    }
    if (keyEvent->key() == Qt::Key_S) {
      openSettingsWindow();
      return true;
    }
  }
  return QApplication::notify(receiver, event);
}
