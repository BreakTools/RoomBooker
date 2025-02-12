#include "bookingstatus.h"
#include "../settingstrings.h"
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QSettings>

BookingStatus::BookingStatus(QWidget *parent) : QWidget{parent} {
  QSettings settings;
  m_bookingStatusText =
      new QString(settings.value(UNBOOKED_STATUS_TEXT_SETTING).toString());
  m_bookingStatusLabel = new QLabel(*m_bookingStatusText, this);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(m_bookingStatusLabel);
  configureLabelStyling();
  configureAnimations();
}

void BookingStatus::configureLabelStyling() {
  QSettings settings;
  m_bookingStatusLabel->setFont(
      settings.value(BOOKING_STATUS_FONT_SETTING).value<QFont>());
  m_bookingStatusLabel->setStyleSheet("color: white;");
  m_bookingStatusLabel->setWordWrap(true);
}

void BookingStatus::configureAnimations() {
  QGraphicsOpacityEffect *opacityEffect =
      new QGraphicsOpacityEffect(m_bookingStatusLabel);
  opacityEffect->setOpacity(100);
  m_bookingStatusLabel->setGraphicsEffect(opacityEffect);

  m_fadeInPropertyAnimation = new QPropertyAnimation(opacityEffect, "opacity");
  m_fadeInPropertyAnimation->setDuration(800);
  m_fadeInPropertyAnimation->setStartValue(0.0);
  m_fadeInPropertyAnimation->setEndValue(1.0);

  m_fadeOutPropertyAnimation = new QPropertyAnimation(opacityEffect, "opacity");
  m_fadeOutPropertyAnimation->setDuration(800);
  m_fadeOutPropertyAnimation->setStartValue(1.0);
  m_fadeOutPropertyAnimation->setKeyValueAt(0.3, 0.0);
  m_fadeOutPropertyAnimation->setEndValue(0.0);
  m_fadeOutPropertyAnimation->setEasingCurve(QEasingCurve::InOutQuad);

  connect(m_fadeOutPropertyAnimation, &QPropertyAnimation::finished, this,
          &BookingStatus::fadeInNewStatus);
}

void BookingStatus::changeBookingStatus(QString newBookingStatusText) {
  *m_bookingStatusText = newBookingStatusText;
  fadeOutOldStatus();
}

void BookingStatus::fadeOutOldStatus() { m_fadeOutPropertyAnimation->start(); }

void BookingStatus::fadeInNewStatus() {
  m_bookingStatusLabel->setText(*m_bookingStatusText);
  m_fadeInPropertyAnimation->start();
}
