/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_IMAGEWIDGET_H
#define SRC_UI_IMAGEWIDGET_H

#include "MdiWidget.h"
#include "PixelDetailsWidget.h"

#include <NFITS/Image/ImageSliceSource.h>

#include <QWidget>

#include <filesystem>

namespace NFITS
{
    class Data;
}

namespace Nastro
{
    class ImageViewWidget;
    class HistogramWidget;
    class ImageControlsToolbar;
    class ImageRenderToolbar;

    /**
     * Mdi widget which allows the user to view/render the image slices provided by an ImageSliceSource.
     *
     * Provides toolbars for selecting image slices, setting render parameters, a view of the rendered slice, and a histogram.
     */
    class ImageWidget : public MdiWidget
    {
        Q_OBJECT

        public:

            explicit ImageWidget(std::unique_ptr<NFITS::ImageSliceSource> pImageSliceSource,
                                 std::optional<FileHDU> associatedHDU = std::nullopt,
                                 QWidget* pParent = nullptr);
            ~ImageWidget() override;

            [[nodiscard]] Type GetType() const override { return Type::Image; }

        private slots:

            void Slot_ImageControls_HistogramToggled(bool checked);
            void Slot_ImageControls_ExportTriggered(bool checked);

            void Slot_ImageRender_ParametersChanged(const NFITS::ImageRenderParams& params);

            void Slot_Histogram_MinVertLineChanged(double physicalValue, bool fromDrag);
            void Slot_Histogram_MaxVertLineChanged(double physicalValue, bool fromDrag);

            void Slot_ImageViewWidget_ImageViewPixelHovered(const std::optional<std::pair<std::size_t, std::size_t>>& pixelPos);

        private:

            void InitUI();

            void RebuildImageView(const NFITS::ImageRenderParams& params);
            void RebuildHistogram();

        private:

            std::unique_ptr<NFITS::ImageSliceSource> m_pImageSliceSource;

            NFITS::ImageSliceKey m_imageSliceKey{};
            NFITS::ImageRenderParams m_latestImageRenderParams{};

            ImageControlsToolbar* m_pImageControlsToolbar{nullptr};
            ImageRenderToolbar* m_pImageRenderToolbar{nullptr};
            PixelDetailsWidget* m_pPixelDetailsWidget{nullptr};
            ImageViewWidget* m_pImageViewWidget{nullptr};
            HistogramWidget* m_pHistogramWidget{nullptr};

            QWidget* m_pErrorWidget{nullptr};
    };
}

#endif //SRC_UI_IMAGEWIDGET_H
