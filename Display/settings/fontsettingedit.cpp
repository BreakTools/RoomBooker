#include "FontSettingEdit.h"
#include <QFontDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

FontSettingEdit::FontSettingEdit(QString textToDisplay, QString settingString,
                                 QWidget *parent)
    : QWidget(parent), m_settingString(settingString),
      m_settings(new QSettings(this)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *titleLabel = new QLabel(textToDisplay, this);
  layout->addWidget(titleLabel);

  m_currentFont = m_settings->value(m_settingString).value<QFont>();
  m_fontLabel = new QLabel("Click me!", this);
  m_fontLabel->setFont(m_currentFont);
  m_fontLabel->installEventFilter(this);
  layout->addWidget(m_fontLabel);

  setLayout(layout);
  updateFontDisplay();
}

bool FontSettingEdit::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton &&
        rect().contains(mouseEvent->pos())) {
      selectFont();
      return true;
    }
  }
  return QObject::eventFilter(obj, event);
}

void FontSettingEdit::selectFont() {
  bool ok;
  QFont chosen = QFontDialog::getFont(&ok, m_currentFont, this);
  if (ok) {
    m_currentFont = chosen;
    m_settings->setValue(m_settingString, m_currentFont);
    updateFontDisplay();
  }
}

void FontSettingEdit::updateFontDisplay() {
  m_fontLabel->setFont(m_currentFont);
}
