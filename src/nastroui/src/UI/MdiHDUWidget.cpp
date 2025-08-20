/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "MdiHDUWidget.h"

namespace Nastro
{

MdiHDUWidget::MdiHDUWidget(std::filesystem::path filePath, uintmax_t hduIndex, QWidget* pParent)
    : QWidget(pParent)
    , m_filePath(std::move(filePath))
    , m_hduIndex(hduIndex)
{

}

}
