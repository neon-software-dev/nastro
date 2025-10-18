/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_IMAGEWIDGET_H
#define SRC_UI_IMAGEWIDGET_H

#include "MdiWidget.h"

#include "../Util/Common.h"

#include <NFITS/Image/ImageSliceSource.h>
#include <NFITS/WCS/WCS.h>

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
    class PixelDetailsWidget;
    class ImageControlsToolbar;
    class ImageRenderToolbar;
    class MainWindowVM;

    /**
     * Mdi widget which allows the user to view/render the image slices provided by an ImageSliceSource.
     *
     * Provides toolbars for selecting image slices, setting render parameters, a view of the rendered slice, and a histogram.
     */
    class ImageWidget : public MdiWidget
    {
        Q_OBJECT

        public:

            ImageWidget(std::unique_ptr<NFITS::ImageSliceSource> pImageSliceSource,
                        MainWindowVM* pMainWindowVM,
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

            void Slot_ImageViewWidget_ImageViewPixelHovered(const std::optional<std::pair<double, double>>& pixelCoord);

        private:

            void InitUI();

            void RebuildImageView(const NFITS::ImageRenderParams& params);
            void RebuildHistogram();

            void OnNewHoveredPixelDetails(const std::optional<PixelDetails>& pixelDetails);

        private:

            std::unique_ptr<NFITS::ImageSliceSource> m_pImageSliceSource;
            MainWindowVM* m_pMainWindowVM;

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
