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

            void SetImageView(NFITS::ImageView imageView);

            /**
             * @return A QImage sized the same as this widget, filled with the
             * current visual contents of the widget
             */
            [[nodiscard]] QImage GetCurrentViewRender();

        signals:

            /**
             * @param pixelCoord The pixel coordinate, in FITS-standard space, of the actively mouse-hovered pixel
             */
            void Signal_OnImageViewPixelHovered(const std::optional<std::pair<double, double>>& pixelCoord);

        protected:

            void resizeEvent(QResizeEvent* pEvent) override;
            void wheelEvent(QWheelEvent* pEvent) override;
            void mouseMoveEvent(QMouseEvent *event) override;
            void mouseReleaseEvent(QMouseEvent *event) override;
            void enterEvent(QEnterEvent *event) override;
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
