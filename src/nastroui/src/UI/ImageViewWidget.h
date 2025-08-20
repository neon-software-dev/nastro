/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_IMAGEVIEWWIDGET_H
#define SRC_UI_IMAGEVIEWWIDGET_H

#include "../Image/ImageView.h"

#include <QWidget>
#include <QGraphicsView>

#include <memory>

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

            void SetImageView(const ImageView& imageView);

        protected:

            void resizeEvent(QResizeEvent* pEvent) override;
            void wheelEvent(QWheelEvent* pEvent) override;

        private:

            void RebuildScene();

        private:

            ImageView m_imageView;

            QGraphicsScene* m_pScene{nullptr};
            QGraphicsPixmapItem* m_pPixmapItem{nullptr};
            bool m_initialFittingDone{false};
    };
}

#endif //SRC_UI_IMAGEVIEWWIDGET_H
