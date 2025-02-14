#include "datafetchinghandler.h"
#include "settingstrings.h"
#include <QApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTimeZone>
#include <QTimer>
#include <QtNetwork/QNetworkReply>

DataFetchingHandler::DataFetchingHandler(QObject *parent)
    : QObject{parent}, m_networkManager(new QNetworkAccessManager()),
      m_getRequest(new QNetworkRequest()) {
  retrieveSettings();
  QTimer *m_getRequestTimer = new QTimer(this);
  connect(m_getRequestTimer, &QTimer::timeout, this,
          &DataFetchingHandler::getBookingData);
  connect(m_networkManager, &QNetworkAccessManager::finished, this,
          &DataFetchingHandler::onBookingDataRequestFinished);
  connect(qApp, &QApplication::aboutToQuit, this,
          &DataFetchingHandler::stopDataFetching);
  m_getRequestTimer->start(1000);
}

void DataFetchingHandler::retrieveSettings() {
  QSettings settings;
  m_apiAddress = settings.value(API_ADDRESS_SETTING).toString();
  m_roomId = settings.value(ROOM_ID_SETTING).toInt();
  m_timeZoneText = getSystemTimezoneId();
  m_unbookedNameText = settings.value(UNBOOKED_NAME_TEXT_SETTING).toString();
  m_unbookedInfoText = settings.value(UNBOOKED_INFO_TEXT_SETTING).toString();
  m_unbookedStatusText =
      settings.value(UNBOOKED_STATUS_TEXT_SETTING).toString();
  m_bookedUsernamePrefixText =
      settings.value(BOOKED_USERNAME_PREFIX_TEXT_SETTING).toString();

  m_constructedURL = m_apiAddress + "/rooms/" + QString::number(m_roomId) +
                     "/" + m_timeZoneText;
}

void DataFetchingHandler::getBookingData() {
  m_getRequest->setUrl(QUrl(m_constructedURL));
  m_networkManager->get(*m_getRequest);
}

void DataFetchingHandler::onBookingDataRequestFinished(
    QNetworkReply *getRequestReply) {

  CurrentBookingData newCurrentBookingData;

  if (getRequestReply->error()) {
    newCurrentBookingData = getErrorCurrentBookingData();

    if (getRequestReply->error() == QNetworkReply::ContentNotFoundError) {
      newCurrentBookingData.name = QString("ERROR: Room ID not found");
      newCurrentBookingData.info = QString("Please provide a valid room ID.");
    }

    if (newCurrentBookingData != m_storedCurrentBookingData) {
      m_storedCurrentBookingData = newCurrentBookingData;

      emit currentBookingChanged(m_storedCurrentBookingData);
    }
    return;
  }

  QString replyString = getRequestReply->readAll();
  QJsonObject parsedBookingDataObject =
      getParsedBookingDataObjectFromReplyString(replyString);

  newCurrentBookingData = getCurrentBookingData(parsedBookingDataObject);

  if (newCurrentBookingData != m_storedCurrentBookingData) {
    m_storedCurrentBookingData = newCurrentBookingData;
    emit currentBookingChanged(m_storedCurrentBookingData);
  }

  QList<UpcomingBookingData> upcomingBookings =
      getUpcomingBookings(parsedBookingDataObject);
  processUpcomingBookings(upcomingBookings);
}

QJsonObject DataFetchingHandler::getParsedBookingDataObjectFromReplyString(
    QString replyString) {
  QJsonDocument parsedBookingDataDocument =
      QJsonDocument::fromJson(replyString.toUtf8());
  QJsonObject parsedBookingDataObject = parsedBookingDataDocument.object();
  return parsedBookingDataObject;
}

CurrentBookingData DataFetchingHandler::getErrorCurrentBookingData() {
  CurrentBookingData errorCurrentBookingData;
  errorCurrentBookingData.state = CurrentBookingState::ERROR;
  errorCurrentBookingData.id = "error";
  errorCurrentBookingData.name = "Connection problem!";
  errorCurrentBookingData.info = "Is the backend running properly?";
  errorCurrentBookingData.status = "???";

  return errorCurrentBookingData;
}

