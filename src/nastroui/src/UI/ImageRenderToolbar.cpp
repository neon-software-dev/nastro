/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageRenderToolbar.h"

#include "../Settings.h"

#include <QToolButton>
#include <QLabel>
#include <QComboBox>
#include <QMenu>
#include <QActionGroup>

#include <unordered_map>

namespace Nastro
{

static QAction* AddCheckableMenuItem(QMenu* pMenu, QActionGroup* pActionGroup, const QString& item)
{
    auto pAction = pMenu->addAction(item);
    pAction->setCheckable(true);

    pActionGroup->addAction(pAction);

    return pAction;
}

ImageRenderToolbar::ImageRenderToolbar(QWidget* pParent)
    : QToolBar(pParent)
{
    InitUI();
}

void ImageRenderToolbar::InitUI()
{
    addAction(new ProducedWidgetAction(this, GetInvertWidgetProducer()));
    addSeparator();
    addAction(new ProducedWidgetAction(this, GetTransferFunctionWidgetProducer()));
    addSeparator();
    addAction(new ProducedWidgetAction(this, GetColorMapWidgetProducer()));
    addSeparator();
    addAction(new ProducedWidgetAction(this, GetScalingModeWidgetProducer()));
    addSeparator();
    addAction(new ProducedWidgetAction(this, GetScalingRangeWidgetProducer()));
}

void ImageRenderToolbar::SetScalingRange(NFITS::ScalingRange scalingRange)
{
    UpdateImageRenderParam(m_baseRenderParams.scalingRange, scalingRange);
}

void ImageRenderToolbar::SetCustomScalingRangeMin(const std::optional<double>& value)
{
    SetCustomScalingRangeVal(m_baseRenderParams.customScalingRangeMin, value);
}

void ImageRenderToolbar::SetCustomScalingRangeMax(const std::optional<double>& value)
{
    SetCustomScalingRangeVal(m_baseRenderParams.customScalingRangeMax, value);
}

void ImageRenderToolbar::SetCustomScalingRangeVal(std::optional<double>& target, const std::optional<double>& value)
{
    // Special case if either is std::nullopt and the other isn't
    if ((target && !value) || (!target && value))
    {
        target = value;
        emit Signal_OnImageRenderParamsChanged(m_baseRenderParams);
        return;
    }

    // Otherwise, if both have values, epsilon compare the values for a change
    const auto valueChanged = std::abs(*target - *value) > std::numeric_limits<double>::epsilon();
    if (!valueChanged)
    {
        return;
    }

    target = value;
    emit Signal_OnImageRenderParamsChanged(m_baseRenderParams);
}

WidgetProducer ImageRenderToolbar::GetInvertWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent){
        auto pImageRenderToolbar = dynamic_cast<ImageRenderToolbar*>(pOwner);

        auto pInvertButton = new QToolButton(pParent);
        pInvertButton->setText(tr("Invert"));
        pInvertButton->setToolTip(tr("Invert colors"));
        pInvertButton->setCheckable(true);

        // Respond to the widget being set to a new value
        connect(pInvertButton, &QToolButton::toggled, pInvertButton, [=,this](bool checked){
            UpdateImageRenderParam(m_baseRenderParams.invertColors, checked);
        });

        // Sync the widget to the latest parameter state
        connect(pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged,
                pInvertButton, [=](const NFITS::ImageRenderParams& imageRenderParams){
            pInvertButton->setChecked(imageRenderParams.invertColors);
        });

        pInvertButton->setChecked(m_baseRenderParams.invertColors);

        return pInvertButton;
    };
}

static const auto transferFuncStrToTransferFunc = std::unordered_map<QString, NFITS::TransferFunction>{
    {"Linear",  NFITS::TransferFunction::Linear},
    {"Log",     NFITS::TransferFunction::Log},
    {"Sqrt",    NFITS::TransferFunction::Sqrt},
    {"Square",  NFITS::TransferFunction::Square},
    {"Asinh",   NFITS::TransferFunction::Asinh}
};

