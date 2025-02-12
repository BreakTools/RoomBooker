#ifndef BOOKINGINFO_H
#define BOOKINGINFO_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QWidget>

class BookingInfo : public QWidget {
  Q_OBJECT
public:
  explicit BookingInfo(QWidget *parent = nullptr);
  void changeBookingInfo(QString newBookingInfoText);

private:
  QString *m_bookingInfoText;
  QLabel *m_bookingInfoLabel;
  int m_maxCharacters;

  QPropertyAnimation *m_fadeInPropertyAnimation;
  QPropertyAnimation *m_fadeOutPropertyAnimation;

  void configureLabelStyling();
  void configureAnimations();
  void fadeOutOldInfo();
  void fadeInNewInfo();
};

#endif // BOOKINGINFO_H
