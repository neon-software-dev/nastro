/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_IMAGEVIEWWIDGET_H
#define SRC_UI_IMAGEVIEWWIDGET_H

#include <NFITS/Image/ImageView.h>

#include <QWidget>
#include <QGraphicsView>

#include <memory>
#include <utility>
#include <optional>

class QGraphicsScene;
class QGraphicsPixmapItem;

namespace Nastro
{
    class ImageViewWidget : public QGraphicsView
    {
        Q_OBJECT

        public:

            explicit ImageViewWidget(QWidget* pParent = nullptr);
            ~ImageViewWidget() override;

            void SetImageView(const NFITS::ImageView& imageView);

            /**
             * @return A QImage sized the same as this widget, filled with the
             * current visual contents of the widget
             */
            [[nodiscard]] QImage GetCurrentViewRender();

        signals:

            void Signal_OnImageViewPixelHovered(const std::optional<std::pair<std::size_t, std::size_t>>& pixelPos);

        protected:

            void resizeEvent(QResizeEvent* pEvent) override;
            void wheelEvent(QWheelEvent* pEvent) override;
            void mouseMoveEvent(QMouseEvent *event) override;
            void leaveEvent(QEvent *event) override;

        private:

            void RebuildScene();

        private:

            NFITS::ImageView m_imageView;

            QGraphicsScene* m_pScene{nullptr};
            QGraphicsPixmapItem* m_pPixmapItem{nullptr};
            bool m_initialFittingDone{false};
    };
}

#endif //SRC_UI_IMAGEVIEWWIDGET_H
