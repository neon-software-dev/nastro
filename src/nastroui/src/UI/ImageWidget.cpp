/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageWidget.h"
#include "ImageViewWidget.h"
#include "AxisSliderWidget.h"
#include "AxisSpinWidget.h"
#include "HistogramWidget.h"
#include "ImageControlsToolbar.h"
#include "ImageRenderToolbar.h"
#include "PixelDetailsWidget.h"

#include "../VM/MainWindowVM.h"

#include <NFITS/Data/ImageData.h>
#include <NFITS/Util/ImageUtil.h>
#include <NFITS/Image/FlattenedImageSliceSource.h>

#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QSplitter>

namespace Nastro
{

ImageWidget::ImageWidget(std::unique_ptr<NFITS::ImageSliceSource> pImageSliceSource,
                         MainWindowVM* pMainWindowVM,
                         std::optional<FileHDU> associatedHDU,
                         QWidget* pParent)
    : MdiWidget(std::move(associatedHDU), pParent)
    , m_pImageSliceSource(std::move(pImageSliceSource))
    , m_pMainWindowVM(pMainWindowVM)
    , m_imageSliceKey(NFITS::GetDefaultSliceKey(m_pImageSliceSource->GetImageSliceSpan()))
{
    InitUI();
}

ImageWidget::~ImageWidget() = default;

void ImageWidget::InitUI()
{
    //
    // Image Controls Toolbar
    //
    m_pImageControlsToolbar = new ImageControlsToolbar();
    connect(m_pImageControlsToolbar, &ImageControlsToolbar::Signal_OnDisplayHistogramToggled, this, &ImageWidget::Slot_ImageControls_HistogramToggled);
    connect(m_pImageControlsToolbar, &ImageControlsToolbar::Signal_OnExportTriggered, this, &ImageWidget::Slot_ImageControls_ExportTriggered);

    //
    // Image Render Toolbar
    //
    m_pImageRenderToolbar = new ImageRenderToolbar();
    connect(m_pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged, this, &ImageWidget::Slot_ImageRender_ParametersChanged);

    //
    // Axis Selection Toolbars
    //
    std::vector<QToolBar*> pSelectionToolbars;

    const auto sliceSpan = m_pImageSliceSource->GetImageSliceSpan();

    // Append axis selection toolbars for every slice axis past the first two
    for (std::size_t x = 2; x < sliceSpan.axes.size(); ++x)
    {
        const auto& axisSpan = sliceSpan.axes.at(x);

        // Don't provide a selection widget for an axis unless it has a size > 1; nothing to select otherwise
        if (axisSpan <= 1)
        {
            continue;
        }

        AxisWidget* pAxisWidget{nullptr};

        // Provide a special slider widget specifically for axis 3
        if (x == 2)
        {
            pAxisWidget = new AxisSliderWidget(static_cast<unsigned int>(x + 1), axisSpan);
        }
        // For all other axes after axis 3, provide a simple spin widget
        else
        {
            pAxisWidget = new AxisSpinWidget(static_cast<unsigned int>(x + 1), axisSpan);
        }

        connect(pAxisWidget, &AxisWidget::Signal_ValueChanged, [=, this](int val) {
            m_imageSliceKey.axesValues.at(x - 2) = val;

            const auto scalingPerImage = m_pImageRenderToolbar->GetImageRenderParams().scalingMode == NFITS::ScalingMode::PerImage;

            // If scaling per image, and slice changed, null out any custom scaling range that might have existed for
            // the previous slice, don't carry it forward to the new slice. TODO: Evaluate whether or not to uncomment this.
            /*if (scalingPerImage)
            {
                m_pImageRenderToolbar->SetCustomScalingRangeMin(std::nullopt);
                m_pImageRenderToolbar->SetCustomScalingRangeMax(std::nullopt);
            }*/

            // Rebuild the image view whenever slice is changed
            RebuildImageView(m_pImageRenderToolbar->GetImageRenderParams());

            // Rebuild the histogram when slice is changed, if we're in per-image scaling range
            if (scalingPerImage)
            {
                RebuildHistogram();
            }
        });

        auto pSelectionToolbar = new QToolBar();
        pSelectionToolbar->addWidget(pAxisWidget);

        pSelectionToolbars.push_back(pSelectionToolbar);
    }

    //
    // Pixel Details Widget
    //
    m_pPixelDetailsWidget = new PixelDetailsWidget();

    //
    // Image View
    //
    m_pImageViewWidget = new ImageViewWidget();
    connect(m_pImageViewWidget, &ImageViewWidget::Signal_OnImageViewPixelHovered, this, &ImageWidget::Slot_ImageViewWidget_ImageViewPixelHovered);

    //
    // Histogram View
    //
    m_pHistogramWidget = new HistogramWidget();
    m_pHistogramWidget->setMinimumHeight(300);
    m_pHistogramWidget->hide();
    connect(m_pHistogramWidget, &HistogramWidget::Signal_OnMinVertLineChanged, this, &ImageWidget::Slot_Histogram_MinVertLineChanged);
    connect(m_pHistogramWidget, &HistogramWidget::Signal_OnMaxVertLineChanged, this, &ImageWidget::Slot_Histogram_MaxVertLineChanged);

    //
    // Image / Histogram Splitter
    //
    auto pSplitter = new QSplitter(Qt::Vertical);
    pSplitter->setStretchFactor(0, 1); // image expands
    pSplitter->setStretchFactor(1, 0); // histogram minimal

    pSplitter->addWidget(m_pImageViewWidget);
    pSplitter->addWidget(m_pHistogramWidget);

    pSplitter->setCollapsible(1, false);

    //
    // Error View
    //
    m_pErrorWidget = new QLabel(tr("Failed to render image"));
    m_pErrorWidget->hide();

    //
    // Layout
    //
    auto pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pImageControlsToolbar);
    pMainLayout->addWidget(m_pImageRenderToolbar);
    for (const auto& pSelectionToolbar : pSelectionToolbars)
    {
        pMainLayout->addWidget(pSelectionToolbar);
    }
    pMainLayout->addWidget(m_pPixelDetailsWidget);
    pMainLayout->addWidget(pSplitter, 1);
    pMainLayout->addWidget(m_pErrorWidget);

