/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageViewWidget.h"

#include "../Data/ImageData.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>

namespace Nastro
{

ImageViewWidget::ImageViewWidget(QWidget* pParent)
    : QGraphicsView(pParent)
{
    m_pScene = new QGraphicsScene(this);
    setScene(m_pScene);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

ImageViewWidget::~ImageViewWidget() = default;

void ImageViewWidget::SetImageView(const ImageView& imageView)
{
    m_imageView = imageView;

    RebuildScene();
}

void ImageViewWidget::RebuildScene()
{
    auto pixmap = QPixmap::fromImage(m_imageView.GetImage());
    if (pixmap.isNull())
    {
        return;
    }

    if (m_pPixmapItem != nullptr)
    {
        m_pScene->removeItem(m_pPixmapItem);
    }

    m_pPixmapItem = m_pScene->addPixmap(pixmap);
}

void ImageViewWidget::resizeEvent(QResizeEvent* pEvent)
{
    QGraphicsView::resizeEvent(pEvent);

    // We wait for a resizeEvent to fit the pixmap fully into the view, as only after this
    // widget is resized does it know its size and the fitInView will actually work. At the
    // moment we're fitting if old size is reported as invalid, which is basically when things
    // are being resized for the first time or when the widget is being massively recreated,
    // such as when you maximize the widget within a mdi area. Note that if you use the scroll
    // wheel to zoom in, which causes scroll bars to appear, a resizeEvent is sent then, and
    // we DON'T want to fitInView in that case, as that would undo the zooming that was just
    // done. At the moment m_initialFittingDone is unused; it was found insufficient as when
    // the widget is in a mdi window there can be multiple resize events as the widget and
    // mdi window are being created, and only responding to the first one isn't enough; the
    // final one has the actual size we need to be fitting to.
    if (m_pPixmapItem && !pEvent->oldSize().isValid())
    {
        fitInView(m_pPixmapItem, Qt::KeepAspectRatio);
        m_initialFittingDone = true;
    }
}

void ImageViewWidget::wheelEvent(QWheelEvent* pEvent)
{
    QGraphicsView::wheelEvent(pEvent);

    if (m_pPixmapItem == nullptr) { return; }

    const double scaleFactor = 1.15;
    if (pEvent->angleDelta().y() > 0)
    {
        scale(scaleFactor, scaleFactor); // zoom in
    }
    else
    {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor); // zoom out
    }
}

}
