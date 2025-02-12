#include "upcomingbookings.h"
#include "../settingstrings.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

UpcomingBooking::UpcomingBooking(QWidget *parent)
    : QWidget{parent}, m_bookingTimeLabel(new QLabel("", this)),
      m_bookingNameLabel(new QLabel("", this)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  QSettings settings;
  m_maxCharacters =
      settings.value(UPCOMING_BOOKING_NAME_MAX_CHARACTERS).toInt();

  layout->addWidget(m_bookingTimeLabel);
  layout->addWidget(m_bookingNameLabel);
  this->setLayout(layout);
  configureLabelStyling();
}

void UpcomingBooking::configureLabelStyling() {
  QSettings settings;
  m_bookingTimeLabel->setFont(
      settings.value(UPCOMING_BOOKINGS_BOLD_FONT_SETTING).value<QFont>());
  m_bookingTimeLabel->setStyleSheet("color: black;");

  m_bookingNameLabel->setFont(
      settings.value(UPCOMING_BOOKINGS_LIGHT_FONT_SETTING).value<QFont>());
  m_bookingNameLabel->setStyleSheet("black;");
  m_bookingNameLabel->setWordWrap(true);
  m_bookingNameLabel->ensurePolished();

  QFontMetrics metrics(m_bookingNameLabel->font());
  int lineHeight = metrics.lineSpacing();
  m_bookingNameLabel->setMaximumHeight(lineHeight * 2);
}

void UpcomingBooking::updateBookingText(
    UpcomingBookingData upcomingBookingDataToDisplay) {
  if (upcomingBookingDataToDisplay.name.size() > m_maxCharacters) {
    upcomingBookingDataToDisplay.name.resize(m_maxCharacters);
    upcomingBookingDataToDisplay.name.append("...");
  }
  m_bookingTimeLabel->setText(upcomingBookingDataToDisplay.timeString);
  m_bookingNameLabel->setText(upcomingBookingDataToDisplay.name);
}

UpcomingBookings::UpcomingBookings(QWidget *parent)
    : QWidget{parent}, m_firstUpcomingBooking(new UpcomingBooking()),
      m_storedUpcomingBookingData(),
      m_secondUpcomingBooking(new UpcomingBooking()),
      m_thirdUpcomingBooking(new UpcomingBooking()),
      firstUpcomingBookingStartingPosition(new QPoint()),
      m_fadeOutAllAnimationGroup(new QParallelAnimationGroup(this)),
      m_fadeInAllAnimationGroup(new QParallelAnimationGroup(this)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  this->setLayout(layout);

  layout->addWidget(m_firstUpcomingBooking);
  layout->addWidget(m_secondUpcomingBooking);
  layout->addWidget(m_thirdUpcomingBooking);
  layout->addStretch();
  configureOpacityEffects();
  configureFadeInAnimations();
  configureFirstBookingPositionAnimation();
  configureFadeOutAnimations();
}

void UpcomingBookings::updateCurrentBooking(CurrentBookingData currentBooking) {
  m_currentBookingId = currentBooking.id;
}

void UpcomingBookings::updateUpcomingBookings(
    QList<UpcomingBookingData> upcomingBookingData) {

  if (!m_storedUpcomingBookingData.isEmpty() &&
      m_storedUpcomingBookingData[0].id == m_currentBookingId) {
    QTimer::singleShot(1100, m_firstBookingPositionAnimation, SLOT(start()));
  }

  m_storedUpcomingBookingData = upcomingBookingData;
  m_fadeOutAllAnimationGroup->start();
}

void UpcomingBookings::onFadeOutAnimationFinished() {
  m_firstUpcomingBooking->updateBookingText(m_storedUpcomingBookingData[0]);
  m_secondUpcomingBooking->updateBookingText(m_storedUpcomingBookingData[1]);
  m_thirdUpcomingBooking->updateBookingText(m_storedUpcomingBookingData[2]);
  m_fadeInAllAnimationGroup->start();
}

