#include "integersettingedit.h"
#include <QLabel>
#include <QVBoxLayout>

IntegerSettingEdit::IntegerSettingEdit(QString textToDisplay,
                                       QString settingString, int maxValue,
                                       QWidget *parent)
    : QWidget{parent}, m_settingString(settingString),
      m_settings(new QSettings()), m_integerInput(new QSpinBox()) {
  QVBoxLayout *layout = new QVBoxLayout();
  QLabel *titleText = new QLabel(textToDisplay);
  layout->addWidget(titleText);
  layout->setAlignment(Qt::AlignTop);
  int storedInteger = m_settings->value(m_settingString).toInt();

  m_integerInput->setRange(1, maxValue);
  m_integerInput->setValue(storedInteger);
  connect(m_integerInput, &QSpinBox::valueChanged, this,
          &IntegerSettingEdit::onInputIntegerChanged);
  layout->addWidget(m_integerInput);

  this->setLayout(layout);
}

void IntegerSettingEdit::onInputIntegerChanged(int newInteger) {
  m_settings->setValue(m_settingString, newInteger);
}