WidgetProducer ImageRenderToolbar::GetTransferFunctionWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent){
        auto pImageRenderToolbar = dynamic_cast<ImageRenderToolbar*>(pOwner);

        auto pTransferFuncButton = new QToolButton(pParent);
        pTransferFuncButton->setToolTip(tr("Transfer function"));
        pTransferFuncButton->setPopupMode(QToolButton::InstantPopup);

        auto transferFuncMenu = new QMenu(pTransferFuncButton);
        auto transferFuncGroup = new QActionGroup(transferFuncMenu);

        auto transferFuncToAction = std::unordered_map<NFITS::TransferFunction, QAction*>();

        const auto rootTransferFuncOptions = QStringList{"Linear", "Log", "Sqrt", "Square", "Asinh"};

        for (const auto& transferFuncStr : rootTransferFuncOptions)
        {
            const auto pAction = AddCheckableMenuItem(transferFuncMenu, transferFuncGroup, transferFuncStr);

            transferFuncToAction.insert({transferFuncStrToTransferFunc.at(transferFuncStr), pAction});
        }

        pTransferFuncButton->setMenu(transferFuncMenu);

        // Respond to the widget being set to a new value
        connect(transferFuncMenu, &QMenu::triggered, pTransferFuncButton, [=,this](QAction* pAction){
            pTransferFuncButton->setText(pAction->text());

            UpdateImageRenderParam(m_baseRenderParams.transferFunction, transferFuncStrToTransferFunc.at(pAction->text()));
        });

        // Sync the widget to the latest parameter state
        connect(pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged,
                pTransferFuncButton, [=](const NFITS::ImageRenderParams& params){
            transferFuncToAction.at(params.transferFunction)->trigger();
        });

        // Display the initial parameter state
        transferFuncToAction.at(m_baseRenderParams.transferFunction)->trigger();

        return pTransferFuncButton;
    };
}

static const auto colorMapStrToColorMap = std::unordered_map<QString, NFITS::ColorMap>{
    // Base
    {"Gray",    NFITS::ColorMap::CET_L01},
    {"Fire",    NFITS::ColorMap::Fire},
    {"Ocean",   NFITS::ColorMap::Ocean},
    {"Ice",     NFITS::ColorMap::Ice},
    // Linear
    {"L01",     NFITS::ColorMap::CET_L01},
    {"L02",     NFITS::ColorMap::CET_L02},
    {"L03",     NFITS::ColorMap::CET_L03},
    {"L04",     NFITS::ColorMap::CET_L04},
    {"L05",     NFITS::ColorMap::CET_L05},
    {"L06",     NFITS::ColorMap::CET_L06},
    {"L07",     NFITS::ColorMap::CET_L07},
    {"L08",     NFITS::ColorMap::CET_L08},
    {"L09",     NFITS::ColorMap::CET_L09},
    {"L10",     NFITS::ColorMap::CET_L10},
    {"L11",     NFITS::ColorMap::CET_L11},
    {"L12",     NFITS::ColorMap::CET_L12},
    {"L13",     NFITS::ColorMap::CET_L13},
    {"L14",     NFITS::ColorMap::CET_L14},
    {"L15",     NFITS::ColorMap::CET_L15},
    {"L16",     NFITS::ColorMap::CET_L16},
    {"L17",     NFITS::ColorMap::CET_L17},
    {"L18",     NFITS::ColorMap::CET_L18},
    {"L19",     NFITS::ColorMap::CET_L19},
    {"L20",     NFITS::ColorMap::CET_L20},
    // Diverging
    {"D01",     NFITS::ColorMap::CET_D01},
    {"D01A",    NFITS::ColorMap::CET_D01A},
    {"D02",     NFITS::ColorMap::CET_D02},
    {"D03",     NFITS::ColorMap::CET_D03},
    {"D04",     NFITS::ColorMap::CET_D04},
    {"D06",     NFITS::ColorMap::CET_D06},
    {"D07",     NFITS::ColorMap::CET_D07},
    {"D08",     NFITS::ColorMap::CET_D08},
    {"D09",     NFITS::ColorMap::CET_D09},
    {"D10",     NFITS::ColorMap::CET_D10},
    {"D13",     NFITS::ColorMap::CET_D13},
    {"R3",      NFITS::ColorMap::CET_R3},
    // Rainbow
    {"R1",      NFITS::ColorMap::CET_R1},
    {"R2",      NFITS::ColorMap::CET_R2},
    {"R4",      NFITS::ColorMap::CET_R4},
};

