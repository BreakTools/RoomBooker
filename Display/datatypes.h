#ifndef DATATYPES_H
#define DATATYPES_H

#include <QString>

enum class CurrentBookingState { UNBOOKED, BOOKED, ERROR };

struct CurrentBookingData {
  CurrentBookingState state = CurrentBookingState::ERROR;
  QString id = "";
  QString name = "";
  QString info = "";
  QString status = "";

  bool operator==(const CurrentBookingData &other) const {
    return id == other.id && status == other.status;
  }

  bool operator!=(const CurrentBookingData &other) const {
    return !(*this == other);
  }
};

struct UpcomingBookingData {
  QString id = "";
  QString name = "";
  QString timeString = "";

  bool operator==(const UpcomingBookingData &other) const {
    return id == other.id && timeString == other.timeString;
  }

  bool operator!=(const UpcomingBookingData &other) const {
    return !(*this == other);
  }
};

#endif // DATATYPES_H
