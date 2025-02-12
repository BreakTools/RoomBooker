#ifndef ICONSETTINGEDIT_H
#define ICONSETTINGEDIT_H

#include <QPixmap>
#include <QSettings>
#include <QWidget>

class QLabel;

class IconSettingEdit : public QWidget {
  Q_OBJECT
public:
  explicit IconSettingEdit(QString textToDisplay, QString settingString,
                           QWidget *parent = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void selectIcon();
  void updateIconDisplay();

  QString m_settingString;
  QSettings *m_settings;
  QPixmap m_currentPixmap;
  QLabel *m_iconLabel;
};

#endif // ICONSETTINGEDIT_H