WidgetProducer ImageRenderToolbar::GetColorMapWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent) {
        auto pImageRenderToolbar = dynamic_cast<ImageRenderToolbar*>(pOwner);

        auto pColorMapButton = new QToolButton(pParent);
        pColorMapButton->setToolTip(tr("Color map"));
        pColorMapButton->setPopupMode(QToolButton::InstantPopup);

        auto pColorMapMenu = new QMenu(pColorMapButton);
        auto pColorMapGroup = new QActionGroup(pColorMapMenu);

        auto colorMapToAction = std::unordered_map<NFITS::ColorMap, QAction*>();

        // Root Color Maps
        const auto rootColorMapOptions = QStringList{"Gray", "Fire", "Ocean", "Ice"};

        for (const auto& colorMapOption : rootColorMapOptions)
        {
            const auto pAction = AddCheckableMenuItem(pColorMapMenu, pColorMapGroup, colorMapOption);

            colorMapToAction.insert({colorMapStrToColorMap.at(colorMapOption), pAction});
        }

        // CET Linear Color Maps
        const auto cetLinearMenu = pColorMapMenu->addMenu("CET Linear");

        const auto cetLinearColorMapOptions = QStringList{"L01", "L02", "L03", "L04", "L05", "L06", "L07", "L08", "L09", "L10",
                                                          "L11", "L12", "L13", "L14", "L15", "L16", "L17", "L18", "L19", "L20"};

        for (const auto& colorMapOption : cetLinearColorMapOptions)
        {
            const auto pAction = AddCheckableMenuItem(cetLinearMenu, pColorMapGroup, colorMapOption);

            colorMapToAction.insert({colorMapStrToColorMap.at(colorMapOption), pAction});
        }

        // CET Diverging Color Maps
        const auto cetDivergingMenu = pColorMapMenu->addMenu("CET Diverging");

        const auto cetDivergingColorMapOptions = QStringList{"D01", "D01A", "D02", "D03", "D04", "D06", "D07", "D08", "D09", "D10", "D13", "R3"};

        for (const auto& colorMapOption : cetDivergingColorMapOptions)
        {
            const auto pAction = AddCheckableMenuItem(cetDivergingMenu, pColorMapGroup, colorMapOption);

            colorMapToAction.insert({colorMapStrToColorMap.at(colorMapOption), pAction});
        }

        // CET Rainbow Color Maps
        const auto cetRainbowMenu = pColorMapMenu->addMenu("CET Rainbow");

        const auto cetRainbowColorMapOptions = QStringList{"R1", "R2", "R4"};

        for (const auto& colorMapOption : cetRainbowColorMapOptions)
        {
            const auto pAction = AddCheckableMenuItem(cetRainbowMenu, pColorMapGroup, colorMapOption);

            colorMapToAction.insert({colorMapStrToColorMap.at(colorMapOption), pAction});
        }

        pColorMapButton->setMenu(pColorMapMenu);

        // Respond to the widget being set to a new value
        connect(pColorMapMenu, &QMenu::triggered, pColorMapButton, [=,this](QAction* pAction){
            pColorMapButton->setText(pAction->text());

            UpdateImageRenderParam(m_baseRenderParams.colorMap, colorMapStrToColorMap.at(pAction->text()));
        });

        // Sync the widget to the latest parameter state
        connect(pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged,
            pColorMapButton, [=](const NFITS::ImageRenderParams& params){
            colorMapToAction.at(params.colorMap)->trigger();
        });

        // Display the initial parameter state
        colorMapToAction.at(m_baseRenderParams.colorMap)->trigger();

        return pColorMapButton;
    };
}

static const auto scalingModeStrToScalingMode = std::unordered_map<QString, NFITS::ScalingMode>{
    {"Image Scaled",  NFITS::ScalingMode::PerImage},
    {"Series Scaled",   NFITS::ScalingMode::PerCube}
};

