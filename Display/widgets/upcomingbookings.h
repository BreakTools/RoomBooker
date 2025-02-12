#ifndef UPCOMINGBOOKINGS_H
#define UPCOMINGBOOKINGS_H

#include "../datatypes.h"
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QList>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QString>
#include <QWidget>

class UpcomingBooking : public QWidget {
  Q_OBJECT
public:
  explicit UpcomingBooking(QWidget *parent = nullptr);
  void updateBookingText(UpcomingBookingData upcomingBookingDataToDisplay);

private:
  QLabel *m_bookingTimeLabel;
  QLabel *m_bookingNameLabel;
  int m_maxCharacters;

  void configureLabelStyling();
};

class UpcomingBookings : public QWidget {
  Q_OBJECT
public:
  explicit UpcomingBookings(QWidget *parent = nullptr);
  void updateCurrentBooking(CurrentBookingData currentBooking);
  void updateUpcomingBookings(QList<UpcomingBookingData> upcomingBookingData);

private:
  QList<UpcomingBookingData> m_storedUpcomingBookingData;
  QString m_currentBookingId;
  UpcomingBooking *m_firstUpcomingBooking;
  UpcomingBooking *m_secondUpcomingBooking;
  UpcomingBooking *m_thirdUpcomingBooking;

  QPoint *firstUpcomingBookingStartingPosition;
  QGraphicsOpacityEffect *m_firstOpacityEffect;
  QGraphicsOpacityEffect *m_secondOpacityEffect;
  QGraphicsOpacityEffect *m_thirdOpacityEffect;
  QParallelAnimationGroup *m_fadeOutAllAnimationGroup;
  QPropertyAnimation *m_firstBookingPositionAnimation;
  QParallelAnimationGroup *m_fadeInAllAnimationGroup;

  void onFadeOutAnimationFinished();
  void onPositionFadeAnimationFinished();

  void configureOpacityEffects();
  void configureFadeInAnimations();
  void configureSingleFadeInAnimation(QGraphicsOpacityEffect *opacityEffect,
                                      int timingDelay, float offsetStep,
                                      float finalOpacity);

  void configureFadeOutAnimations();

  void configureSingleFadeOutAnimation(QGraphicsOpacityEffect *opacityEffect,
                                       int timingDelay, float offsetStep,
                                       float beginningOpacity);
  void configureFirstBookingPositionAnimation();
};

#endif // UPCOMINGBOOKINGS_H
