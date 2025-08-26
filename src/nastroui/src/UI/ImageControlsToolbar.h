/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_IMAGECONTROLSTOOLBAR_H
#define NASTROUI_SRC_UI_IMAGECONTROLSTOOLBAR_H

#include "ProducedWidgetAction.h"

#include <QToolBar>

namespace Nastro
{
    class ImageControlsToolbar : public QToolBar
    {
        Q_OBJECT

        public:

            explicit ImageControlsToolbar(QWidget* pParent = nullptr);

            [[nodiscard]] bool GetDisplayHistogram() const noexcept { return m_displayHistogram; }
            void SetDisplayHistogram(bool checked);

        signals:

            void Signal_OnDisplayHistogramToggled(bool checked);
            void Signal_OnExportTriggered(bool checked);

        private:

            void InitUI();

            [[nodiscard]] WidgetProducer GetHistogramWidgetProducer();

            template <typename T>
            void UpdateDisplayHistogram(T& target, const T& newValue)
            {
                const bool changed = target != newValue;
                target = newValue;
                if (changed) { emit Signal_OnDisplayHistogramToggled(m_displayHistogram); }
            }

        private:

            bool m_displayHistogram{false};
    };
}

#endif //NASTROUI_SRC_UI_IMAGECONTROLSTOOLBAR_H
