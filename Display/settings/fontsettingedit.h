#ifndef FONTSETTINGEDIT_H
#define FONTSETTINGEDIT_H

#include <QFont>
#include <QSettings>
#include <QWidget>

class QLabel;

class FontSettingEdit : public QWidget {
  Q_OBJECT
public:
  explicit FontSettingEdit(QString textToDisplay, QString settingString,
                           QWidget *parent = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void selectFont();
  void updateFontDisplay();

  QString m_settingString;
  QSettings *m_settings;
  QFont m_currentFont;
  QLabel *m_fontLabel;
};

#endif // FONTSETTINGEDIT_H