    //
    // Build the image view and histogram from our initial data/params
    //
    RebuildImageView(m_pImageRenderToolbar->GetImageRenderParams());
    RebuildHistogram();
}

void ImageWidget::RebuildImageView(const NFITS::ImageRenderParams& params)
{
    const auto imageSlice = m_pImageSliceSource->GetImageSlice(m_imageSliceKey);
    if (!imageSlice)
    {
        return;
    }

    auto imageView = NFITS::ImageView::Render(*imageSlice, params);
    if (imageView)
    {
        m_pImageViewWidget->SetImageView(std::move(*imageView));
    }

    m_pErrorWidget->setVisible(!imageView.has_value());
}

void ImageWidget::RebuildHistogram()
{
    const auto imageSlice = m_pImageSliceSource->GetImageSlice(m_imageSliceKey);
    if (!imageSlice)
    {
        return;
    }

    const auto imageRenderParams = m_pImageRenderToolbar->GetImageRenderParams();

    NFITS::PhysicalStats physicalStats{};

    switch (imageRenderParams.scalingMode)
    {
        case NFITS::ScalingMode::PerImage: physicalStats = imageSlice->physicalStats; break;
        case NFITS::ScalingMode::PerCube: physicalStats = imageSlice->cubePhysicalStats; break;
    }

    std::pair<double, double> scalingRange;

    switch (imageRenderParams.scalingRange)
    {
        case NFITS::ScalingRange::Full:
        {
            scalingRange = physicalStats.minMax;
        }
        break;
        case NFITS::ScalingRange::Custom:
        {
            const auto minValue = imageRenderParams.customScalingRangeMin ? *imageRenderParams.customScalingRangeMin : physicalStats.minMax.first;
            const auto maxValue = imageRenderParams.customScalingRangeMax ? *imageRenderParams.customScalingRangeMax : physicalStats.minMax.second;

            scalingRange = {minValue, maxValue};
        }
        break;
        case NFITS::ScalingRange::p99:
        {
            scalingRange = NFITS::CalculatePercentileRange(physicalStats, 0.99f);
        }
        break;
        case NFITS::ScalingRange::p95:
        {
            scalingRange = NFITS::CalculatePercentileRange(physicalStats, 0.95f);
        }
        break;
    }

    m_pHistogramWidget->DisplayHistogram(physicalStats, scalingRange.first, scalingRange.second);
}

void ImageWidget::Slot_ImageControls_HistogramToggled(bool checked)
{
    m_pHistogramWidget->setVisible(checked);
}

void ImageWidget::Slot_ImageControls_ExportTriggered(bool)
{
    QString selectedFilter;

    auto fileName = QFileDialog::getSaveFileName(this, "Export Image", QString(), tr("PNG (*.png);;BMP (*.bmp);;JPG (*.jpg);;All files (*)"), &selectedFilter);
    if (fileName.isEmpty())
    {
        return;
    }

    // If the user chosen a format, but typed in a filename without an extension, append the appropriate extension
    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty())
    {
        if      (selectedFilter.contains("PNG")) { fileName += ".png"; }
        else if (selectedFilter.contains("BMP")) { fileName += ".bmp"; }
        else if (selectedFilter.contains("JPG")) { fileName += ".jpg"; }
    }

    bool qualityResult{false};
    const auto selectedQuality = QInputDialog::getInt(this, tr("Image Quality"), tr("Quality (1–100):"), 90, 1, 100, 1, &qualityResult);

    if (!qualityResult)
    {
        return;
    }

    const auto qImage = m_pImageViewWidget->GetCurrentViewRender();

    qImage.save(fileName, nullptr, selectedQuality);
}

