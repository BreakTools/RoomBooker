#ifndef DATAFETCHINGHANDLER_H
#define DATAFETCHINGHANDLER_H

#include "datatypes.h"
#include <QList>
#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

class DataFetchingHandler : public QObject {
  Q_OBJECT
public:
  explicit DataFetchingHandler(QObject *parent = nullptr);
  void getBookingData();

signals:
  void currentBookingChanged(CurrentBookingData);
  void upcomingBookingsChanged(QList<UpcomingBookingData>);

private:
  QNetworkAccessManager *m_networkManager;
  QNetworkRequest *m_getRequest;
  CurrentBookingData m_storedCurrentBookingData;
  QList<UpcomingBookingData> m_storedUpcomingBookings;

  QString m_apiAddress;
  int m_roomId;
  QString m_constructedURL;
  QString m_unbookedNameText;
  QString m_unbookedInfoText;
  QString m_unbookedStatusText;
  QString m_bookedUsernamePrefixText;
  QString m_timeZoneText;

  void retrieveSettings();
  void onBookingDataRequestFinished(QNetworkReply *reply);
  QJsonObject getParsedBookingDataObjectFromReplyString(QString replyString);

  CurrentBookingData getErrorCurrentBookingData();
  CurrentBookingData getCurrentBookingData(QJsonObject parsedBookingDataObject);
  CurrentBookingData getBookedBookingData(QJsonObject parsedBookingDataObject);
  CurrentBookingData getUnbookedBookingData();

  void processUpcomingBookings(QList<UpcomingBookingData> upcomingBookings);
  QList<UpcomingBookingData>
  getUpcomingBookings(QJsonObject parsedBookingDataObject);
  UpcomingBookingData
  getUpcomingBookingData(QJsonObject parsedBookingDataObject,
                         QString upcomingBookingText);

  QString getSystemTimezoneId();
  QString getTimeStringFromStartEndTime(int startTime, int endTime);
};

#endif // DATAFETCHINGHANDLER_H
