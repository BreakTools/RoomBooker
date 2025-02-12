#ifndef COLORSETTINGEDIT_H
#define COLORSETTINGEDIT_H

#include <QColor>
#include <QSettings>
#include <QWidget>

class QFrame;

class ColorSettingEdit : public QWidget {
  Q_OBJECT
public:
  explicit ColorSettingEdit(QString textToDisplay, QString settingString,
                            QWidget *parent = nullptr);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void selectColor();
  void updateColorFrame();

  QString m_settingString;
  QSettings *m_settings;
  QColor m_currentColor;
  QFrame *m_colorFrame;
};

#endif // COLORSETTINGEDIT_H