void ImageWidget::Slot_ImageRender_ParametersChanged(const NFITS::ImageRenderParams& params)
{
    //
    // Take note of relevant parameters which changed, then update our latest render params state
    //
    const bool scalingRangeChanged = m_latestImageRenderParams.scalingRange != params.scalingRange;

    m_latestImageRenderParams = params;

    //
    // Rebuild the image view whenever image render params change
    //
    RebuildImageView(params);

    // TODO Perf: Not always, only if scaling range, mode, or custom scale range changed
    RebuildHistogram();

    //
    // Special case handling for parameters which changed
    //

    // If scaling range was newly set to Custom, force the Histogram to open
    if (scalingRangeChanged && (params.scalingRange == NFITS::ScalingRange::Custom))
    {
        m_pImageControlsToolbar->SetDisplayHistogram(true);
    }
}

void ImageWidget::Slot_Histogram_MinVertLineChanged(double physicalValue, bool fromDrag)
{
    // If the user is dragging a histogram vert line, force them to Custom scaling range
    if (fromDrag && (m_latestImageRenderParams.scalingRange != NFITS::ScalingRange::Custom))
    {
        m_pImageRenderToolbar->SetScalingRange(NFITS::ScalingRange::Custom);
    }

    m_pImageRenderToolbar->SetCustomScalingRangeMin(physicalValue);
}

void ImageWidget::Slot_Histogram_MaxVertLineChanged(double physicalValue, bool fromDrag)
{
    // If the user is dragging a histogram vert line, force them to Custom scaling range
    if (fromDrag && (m_latestImageRenderParams.scalingRange != NFITS::ScalingRange::Custom))
    {
        m_pImageRenderToolbar->SetScalingRange(NFITS::ScalingRange::Custom);
    }

    m_pImageRenderToolbar->SetCustomScalingRangeMax(physicalValue);
}

void ImageWidget::Slot_ImageViewWidget_ImageViewPixelHovered(const std::optional<std::pair<double, double>>& pixelCoord)
{
    // Handle nothing hovered / error determining pixel that's hovered
    if (!pixelCoord)
    {
        OnNewHoveredPixelDetails(std::nullopt);
        return;
    }

    // Fetch the image slice currently being displayed
    const auto imageSlice = m_pImageSliceSource->GetImageSlice(m_imageSliceKey);
    if (!imageSlice)
    {
        OnNewHoveredPixelDetails(std::nullopt);
        return;
    }
    const auto imageSliceSpan = m_pImageSliceSource->GetImageSliceSpan();
    const auto imageSliceWidth = static_cast<uintmax_t>(imageSliceSpan.axes.at(0));

    // Determine (zero-based) data index of the pixel being hovered, within its slice.
    // Subtracting 0.5 and flooring to convert to zero-based.
    const auto dataPos = std::pair<std::size_t, std::size_t>(
        static_cast<std::size_t>(std::floor(pixelCoord->first - 0.5f)),
        static_cast<std::size_t>(std::floor(pixelCoord->second - 0.5f))
    );
    const auto dataX = dataPos.first;
    const auto dataY = dataPos.second;
    const auto dataIndex = (dataY * imageSliceWidth) + dataX;

    assert(dataIndex < imageSlice->physicalValues.size());

    // Determine the fully dimensioned local pixel coordinate by combining the 2D pixel
    // coordinate with the rest of the axes selections from the active slice key.
    auto localPixelCoord = std::vector<double>{pixelCoord->first, pixelCoord->second};

    auto localSliceKey = m_imageSliceKey;

    // If displaying a FlattenedImageSliceSource, then m_imageSliceKey is in "flattened"/"global" slice
    // span space. WCS logic below requires the full pixel coordinate to be in local slice space, so we
    // need to special case ask the flattened source for the original local key.
    auto *pFlattenedSource = dynamic_cast<NFITS::FlattenedImageSliceSource*>(m_pImageSliceSource.get());
    if (pFlattenedSource != nullptr)
    {
        // Get the local slice key
        const auto localKey = pFlattenedSource->GetLocalKey(m_imageSliceKey);
        if (!localKey) { return; }

        localSliceKey = *localKey;
    }

    // Append slice key axes selection values to the local pixel coordinate
    for (const auto& axisValue : localSliceKey.axesValues)
    {
        localPixelCoord.push_back(static_cast<double>(axisValue));
    }

    std::vector<NFITS::WCSWorldCoord> wcsCoords;

    if (imageSlice->wcsParams)
    {
        const auto wcsCoordsExpect = NFITS::PixelCoordToWorldCoords(localPixelCoord, *imageSlice->wcsParams);
        if (wcsCoordsExpect)
        {
            wcsCoords = *wcsCoordsExpect;
        }
    }

    OnNewHoveredPixelDetails(PixelDetails{
        .pixelCoordinate = localPixelCoord,
        .physicalValue = imageSlice->physicalValues[dataIndex],
        .physicalUnit = imageSlice->physicalUnit,
        .wcsCoords = wcsCoords
    });
}

void ImageWidget::OnNewHoveredPixelDetails(const std::optional<PixelDetails>& pixelDetails)
{
    // Manually update our internal pixel details widget to display pixel coord/value. We do
    // this manually to handle multiple window case; we only want the one, relevant, pixel
    // details widget to display that information
    m_pPixelDetailsWidget->DisplayPixelDetails(pixelDetails);

    // Report to the main VM that a new pixel is hovered
    m_pMainWindowVM->OnPixelHovered(pixelDetails);
}

}
