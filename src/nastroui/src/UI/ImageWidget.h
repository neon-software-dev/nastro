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

namespace Nastro
{
    class Data;
    class ImageViewWidget;

    class ImageWidget : public MdiHDUWidget
    {
        Q_OBJECT

        public:

            ImageWidget(std::filesystem::path filePath, uintmax_t hduIndex, std::unique_ptr<Data> imageData, QWidget* pParent = nullptr);
            ~ImageWidget() override;

            [[nodiscard]] Type GetType() const override { return Type::Image; }

        private slots:

            void Slot_UI_ExportAction_Triggered(bool checked);

        private:

            void InitUI();

            void RebuildImageView();

        private:

            std::unique_ptr<Data> m_imageData;

            ImageView::Selection m_imageViewSelection{};
            ImageView::Params m_imageViewParams{};
            ImageViewWidget* m_pImageViewWidget{nullptr};

            QWidget* m_pErrorWidget{nullptr};
    };
}

#endif //SRC_UI_IMAGEWIDGET_H
