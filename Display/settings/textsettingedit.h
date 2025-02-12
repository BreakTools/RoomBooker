#ifndef TEXTSETTINGEDIT_H
#define TEXTSETTINGEDIT_H

#include <QLineEdit>
#include <QSettings>
#include <QString>
#include <QWidget>

class TextSettingEdit : public QWidget {
  Q_OBJECT
public:
  explicit TextSettingEdit(QString textToDisplay, QString settingString,
                           QWidget *parent = nullptr);

private:
  QString m_settingString;
  QSettings *m_settings;
  QLineEdit *m_textInput;
  void onTextInputChanged(QString newText);
};

#endif // TEXTSETTINGEDIT_H
