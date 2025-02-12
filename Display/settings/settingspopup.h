#ifndef SETTINGSPOPUP_H
#define SETTINGSPOPUP_H

#include <QDialog>
#include <QObject>
#include <QWidget>

class SettingsPopup : public QDialog {
  Q_OBJECT
public:
  SettingsPopup();

private:
  void populateWidgets();
  QWidget *getBottomButtons();
  void restartProgram();
};

#endif // SETTINGSPOPUP_H
