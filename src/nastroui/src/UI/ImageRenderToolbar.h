/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_UI_IMAGERENDERTOOLBAR_H
#define NASTROUI_SRC_UI_IMAGERENDERTOOLBAR_H

#include "ProducedWidgetAction.h"

#include "../Image/ImageCommon.h"

#include <QToolBar>

namespace Nastro
{
    class ImageRenderToolbar : public QToolBar
    {
        Q_OBJECT

        public:

            explicit ImageRenderToolbar(QWidget* pParent = nullptr);

            [[nodiscard]] const ImageRenderParams& GetImageRenderParams() const noexcept { return m_imageRenderParams; }

            void SetScalingRange(ScalingRange scalingRange);

            void SetCustomScalingRangeMin(const std::optional<double>& value);
            void SetCustomScalingRangeMax(const std::optional<double>& value);

        signals:

            void Signal_OnImageRenderParamsChanged(const ImageRenderParams& imageRenderParams);

        private:

            void InitUI();

            void SetCustomScalingRangeVal(std::optional<double>& target, const std::optional<double>& value);

            [[nodiscard]] WidgetProducer GetInvertWidgetProducer();
            [[nodiscard]] WidgetProducer GetTransferFunctionWidgetProducer();
            [[nodiscard]] WidgetProducer GetColorMapWidgetProducer();
            [[nodiscard]] WidgetProducer GetScalingModeWidgetProducer();
            [[nodiscard]] WidgetProducer GetScalingRangeWidgetProducer();

            template <typename T>
            void UpdateImageRenderParam(T& target, const T& newValue)
            {
                const bool changed = target != newValue;
                target = newValue;
                if (changed) { emit Signal_OnImageRenderParamsChanged(m_imageRenderParams); }
            }

        private:

            ImageRenderParams m_imageRenderParams{};
    };
}

#endif //NASTROUI_SRC_UI_IMAGERENDERTOOLBAR_H
