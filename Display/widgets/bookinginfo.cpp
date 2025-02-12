#include "bookinginfo.h"
#include "../settingstrings.h"
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include <QVBoxLayout>

BookingInfo::BookingInfo(QWidget *parent) : QWidget{parent} {
  QSettings settings;
  m_maxCharacters = settings.value(BOOKING_USERNAME_MAX_CHARACTERS).toInt();

  m_bookingInfoText =
      new QString(settings.value(UNBOOKED_INFO_TEXT_SETTING).toString());
  m_bookingInfoLabel = new QLabel(*m_bookingInfoText, this);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(m_bookingInfoLabel);
  configureLabelStyling();
  configureAnimations();
}

void BookingInfo::configureLabelStyling() {
  QSettings settings;
  m_bookingInfoLabel->setFont(
      settings.value(BOOKING_INFO_FONT_SETTING).value<QFont>());
  m_bookingInfoLabel->setStyleSheet("color: white;");
  m_bookingInfoLabel->setWordWrap(true);
}

void BookingInfo::configureAnimations() {
  QGraphicsOpacityEffect *opacityEffect =
      new QGraphicsOpacityEffect(m_bookingInfoLabel);
  opacityEffect->setOpacity(100);
  m_bookingInfoLabel->setGraphicsEffect(opacityEffect);

  m_fadeInPropertyAnimation = new QPropertyAnimation(opacityEffect, "opacity");
  m_fadeInPropertyAnimation->setDuration(1000);
  m_fadeInPropertyAnimation->setStartValue(0.0);
  m_fadeInPropertyAnimation->setEndValue(1.0);

  m_fadeOutPropertyAnimation = new QPropertyAnimation(opacityEffect, "opacity");
  m_fadeOutPropertyAnimation->setDuration(500);
  m_fadeOutPropertyAnimation->setStartValue(1.0);
  m_fadeOutPropertyAnimation->setEndValue(0.0);
  m_fadeOutPropertyAnimation->setEasingCurve(QEasingCurve::InOutQuad);

  connect(m_fadeOutPropertyAnimation, &QPropertyAnimation::finished, this,
          &BookingInfo::fadeInNewInfo);
}

void BookingInfo::changeBookingInfo(QString newBookingInfoText) {
  if (newBookingInfoText.size() > m_maxCharacters) {
    newBookingInfoText.resize(m_maxCharacters);
    newBookingInfoText.append("...");
  }
  *m_bookingInfoText = newBookingInfoText;
  fadeOutOldInfo();
}

void BookingInfo::fadeOutOldInfo() { m_fadeOutPropertyAnimation->start(); }

void BookingInfo::fadeInNewInfo() {

  m_bookingInfoLabel->setText(*m_bookingInfoText);
  m_fadeInPropertyAnimation->start();
}
