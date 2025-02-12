#include "ColorSettingEdit.h"
#include <QColorDialog>
#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPalette>
#include <QVBoxLayout>

ColorSettingEdit::ColorSettingEdit(QString textToDisplay, QString settingString,
                                   QWidget *parent)
    : QWidget(parent), m_settingString(settingString),
      m_settings(new QSettings(this)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *titleLabel = new QLabel(textToDisplay, this);
  layout->addWidget(titleLabel);

  m_currentColor = m_currentColor =
      m_settings->value(m_settingString).value<QColor>();
  m_colorFrame = new QFrame(this);
  m_colorFrame->setFixedHeight(40);
  m_colorFrame->setMinimumWidth(80);
  m_colorFrame->setAutoFillBackground(true);
  m_colorFrame->installEventFilter(this);

  updateColorFrame();
  layout->addWidget(m_colorFrame);
  setLayout(layout);
}

bool ColorSettingEdit::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_colorFrame && event->type() == QEvent::MouseButtonPress) {
    if (static_cast<QMouseEvent *>(event)->button() == Qt::LeftButton) {
      selectColor();
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void ColorSettingEdit::selectColor() {
  QColor chosenColor = QColorDialog::getColor(m_currentColor, this);
  if (chosenColor.isValid()) {
    m_currentColor = chosenColor;
    m_settings->setValue(m_settingString, m_currentColor);
    updateColorFrame();
  }
}

void ColorSettingEdit::updateColorFrame() {
  QPalette pal = m_colorFrame->palette();
  pal.setColor(QPalette::Window, m_currentColor);
  m_colorFrame->setPalette(pal);
}
