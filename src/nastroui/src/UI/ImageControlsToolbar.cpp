/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageControlsToolbar.h"

#include <QToolButton>

namespace Nastro
{

ImageControlsToolbar::ImageControlsToolbar(QWidget* pParent)
    : QToolBar(pParent)
{
    InitUI();
}

void ImageControlsToolbar::InitUI()
{
    addAction(new ProducedWidgetAction(this, GetHistogramWidgetProducer()));

    auto pExportAction = addAction(tr("Export"));
    pExportAction->setToolTip(tr("Export render to disk"));
    connect(pExportAction, &QAction::triggered, this, &ImageControlsToolbar::Signal_OnExportTriggered);
}

void ImageControlsToolbar::SetDisplayHistogram(bool checked)
{
    UpdateDisplayHistogram(m_displayHistogram, checked);
}

WidgetProducer ImageControlsToolbar::GetHistogramWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent){
        auto pImageControlsToolbar = dynamic_cast<ImageControlsToolbar*>(pOwner);

        auto pHistogramButton = new QToolButton(pParent);
        pHistogramButton->setText(tr("Histogram"));
        pHistogramButton->setToolTip(tr("Display histogram"));
        pHistogramButton->setCheckable(true);

        // Respond to the widget being set to a new value
        connect(pHistogramButton, &QToolButton::toggled, pHistogramButton, [=,this](bool checked){
            UpdateDisplayHistogram(m_displayHistogram, checked);
        });

        // Sync the widget to the latest parameter state
        connect(pImageControlsToolbar, &ImageControlsToolbar::Signal_OnDisplayHistogramToggled,
                pHistogramButton, [=](bool displayHistogram){
            pHistogramButton->setChecked(displayHistogram);
        });

        // Set initial widget state
        pHistogramButton->setChecked(m_displayHistogram);

        return pHistogramButton;
    };
}

}
