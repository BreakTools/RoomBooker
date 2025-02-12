#include "iconsettingedit.h"
#include <QFileDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

IconSettingEdit::IconSettingEdit(QString textToDisplay, QString settingString,
                                 QWidget *parent)
    : QWidget(parent), m_settingString(settingString),
      m_settings(new QSettings(this)) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  QLabel *titleLabel = new QLabel(textToDisplay, this);
  layout->addWidget(titleLabel);

  QString storedPath = m_settings->value(m_settingString).toString();
  if (!QFile::exists(storedPath)) {
    storedPath = QString(":/resources/defaulticon.png");
  }

  m_currentPixmap.load(storedPath);
  m_iconLabel = new QLabel(this);
  m_iconLabel->setPixmap(m_currentPixmap.scaled(64, 64, Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
  m_iconLabel->installEventFilter(this);
  layout->addWidget(m_iconLabel);

  setLayout(layout);
}

bool IconSettingEdit::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() == Qt::LeftButton &&
        m_iconLabel->rect().contains(mouseEvent->pos())) {
      selectIcon();
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}

void IconSettingEdit::selectIcon() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Select Icon"), QString(), tr("PNG Files (*.png)"));
  if (!fileName.isEmpty()) {
    m_currentPixmap.load(fileName);
    m_settings->setValue(m_settingString, fileName);
    updateIconDisplay();
  }
}

void IconSettingEdit::updateIconDisplay() {
  m_iconLabel->setPixmap(m_currentPixmap.scaled(64, 64, Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}
