/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_IMAGEWIDGET_H
#define SRC_UI_IMAGEWIDGET_H

#include "MdiHDUWidget.h"

#include "../Image/ImageView.h"

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

    class ImageWidget : public MdiHDUWidget
    {
        Q_OBJECT

        public:

            ImageWidget(std::filesystem::path filePath, uintmax_t hduIndex, std::unique_ptr<NFITS::Data> imageData, QWidget* pParent = nullptr);
            ~ImageWidget() override;

            [[nodiscard]] Type GetType() const override { return Type::Image; }

        private slots:

            void Slot_ImageControls_HistogramToggled(bool checked);
            void Slot_ImageControls_ExportTriggered(bool checked);

            void Slot_ImageRender_ParametersChanged(const ImageRenderParams& params);

            void Slot_Histogram_MinVertLineChanged(double physicalValue, bool fromDrag);
            void Slot_Histogram_MaxVertLineChanged(double physicalValue, bool fromDrag);

        private:

            void InitUI();

            void RebuildImageView(const ImageRenderParams& params);
            void RebuildHistogram();

        private:

            std::unique_ptr<NFITS::Data> m_imageData;

            NFITS::ImageSlice m_imageSlice{};
            ImageRenderParams m_latestImageRenderParams{};

            ImageControlsToolbar* m_pImageControlsToolbar{nullptr};
            ImageRenderToolbar* m_pImageRenderToolbar{nullptr};
            ImageViewWidget* m_pImageViewWidget{nullptr};
            HistogramWidget* m_pHistogramWidget{nullptr};

            QWidget* m_pErrorWidget{nullptr};
    };
}

#endif //SRC_UI_IMAGEWIDGET_H
