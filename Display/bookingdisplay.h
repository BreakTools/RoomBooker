#ifndef BOOKINGDISPLAY_H
#define BOOKINGDISPLAY_H

#include <QApplication>

#include "datafetchinghandler.h"
#include "widgets/bookinginfo.h"
#include "widgets/bookingname.h"
#include "widgets/bookingstatus.h"
#include "widgets/bookingwidget.h"
#include "widgets/upcomingbookings.h"

class BookingDisplay : public QApplication {
public:
  BookingDisplay(int &argc, char **argv);

protected:
  bool notify(QObject *receiver, QEvent *event) override;

private:
  BookingWidget *m_bookingWidget;
  BookingName *m_bookingName;
  BookingInfo *m_bookingInfo;
  BookingStatus *m_bookingStatus;
  DataFetchingHandler *m_dataFetchingHandler;
  UpcomingBookings *m_upcomingBookings;
  QColor m_unbookedColor;
  QColor m_bookedColor;

  void addFontsToDatabase();
  void populateSettingsIfNeeded();
  void configureStoredColors();
  void configureWidgetLayout();
  void connectSignals();
  void updateCurrentBooking(CurrentBookingData newCurrentBooking);
  void openSettingsWindow();
};

#endif // BOOKINGDISPLAY_H