CurrentBookingData DataFetchingHandler::getCurrentBookingData(
    QJsonObject parsedBookingDataObject) {
  QString id =
      parsedBookingDataObject[QString("current_booking")][QString("id")]
          .toString();

  if (id == "") {
    return getUnbookedBookingData();
  } else {
    return getBookedBookingData(parsedBookingDataObject);
  }
}

CurrentBookingData DataFetchingHandler::getUnbookedBookingData() {
  CurrentBookingData currentBookingData;

  currentBookingData.state = CurrentBookingState::UNBOOKED;
  currentBookingData.id = "";
  currentBookingData.name = m_unbookedNameText;
  currentBookingData.info = m_unbookedInfoText;
  currentBookingData.status = m_unbookedStatusText;

  return currentBookingData;
}

CurrentBookingData
DataFetchingHandler::getBookedBookingData(QJsonObject parsedBookingDataObject) {
  CurrentBookingData currentBookingData;

  currentBookingData.state = CurrentBookingState::BOOKED;
  currentBookingData.id =
      parsedBookingDataObject[QString("current_booking")][QString("id")]
          .toString();
  currentBookingData.name =
      parsedBookingDataObject[QString("current_booking")][QString("name")]
          .toString();
  currentBookingData.info =
      m_bookedUsernamePrefixText +
      parsedBookingDataObject[QString("current_booking")][QString("user")]
          .toString();
  currentBookingData.status = getTimeStringFromStartEndTime(
      parsedBookingDataObject[QString("current_booking")][QString("start_time")]
          .toInt(),
      parsedBookingDataObject[QString("current_booking")][QString("end_time")]
          .toInt());

  return currentBookingData;
}

void DataFetchingHandler::processUpcomingBookings(
    QList<UpcomingBookingData> upcomingBookings) {

  if (upcomingBookings != m_storedUpcomingBookings) {
    emit upcomingBookingsChanged(upcomingBookings);
    m_storedUpcomingBookings = upcomingBookings;
  }
}

QList<UpcomingBookingData>
DataFetchingHandler::getUpcomingBookings(QJsonObject parsedBookingDataObject) {
  QList<UpcomingBookingData> upcomingBookings;
  upcomingBookings.append(getUpcomingBookingData(parsedBookingDataObject,
                                                 "first_upcoming_booking"));
  upcomingBookings.append(getUpcomingBookingData(parsedBookingDataObject,
                                                 "second_upcoming_booking"));
  upcomingBookings.append(getUpcomingBookingData(parsedBookingDataObject,
                                                 "third_upcoming_booking"));
  return upcomingBookings;
}

UpcomingBookingData
DataFetchingHandler::getUpcomingBookingData(QJsonObject parsedBookingDataObject,
                                            QString upcomingBookingText) {
  UpcomingBookingData upcomingBooking;
  upcomingBooking.id =
      parsedBookingDataObject[upcomingBookingText][QString("id")].toString();
  upcomingBooking.name =
      parsedBookingDataObject[upcomingBookingText][QString("name")].toString();
  upcomingBooking.timeString = getTimeStringFromStartEndTime(
      parsedBookingDataObject[upcomingBookingText][QString("start_time")]
          .toInt(),
      parsedBookingDataObject[upcomingBookingText][QString("end_time")]
          .toInt());

  return upcomingBooking;
}

QString DataFetchingHandler::getSystemTimezoneId() {
  QTimeZone tz = QTimeZone::systemTimeZone();
  QString id = QString::fromUtf8(tz.id());
  id.replace("/", "&");
  return id;
}

QString DataFetchingHandler::getTimeStringFromStartEndTime(int startTime,
                                                           int endTime) {
  if (startTime == 0) {
    return QString("");
  }

  QDateTime start = QDateTime::fromSecsSinceEpoch(startTime);
  QDateTime end = QDateTime::fromSecsSinceEpoch(endTime + 1);
  return start.toString("hh:mm") + " - " + end.toString("hh:mm");
}

void DataFetchingHandler::stopDataFetching() {
  m_getRequestTimer->stop();
  delete m_getRequestTimer;
}
