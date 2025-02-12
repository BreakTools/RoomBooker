#ifndef INTEGERSETTINGEDIT_H
#define INTEGERSETTINGEDIT_H

#include <QSettings>
#include <QSpinBox>
#include <QWidget>

class IntegerSettingEdit : public QWidget {
  Q_OBJECT
public:
  explicit IntegerSettingEdit(QString textToDisplay, QString settingString,
                              int maxValue, QWidget *parent = nullptr);

private:
  QString m_settingString;
  QSettings *m_settings;
  QSpinBox *m_integerInput;
  void onInputIntegerChanged(int newInteger);
};

#endif // INTEGERSETTINGEDIT_H
