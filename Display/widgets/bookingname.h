#ifndef BOOKINGNAME_H
#define BOOKINGNAME_H

#include <QLabel>
#include <QParallelAnimationGroup>
#include <QWidget>

class BookingName : public QWidget {
  Q_OBJECT
public:
  explicit BookingName(QWidget *parent = nullptr);
  void changeBookingName(QString newBookingNameText);

private:
  QString *m_bookingNameText;
  QLabel *m_bookingNameLabel;
  int m_maxCharacters;
  QParallelAnimationGroup *m_fadeInAnimationGroup;
  QParallelAnimationGroup *m_fadeOutAnimationGroup;

  void configureLabelStyling();
  void configureAnimations();
  void fadeOutOldName();
  void fadeInNewName();
};

#endif // BOOKINGNAME_H
