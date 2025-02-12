#ifndef BOOKINGWIDGET_H
#define BOOKINGWIDGET_H

#include <QColor>
#include <QPainter>
#include <QPropertyAnimation>
#include <QRect>
#include <QWidget>

class BookingWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(
      float slidingColorAnimationProgress READ slidingColorAnimationProgress
          WRITE setSlidingColorAnimationProgress)

public:
  explicit BookingWidget(QWidget *parent = nullptr);
  void changeStatusColor(QColor newColor);

private:
  QColor m_currentlyDisplayingStatusColor;
  QColor m_statusColorToAnimateTo;
  QColor m_backgroundColor;
  float m_slidingColorAnimationProgress;
  QPropertyAnimation *m_slidingColorAnimation;

  void configureAnimation();
  void onAnimationFinished();
  float slidingColorAnimationProgress() const;
  void setSlidingColorAnimationProgress(float progress);

  void drawBackgroundColor(QPainter &painter, QRect &backgroundRectangle);
  void drawAnimatedStatusColor(QPainter &painter, QRect &backgroundRectangle);
  void drawOverlayGradient(QPainter &painter, QRect &backgroundRectangle);

protected:
  void paintEvent(QPaintEvent *event) override;
};

#endif // BOOKINGWIDGET_H
