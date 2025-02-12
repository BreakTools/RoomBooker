#include "bookingname.h"
#include "../settingstrings.h"
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QLayout>
#include <QPropertyAnimation>
#include <QSettings>

BookingName::BookingName(QWidget *parent)
    : QWidget{parent}, m_fadeInAnimationGroup(new QParallelAnimationGroup()),
      m_fadeOutAnimationGroup(new QParallelAnimationGroup()) {
  QSettings settings;
  m_maxCharacters = settings.value(BOOKING_NAME_MAX_CHARACTERS).toInt();
  m_bookingNameText =
      new QString(settings.value(UNBOOKED_NAME_TEXT_SETTING).toString());
  m_bookingNameLabel = new QLabel(*m_bookingNameText, this);

  QVBoxLayout *layout = new QVBoxLayout(this);

  layout->addWidget(m_bookingNameLabel);
  configureLabelStyling();
  configureAnimations();
}

void BookingName::configureLabelStyling() {
  QSettings settings;
  m_bookingNameLabel->setFont(
      settings.value(BOOKNG_NAME_FONT_SETTING).value<QFont>());
  m_bookingNameLabel->setStyleSheet("color: white;");
  m_bookingNameLabel->setWordWrap(true);
  m_bookingNameLabel->ensurePolished();

  QFontMetrics metrics(m_bookingNameLabel->font());
  int lineHeight = metrics.lineSpacing();
  m_bookingNameLabel->setMaximumHeight(lineHeight * 3);
}

void BookingName::configureAnimations() {
  QPoint currentPoint = m_bookingNameLabel->pos();

  QGraphicsOpacityEffect *opacityEffect =
      new QGraphicsOpacityEffect(m_bookingNameLabel);
  opacityEffect->setOpacity(100);
  m_bookingNameLabel->setGraphicsEffect(opacityEffect);

  QPropertyAnimation *fadeInOpacityAnimation =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeInOpacityAnimation->setDuration(1000);
  fadeInOpacityAnimation->setStartValue(0.0);
  fadeInOpacityAnimation->setEndValue(1.0);
  m_fadeInAnimationGroup->addAnimation(fadeInOpacityAnimation);

  QPropertyAnimation *fadeInPositionAnimation =
      new QPropertyAnimation(m_bookingNameLabel, "pos");
  fadeInPositionAnimation->setDuration(800);
  // When animating to a new value the margins don't seem to be used anymore...
  fadeInPositionAnimation->setStartValue(currentPoint + QPoint(9, -30));
  fadeInPositionAnimation->setEndValue(currentPoint + QPoint(9, 9));
  fadeInPositionAnimation->setEasingCurve(QEasingCurve::OutQuad);
  m_fadeInAnimationGroup->addAnimation(fadeInPositionAnimation);

  QPropertyAnimation *fadeOutOpacityAnimation =
      new QPropertyAnimation(opacityEffect, "opacity");
  fadeOutOpacityAnimation->setDuration(500);
  fadeOutOpacityAnimation->setStartValue(1.0);
  fadeOutOpacityAnimation->setEndValue(0.0);
  fadeOutOpacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
  m_fadeOutAnimationGroup->addAnimation(fadeOutOpacityAnimation);

  QPropertyAnimation *fadeOutPositionAnimation =
      new QPropertyAnimation(m_bookingNameLabel, "pos");
  fadeOutPositionAnimation->setDuration(500);
  fadeOutPositionAnimation->setStartValue(currentPoint + QPoint(9, 9));
  fadeOutPositionAnimation->setEndValue(currentPoint + QPoint(9, 30));
  fadeOutPositionAnimation->setEasingCurve(QEasingCurve::InOutQuad);

  m_fadeOutAnimationGroup->addAnimation(fadeOutPositionAnimation);

  connect(m_fadeOutAnimationGroup, &QParallelAnimationGroup::finished, this,
          &BookingName::fadeInNewName);
}

void BookingName::changeBookingName(QString newBookingNameText) {
  if (newBookingNameText.size() > m_maxCharacters) {
    newBookingNameText.resize(m_maxCharacters);
    newBookingNameText.append("...");
  }
  *m_bookingNameText = newBookingNameText;
  fadeOutOldName();
}

void BookingName::fadeOutOldName() { m_fadeOutAnimationGroup->start(); }

void BookingName::fadeInNewName() {
  m_bookingNameLabel->setText(*m_bookingNameText);
  m_fadeInAnimationGroup->start();
}
