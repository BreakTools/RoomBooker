#include "clickableicon.h"
#include "../settingstrings.h"
#include <QFile>
#include <QMouseEvent>
#include <QSettings>

ClickableIcon::ClickableIcon(QWidget *parent) {
  QSettings settings;

  QString imagePath = settings.value(ICON_FILE_PATH_SETTING).toString();
  if (!QFile::exists(imagePath)) {
    imagePath = QString(":/resources/defaulticon.png");
  }

  QPixmap iconPixmap(imagePath);
  QPixmap scaledIconPixmap =
      iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  this->setPixmap(scaledIconPixmap);
}

void ClickableIcon::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    emit clicked();
  }
}