WidgetProducer ImageRenderToolbar::GetScalingModeWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent){
        auto pImageRenderToolbar = dynamic_cast<ImageRenderToolbar*>(pOwner);

        auto pScalingModeButton = new QToolButton(pParent);
        pScalingModeButton->setToolTip(tr("Scaling mode"));
        pScalingModeButton->setPopupMode(QToolButton::InstantPopup);

        auto scalingModeMenu = new QMenu(pScalingModeButton);
        auto scalingModeGroup = new QActionGroup(scalingModeMenu);

        auto scalingModeToAction = std::unordered_map<NFITS::ScalingMode, QAction*>();

        const auto rootScalingModeOptions = QStringList{"Image Scaled", "Series Scaled"};

        for (const auto& scalingModeStr : rootScalingModeOptions)
        {
            const auto pAction = AddCheckableMenuItem(scalingModeMenu, scalingModeGroup, scalingModeStr);

            scalingModeToAction.insert({scalingModeStrToScalingMode.at(scalingModeStr), pAction});
        }

        pScalingModeButton->setMenu(scalingModeMenu);

        // Respond to the widget being set to a new value
        connect(scalingModeMenu, &QMenu::triggered, pScalingModeButton, [=,this](QAction* pAction){
            pScalingModeButton->setText(pAction->text());

            UpdateImageRenderParam(m_baseRenderParams.scalingMode, scalingModeStrToScalingMode.at(pAction->text()));
        });

        // Sync the widget to the latest parameter state
        connect(pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged,
            pScalingModeButton, [=](const NFITS::ImageRenderParams& params){
            scalingModeToAction.at(params.scalingMode)->trigger();
        });

        // Display the initial parameter state
        scalingModeToAction.at(m_baseRenderParams.scalingMode)->trigger();

        return pScalingModeButton;
    };
}

static const auto scalingRangeStrToScalingRange = std::unordered_map<QString, NFITS::ScalingRange>{
    {"Full Range",  NFITS::ScalingRange::Full},
    {"p99",         NFITS::ScalingRange::p99},
    {"p95",         NFITS::ScalingRange::p95},
    {"Custom",      NFITS::ScalingRange::Custom},
};

WidgetProducer ImageRenderToolbar::GetScalingRangeWidgetProducer()
{
    return [this](QObject* pOwner, QWidget* pParent){
        auto pImageRenderToolbar = dynamic_cast<ImageRenderToolbar*>(pOwner);

        auto pScalingRangeButton = new QToolButton(pParent);
        pScalingRangeButton->setToolTip(tr("Scaling range"));
        pScalingRangeButton->setPopupMode(QToolButton::InstantPopup);

        auto scalingRangeMenu = new QMenu(pScalingRangeButton);
        auto scalingRangeGroup = new QActionGroup(scalingRangeMenu);

        auto scalingRangeToAction = std::unordered_map<NFITS::ScalingRange, QAction*>();

        const auto rootScalingRangeOptions = QStringList{"Full Range", "p99", "p95", "Custom"};

        for (const auto& scalingRangeStr : rootScalingRangeOptions)
        {
            const auto pAction = AddCheckableMenuItem(scalingRangeMenu, scalingRangeGroup, scalingRangeStr);

            scalingRangeToAction.insert({scalingRangeStrToScalingRange.at(scalingRangeStr), pAction});
        }

        pScalingRangeButton->setMenu(scalingRangeMenu);

        // Respond to the widget being set to a new value
        connect(scalingRangeMenu, &QMenu::triggered, pScalingRangeButton, [=,this](QAction* pAction){
            pScalingRangeButton->setText(pAction->text());

            UpdateImageRenderParam(m_baseRenderParams.scalingRange, scalingRangeStrToScalingRange.at(pAction->text()));
        });

        // Sync the widget to the latest parameter state
        connect(pImageRenderToolbar, &ImageRenderToolbar::Signal_OnImageRenderParamsChanged,
            pScalingRangeButton, [=](const NFITS::ImageRenderParams& params){
            scalingRangeToAction.at(params.scalingRange)->trigger();
        });

        // Display the initial parameter state
        scalingRangeToAction.at(m_baseRenderParams.scalingRange)->trigger();

        return pScalingRangeButton;
    };
}

NFITS::ImageRenderParams ImageRenderToolbar::GetImageRenderParams() const
{
    // Base parameters that come from this widget
    auto params = m_baseRenderParams;

    // Other parameters that are external to this widget
    const auto blankColor = SafeGetSetting<QColor>(QSettings(), SETTINGS_RENDERING_BLANK_COLOR, DEFAULT_BLANK_COLOR);
    params.blankColor[0] = static_cast<unsigned char>(blankColor.red());
    params.blankColor[1] = static_cast<unsigned char>(blankColor.green());
    params.blankColor[2] = static_cast<unsigned char>(blankColor.blue());

    return params;
}

}
