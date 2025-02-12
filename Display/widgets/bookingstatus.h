#ifndef BOOKINGSTATUS_H
#define BOOKINGSTATUS_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QWidget>

class BookingStatus : public QWidget {
  Q_OBJECT
public:
  explicit BookingStatus(QWidget *parent = nullptr);
  void changeBookingStatus(QString newBookingStatusText);

private:
  QString *m_bookingStatusText;
  QLabel *m_bookingStatusLabel;

  QPropertyAnimation *m_fadeInPropertyAnimation;
  QPropertyAnimation *m_fadeOutPropertyAnimation;

  void configureLabelStyling();
  void configureAnimations();
  void fadeOutOldStatus();
  void fadeInNewStatus();
};

#endif // BOOKINGSTATUS_H
