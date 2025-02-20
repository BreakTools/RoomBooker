#include "bookingwidget.h"
#include "../settingstrings.h"
#include <QSettings>

BookingWidget::BookingWidget(QWidget *parent)
    : QWidget{parent}, m_statusColorToAnimateTo(Qt::black),
      m_slidingColorAnimationProgress(0.0),
      m_slidingColorAnimation(
          new QPropertyAnimation(this, "slidingColorAnimationProgress")) {

  QSettings settings;
  m_backgroundColor = settings.value(BACKGROUND_COLOR_SETTING).value<QColor>();
  m_currentlyDisplayingStatusColor =
      settings.value(UNBOOKED_COLOR_SETTING).value<QColor>();

  this->setWindowTitle("BreakTools Room Booker");
  QIcon windowIcon(":/icon.png");
  this->setWindowIcon(windowIcon);
  configureAnimation();
  this->showFullScreen();
  QCursor cursor = Qt::BlankCursor;
  this->setCursor(cursor);
}

void BookingWidget::changeStatusColor(QColor newColor) {
  m_statusColorToAnimateTo = newColor;
  m_slidingColorAnimation->start();
}

void BookingWidget::configureAnimation() {
  m_slidingColorAnimation->setDuration(700);
  m_slidingColorAnimation->setStartValue(0.0);
  m_slidingColorAnimation->setEndValue(1.0);

  connect(m_slidingColorAnimation, &QPropertyAnimation::finished, this,
          &BookingWidget::onAnimationFinished);
}

void BookingWidget::onAnimationFinished() {
  m_currentlyDisplayingStatusColor = m_statusColorToAnimateTo;
  m_slidingColorAnimationProgress = 0.0;
}

float BookingWidget::slidingColorAnimationProgress() const {
  return m_slidingColorAnimationProgress;
}

void BookingWidget::setSlidingColorAnimationProgress(float progress) {
  m_slidingColorAnimationProgress = progress;
  update();
}

void BookingWidget::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  QRect backgroundRectangle = this->rect();
  drawBackgroundColor(painter, backgroundRectangle);
  drawAnimatedStatusColor(painter, backgroundRectangle);
  drawOverlayGradient(painter, backgroundRectangle);
}

void BookingWidget::drawBackgroundColor(QPainter &painter,
                                        QRect &backgroundRectangle) {
  QRect largeTopRectangle =
      backgroundRectangle.adjusted(0, 0, 0, -backgroundRectangle.height() / 5);
  painter.fillRect(backgroundRectangle, m_backgroundColor);
}

void BookingWidget::drawAnimatedStatusColor(QPainter &painter,
                                            QRect &backgroundRectangle) {
  QRect lowerRect = backgroundRectangle.adjusted(
      0, backgroundRectangle.height() - backgroundRectangle.height() / 5, 0, 0);
  int totalWidth = lowerRect.width();
  int slidingWidth =
      static_cast<int>(totalWidth * m_slidingColorAnimationProgress);

  QRect currentColorRect = lowerRect.adjusted(0, 0, -slidingWidth, 0);
  painter.fillRect(currentColorRect, m_currentlyDisplayingStatusColor);

  QRect nextColorRect = lowerRect.adjusted(totalWidth - slidingWidth, 0, 0, 0);
  painter.fillRect(nextColorRect, m_statusColorToAnimateTo);
}

void BookingWidget::drawOverlayGradient(QPainter &painter,
                                        QRect &backgroundRectangle) {
  QLinearGradient gradient(backgroundRectangle.width() -
                               backgroundRectangle.width() / 3,
                           0, backgroundRectangle.width(), 0);
  gradient.setColorAt(0.0, QColor(255, 255, 255, 0));
  gradient.setColorAt(1.0, QColor(255, 255, 255, 90));
  painter.fillRect(backgroundRectangle, gradient);
}
