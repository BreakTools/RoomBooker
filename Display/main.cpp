#include "bookingdisplay.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName("BreakTools");
  QCoreApplication::setApplicationName("RoomBooker");
  BookingDisplay bookingDisplay(argc, argv);
  return bookingDisplay.exec();
}