void UpcomingBookings::onPositionFadeAnimationFinished() {
  m_firstUpcomingBooking->move(*firstUpcomingBookingStartingPosition +
                               QPoint(9, 9));
}

void UpcomingBookings::configureOpacityEffects() {
  m_firstOpacityEffect = new QGraphicsOpacityEffect(m_firstUpcomingBooking);
  m_firstOpacityEffect->setOpacity(100);
  m_firstUpcomingBooking->setGraphicsEffect(m_firstOpacityEffect);

  m_secondOpacityEffect = new QGraphicsOpacityEffect(m_secondUpcomingBooking);
  m_secondOpacityEffect->setOpacity(60);
  m_secondUpcomingBooking->setGraphicsEffect(m_secondOpacityEffect);

  m_thirdOpacityEffect = new QGraphicsOpacityEffect(m_thirdUpcomingBooking);
  m_thirdOpacityEffect->setOpacity(40);
  m_thirdUpcomingBooking->setGraphicsEffect(m_thirdOpacityEffect);
}

void UpcomingBookings::configureFadeInAnimations() {
  configureSingleFadeInAnimation(m_firstOpacityEffect, 600, 0.375, 1.0);
  configureSingleFadeInAnimation(m_secondOpacityEffect, 300, 0.23, 0.6);
  configureSingleFadeInAnimation(m_thirdOpacityEffect, 0, 0.0, 0.4);
}

void UpcomingBookings::configureSingleFadeInAnimation(
    QGraphicsOpacityEffect *opacityEffect, int timingDelay, float offsetStep,
    float finalOpacity) {
  QPropertyAnimation *fadeInOpacityAnimation =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeInOpacityAnimation->setDuration(1000 + timingDelay);
  fadeInOpacityAnimation->setKeyValueAt(offsetStep, 0.0);
  fadeInOpacityAnimation->setStartValue(0.0);
  fadeInOpacityAnimation->setEndValue(finalOpacity);
  m_fadeInAllAnimationGroup->addAnimation(fadeInOpacityAnimation);
}

void UpcomingBookings::configureFadeOutAnimations() {
  configureSingleFadeOutAnimation(m_firstOpacityEffect, 1000, 0.5, 1.0);
  configureSingleFadeOutAnimation(m_secondOpacityEffect, 500, 0.33, 0.6);
  configureSingleFadeOutAnimation(m_thirdOpacityEffect, 0, 0.0, 0.4);

  connect(m_fadeOutAllAnimationGroup, &QParallelAnimationGroup::finished, this,
          &UpcomingBookings::onFadeOutAnimationFinished);
}

void UpcomingBookings::configureSingleFadeOutAnimation(
    QGraphicsOpacityEffect *opacityEffect, int timingDelay, float offsetStep,
    float beginningOpacity) {
  QPropertyAnimation *fadeInOpacityAnimation =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeInOpacityAnimation->setDuration(1000 + timingDelay);
  fadeInOpacityAnimation->setKeyValueAt(offsetStep, beginningOpacity);
  fadeInOpacityAnimation->setStartValue(beginningOpacity);
  fadeInOpacityAnimation->setEndValue(0.0);

  m_fadeOutAllAnimationGroup->addAnimation(fadeInOpacityAnimation);
}

void UpcomingBookings::configureFirstBookingPositionAnimation() {
  *firstUpcomingBookingStartingPosition = m_firstUpcomingBooking->pos();

  m_firstBookingPositionAnimation =
      new QPropertyAnimation(m_firstUpcomingBooking, "pos");
  m_firstBookingPositionAnimation->setDuration(1200);
  m_firstBookingPositionAnimation->setStartValue(
      *firstUpcomingBookingStartingPosition + QPoint(9, 9));
  m_firstBookingPositionAnimation->setEndValue(
      *firstUpcomingBookingStartingPosition + QPoint(9, -150));
  m_firstBookingPositionAnimation->setEasingCurve(QEasingCurve::InCubic);

  connect(m_firstBookingPositionAnimation, &QPropertyAnimation::finished, this,
          &UpcomingBookings::onPositionFadeAnimationFinished);
}
