/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "SettingsRenderingWidget.h"

#include "../Settings.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QColorDialog>

namespace Nastro
{

SettingsRenderingWidget::SettingsRenderingWidget(QWidget* parent)
    : QWidget(parent)
{
    InitUI();
}

void SettingsRenderingWidget::InitUI()
{
    //
    // BLANK color
    //
    auto pBlankColorRow = new QWidget();

    m_pColorSwatchFrame = new QFrame();
    m_pColorSwatchFrame->setFixedSize(32, 18);
    m_pColorSwatchFrame->setFrameShape(QFrame::Box);
    m_pColorSwatchFrame->setFrameShadow(QFrame::Sunken);

    auto pBlankPixelButton = new QPushButton(tr("Change"));
    connect(pBlankPixelButton, &QPushButton::pressed, this, &SettingsRenderingWidget::Slot_OnBlankPixelColorTriggered);

    auto pBlankColorLayout = new QHBoxLayout(pBlankColorRow);
    pBlankColorLayout->addWidget(m_pColorSwatchFrame);
    pBlankColorLayout->addWidget(pBlankPixelButton);

    Sync_BlankPixelColor_FromSetting();

    //
    // Main layout
    //
    auto pFormLayout = new QFormLayout(this);

    pFormLayout->addRow(tr("BLANK pixel color:"), pBlankColorRow);
}

void SettingsRenderingWidget::Slot_OnBlankPixelColorTriggered()
{
    const auto settingsColor = SafeGetSetting<QColor>(m_settings, SETTINGS_RENDERING_BLANK_COLOR, DEFAULT_BLANK_COLOR);

    const auto chosenColor = QColorDialog::getColor(settingsColor, this);
    if (!chosenColor.isValid())
    {
        return;
    }

    m_settings.setValue(SETTINGS_RENDERING_BLANK_COLOR, chosenColor);
    Sync_BlankPixelColor_FromSetting();
}

void SettingsRenderingWidget::Sync_BlankPixelColor_FromSetting()
{
    const auto blankColor = SafeGetSetting<QColor>(m_settings, SETTINGS_RENDERING_BLANK_COLOR, DEFAULT_BLANK_COLOR);

    m_pColorSwatchFrame->setStyleSheet(QString("background-color: %1;").arg(blankColor.name()));
}

}
