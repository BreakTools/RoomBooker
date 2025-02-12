#include "textsettingedit.h"
#include <QLabel>
#include <QVBoxLayout>

TextSettingEdit::TextSettingEdit(QString textToDisplay, QString settingString,
                                 QWidget *parent)
    : QWidget{parent}, m_settingString(settingString),
      m_settings(new QSettings()), m_textInput(new QLineEdit()) {
  QVBoxLayout *layout = new QVBoxLayout();
  QLabel *titleText = new QLabel(textToDisplay);
  layout->addWidget(titleText);
  layout->setAlignment(Qt::AlignTop);

  QString storedText = m_settings->value(m_settingString).toString();
  m_textInput->setText(storedText);
  connect(m_textInput, &QLineEdit::textChanged, this,
          &TextSettingEdit::onTextInputChanged);
  layout->addWidget(m_textInput);

  this->setLayout(layout);
}

void TextSettingEdit::onTextInputChanged(QString newText) {
  m_settings->setValue(m_settingString, newText);
}
