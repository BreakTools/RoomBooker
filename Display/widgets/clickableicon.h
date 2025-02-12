#ifndef CLICKABLEICON_H
#define CLICKABLEICON_H

#include <QLabel>

class ClickableIcon : public QLabel {
  Q_OBJECT
public:
  ClickableIcon(QWidget *parent = nullptr);

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent *event) override;
};

#endif // CLICKABLEICON_H
