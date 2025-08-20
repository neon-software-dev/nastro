/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageWidget.h"
#include "ImageViewWidget.h"
#include "AxisSliderWidget.h"
#include "AxisSpinWidget.h"

#include "../Data/ImageData.h"

#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QActionGroup>

namespace Nastro
{

ImageWidget::ImageWidget(std::filesystem::path filePath, uintmax_t hduIndex, std::unique_ptr<Data> imageData, QWidget* pParent)
    : MdiHDUWidget(std::move(filePath), hduIndex, pParent)
    , m_imageData(std::move(imageData))
{
    InitUI();
}

ImageWidget::~ImageWidget() = default;

void AddMenuItems(QMenu* pMenu, QActionGroup* pActionGroup, const QString& checkedItem, const QStringList& items)
{
    for (const auto& item : items)
    {
        auto pAction = pMenu->addAction(item);
        pAction->setCheckable(true);
        if (item == checkedItem) { pAction->setChecked(true); }

        pActionGroup->addAction(pAction);
    }
}

void ImageWidget::InitUI()
{
    //
    // Image Params Toolbar
    //
    auto pParamsToolbar = new QToolBar();

    // Invert colors action
        auto pInvertButton = new QToolButton();
        pInvertButton->setCheckable(true);
        pInvertButton->setText("Invert");
        pInvertButton->setToolTip(tr("Invert colors"));
        connect(pInvertButton, &QToolButton::toggled, [this](bool checked){
            m_imageViewParams.invertColors = checked;
            RebuildImageView();
        });

    // Transfer function action
        auto transferFuncButton = new QToolButton();
        transferFuncButton->setText("Linear"); // WARNING: Must match default ImageView transfer function value
        transferFuncButton->setPopupMode(QToolButton::InstantPopup);

        auto transferFuncMenu = new QMenu(transferFuncButton);
        auto transferFuncGroup = new QActionGroup(transferFuncMenu);

        // WARNING: Must match ImageView transfer func options
        const auto transferFuncOptions = QStringList{"Linear", "Log", "Sqrt", "Square"};
        for (const auto& opt : transferFuncOptions)
        {
            auto pOptionAction = new QAction(opt, transferFuncGroup);
            pOptionAction->setCheckable(true);

            transferFuncGroup->addAction(pOptionAction);
            transferFuncMenu->addAction(pOptionAction);
        }

        connect(transferFuncMenu, &QMenu::triggered, [=,this](QAction* pAction){
            if (pAction->text() == "Linear") { m_imageViewParams.transferFunction = TransferFunction::Linear; }
            if (pAction->text() == "Log") { m_imageViewParams.transferFunction = TransferFunction::Log; }
            if (pAction->text() == "Sqrt") { m_imageViewParams.transferFunction = TransferFunction::Sqrt; }
            if (pAction->text() == "Square") { m_imageViewParams.transferFunction = TransferFunction::Square; }

            transferFuncButton->setText(pAction->text());

            RebuildImageView();
        });

        transferFuncButton->setMenu(transferFuncMenu);

    // Color Map action
        const auto defaultColorMapValue("Gray"); // WARNING: Must match default ImageView::Params color map value

        auto pColorMapButton = new QToolButton();
        pColorMapButton->setText(defaultColorMapValue);
        pColorMapButton->setPopupMode(QToolButton::InstantPopup);

        auto pColorMapMenu = new QMenu(pColorMapButton);
        auto pColorMapActionGroup = new QActionGroup(pColorMapMenu);

        AddMenuItems(pColorMapMenu, pColorMapActionGroup, defaultColorMapValue,
                     {"Gray", "Fire", "Ocean", "Ice"});

        AddMenuItems(pColorMapMenu->addMenu("CET Linear"), pColorMapActionGroup, defaultColorMapValue,
                     {"L01", "L02", "L03", "L04", "L05", "L06", "L07", "L08", "L09", "L10",
                      "L11", "L12", "L13", "L14", "L15", "L16", "L17", "L18", "L19", "L20"});

        AddMenuItems(pColorMapMenu->addMenu("CET Diverging"), pColorMapActionGroup, defaultColorMapValue,
                     {"D01", "D01A", "D02", "D03", "D04", "D06", "D07", "D08", "D09", "D10", "D13", "R3"});

        AddMenuItems(pColorMapMenu->addMenu("CET Rainbow"), pColorMapActionGroup, defaultColorMapValue,
                     {"R1", "R2", "R4"});

        connect(pColorMapMenu, &QMenu::triggered, [=,this](QAction* pAction){
            if (pAction->text() == "Gray")  { m_imageViewParams.colorMap = ColorMap::Gray; }
            if (pAction->text() == "Fire")  { m_imageViewParams.colorMap = ColorMap::Fire; }
            if (pAction->text() == "Ocean") { m_imageViewParams.colorMap = ColorMap::Ocean; }
            if (pAction->text() == "Ice")   { m_imageViewParams.colorMap = ColorMap::Ice; }

            if (pAction->text() == "L01")   { m_imageViewParams.colorMap = ColorMap::CET_L01; }
            if (pAction->text() == "L02")   { m_imageViewParams.colorMap = ColorMap::CET_L02; }
            if (pAction->text() == "L03")   { m_imageViewParams.colorMap = ColorMap::CET_L03; }
            if (pAction->text() == "L04")   { m_imageViewParams.colorMap = ColorMap::CET_L04; }
            if (pAction->text() == "L05")   { m_imageViewParams.colorMap = ColorMap::CET_L05; }
            if (pAction->text() == "L06")   { m_imageViewParams.colorMap = ColorMap::CET_L06; }
            if (pAction->text() == "L07")   { m_imageViewParams.colorMap = ColorMap::CET_L07; }
            if (pAction->text() == "L08")   { m_imageViewParams.colorMap = ColorMap::CET_L08; }
            if (pAction->text() == "L09")   { m_imageViewParams.colorMap = ColorMap::CET_L09; }
            if (pAction->text() == "L10")   { m_imageViewParams.colorMap = ColorMap::CET_L10; }
            if (pAction->text() == "L11")   { m_imageViewParams.colorMap = ColorMap::CET_L11; }
            if (pAction->text() == "L12")   { m_imageViewParams.colorMap = ColorMap::CET_L12; }
            if (pAction->text() == "L13")   { m_imageViewParams.colorMap = ColorMap::CET_L13; }
            if (pAction->text() == "L14")   { m_imageViewParams.colorMap = ColorMap::CET_L14; }
            if (pAction->text() == "L15")   { m_imageViewParams.colorMap = ColorMap::CET_L15; }
            if (pAction->text() == "L16")   { m_imageViewParams.colorMap = ColorMap::CET_L16; }
            if (pAction->text() == "L17")   { m_imageViewParams.colorMap = ColorMap::CET_L17; }
            if (pAction->text() == "L18")   { m_imageViewParams.colorMap = ColorMap::CET_L18; }
            if (pAction->text() == "L19")   { m_imageViewParams.colorMap = ColorMap::CET_L19; }
            if (pAction->text() == "L20")   { m_imageViewParams.colorMap = ColorMap::CET_L20; }

            if (pAction->text() == "D01")   { m_imageViewParams.colorMap = ColorMap::CET_D01; }
            if (pAction->text() == "D01A")   { m_imageViewParams.colorMap = ColorMap::CET_D01A; }
            if (pAction->text() == "D02")   { m_imageViewParams.colorMap = ColorMap::CET_D02; }
            if (pAction->text() == "D03")   { m_imageViewParams.colorMap = ColorMap::CET_D03; }
            if (pAction->text() == "D04")   { m_imageViewParams.colorMap = ColorMap::CET_D04; }
            if (pAction->text() == "D06")   { m_imageViewParams.colorMap = ColorMap::CET_D06; }
            if (pAction->text() == "D07")   { m_imageViewParams.colorMap = ColorMap::CET_D07; }
            if (pAction->text() == "D08")   { m_imageViewParams.colorMap = ColorMap::CET_D08; }
            if (pAction->text() == "D09")   { m_imageViewParams.colorMap = ColorMap::CET_D09; }
            if (pAction->text() == "D10")   { m_imageViewParams.colorMap = ColorMap::CET_D10; }
            if (pAction->text() == "D13")   { m_imageViewParams.colorMap = ColorMap::CET_D13; }
            if (pAction->text() == "R3")   { m_imageViewParams.colorMap = ColorMap::CET_R3; }

            if (pAction->text() == "R1")   { m_imageViewParams.colorMap = ColorMap::CET_R1; }
            if (pAction->text() == "R2")   { m_imageViewParams.colorMap = ColorMap::CET_R2; }
            if (pAction->text() == "R4")   { m_imageViewParams.colorMap = ColorMap::CET_R4; }

            pColorMapButton->setText(pAction->text());

            RebuildImageView();
        });

        pColorMapButton->setMenu(pColorMapMenu);

    // Build toolbar
    pParamsToolbar->addWidget(pInvertButton);
    pParamsToolbar->addSeparator();
    pParamsToolbar->addWidget(transferFuncButton);
    pParamsToolbar->addSeparator();
    pParamsToolbar->addWidget(pColorMapButton);

    //
    // Axis Selection Toolbars
    //
    std::vector<QToolBar*> pSelectionToolbars;

    const auto naxisns = dynamic_cast<const ImageData*>(m_imageData.get())->GetParams().naxisns;

    // Append axis selection toolbars for every axis past the first two
    for (std::size_t x = 2; x < naxisns.size(); ++x)
    {
        const auto& naxisn = naxisns.at(x);

        // Don't provide a selection widget for an axis unless it has a size > 1; nothing to select otherwise
        if (naxisn <= 1)
        {
            continue;
        }

        AxisWidget* pAxisWidget{nullptr};

        // Provide a special slider widget specifically for axis 3
        if (x == 2)
        {
            pAxisWidget = new AxisSliderWidget(static_cast<unsigned int>(x + 1), naxisn);
        }
        // For all other axes after axis 3, provide a simple spin widget
        else
        {
            pAxisWidget = new AxisSpinWidget(static_cast<unsigned int>(x + 1), naxisn);
        }

        connect(pAxisWidget, &AxisWidget::Signal_ValueChanged, [=, this](int val) {
            m_imageViewSelection.axisSelection.insert_or_assign(static_cast<unsigned int>(x + 1), val);
            RebuildImageView();
        });

        auto pSelectionToolbar = new QToolBar();
        pSelectionToolbar->addWidget(pAxisWidget);

        pSelectionToolbars.push_back(pSelectionToolbar);
    }

    //
    // Image View
    //
    m_pImageViewWidget = new ImageViewWidget();

    //
    // Error View
    //
    m_pErrorWidget = new QLabel(tr("Failed to render image"));
    m_pErrorWidget->hide();

    //
    // Layout
    //
    auto pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(pParamsToolbar);
    for (const auto& pSelectionToolbar : pSelectionToolbars)
    {
        pMainLayout->addWidget(pSelectionToolbar);
    }
    pMainLayout->addWidget(m_pImageViewWidget, 1);
    pMainLayout->addWidget(m_pErrorWidget);

    //
    // Build the initial image view from our data
    //
    RebuildImageView();
}

void ImageWidget::RebuildImageView()
{
    const bool isImageData = m_imageData->GetType() == Data::Type::Image;
    assert(isImageData); if (!isImageData) { return; }

    auto imageView = ImageView::From(m_imageViewSelection, m_imageViewParams, dynamic_cast<const ImageData*>(m_imageData.get()));
    if (imageView)
    {
        m_pImageViewWidget->SetImageView(*imageView);
    }

    m_pErrorWidget->setVisible(!imageView.has_value());
}

}
