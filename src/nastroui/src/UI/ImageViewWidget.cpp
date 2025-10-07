/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageViewWidget.h"

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
    setDragMode(QGraphicsView::ScrollHandDrag); // Allow mouse dragging to pan
    setMouseTracking(true); // Allow mouse hover to generate mouseMoveEvents
}

ImageViewWidget::~ImageViewWidget() = default;

void ImageViewWidget::SetImageView(const NFITS::ImageView& imageView)
{
    m_imageView = imageView;

    RebuildScene();
}

QImage ImageViewWidget::GetCurrentViewRender()
{
    auto qImage = QImage(viewport()->size(), QImage::Format_ARGB32);
    qImage.fill(Qt::transparent); // TODO: Make configurable

    auto painter = QPainter(&qImage);
    render(&painter);

    return qImage;
}

static QImage QImageFromImageRender(const NFITS::ImageRender& imageRender)
{
    QImage::Format qImageFormat{};
    switch (imageRender.format)
    {
        case NFITS::ImageRender::Format::RGB888: qImageFormat = QImage::Format::Format_RGB888; break;
    }

    auto qImage = QImage(static_cast<int>(imageRender.width), static_cast<int>(imageRender.height), qImageFormat);

    for (std::size_t y = 0; y < imageRender.height; ++y)
    {
        // ImageRenders are stored bottom to top, whereas QImage expects top to bottom, so inverse fill
        // the QImage from data from ImageRender rows
        const auto pImageRenderScanLine = imageRender.GetScanLineBytesStart(imageRender.height - y - 1);
        const auto pQImageScanLine = qImage.scanLine(static_cast<int>(y));

        for (std::size_t x = 0; x < imageRender.width; ++x)
        {
            for (std::size_t comp = 0; comp < imageRender.BytesPerPixel(); ++comp)
            {
                pQImageScanLine[(x * 3) + comp] = pImageRenderScanLine[((x * 3) + comp)];
            }
        }
    }

    return qImage;
}

void ImageViewWidget::RebuildScene()
{
    const auto& imageRender = m_imageView.GetImageRender();

    const auto qImage = QImageFromImageRender(imageRender);
    if (qImage.isNull())
    {
        return;
    }

    const auto pixmap = QPixmap::fromImage(qImage);
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

void ImageViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);

    if (m_pPixmapItem == nullptr) { return; }

    const auto viewPos = event->pos();
    const auto scenePos = mapToScene(viewPos);
    const auto itemPos = m_pPixmapItem->mapFromScene(scenePos);

    // Using floor to get pixelPos rather than itemPos.toPoint() so
    // that pixel pos stays constant over the full area of a pixel,
    // only changing at boundaries between pixels
    auto pixelPos = QPoint(
        static_cast<int>(std::floor(itemPos.x())),
        static_cast<int>(std::floor(itemPos.y()))
    );

    const auto pixmap = m_pPixmapItem->pixmap();
    if (pixmap.rect().contains(pixelPos))
    {
        // Invert y-coordinates as Qt uses top-left as origin whereas FITS uses bottom-left
        pixelPos.setY(pixmap.height() - pixelPos.y() - 1);

        emit Signal_OnImageViewPixelHovered(std::make_pair(pixelPos.x(), pixelPos.y()));
    }
    else
    {
        emit Signal_OnImageViewPixelHovered(std::nullopt);
    }
}

void ImageViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);

    // When no longer interacting / dragging, change the cursor
    // back to cross cursor (see: enterEvent)
    viewport()->setCursor(Qt::CrossCursor);
}

void ImageViewWidget::enterEvent(QEnterEvent* event)
{
    QGraphicsView::enterEvent(event);

    // Use the cross cursor when mouse is hovered over the widget, rather than
    // the default hand cursor; easier to see which pixel is hovered
    viewport()->setCursor(Qt::CrossCursor);
}

void ImageViewWidget::leaveEvent(QEvent* event)
{
    // Clear hovered pixel when the mouse leaves the widget's area
    emit Signal_OnImageViewPixelHovered(std::nullopt);

    QWidget::leaveEvent(event);
}

}
