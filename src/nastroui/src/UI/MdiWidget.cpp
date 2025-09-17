/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "MdiWidget.h"

namespace Nastro
{

MdiWidget::MdiWidget(std::optional<FileHDU> associatedHDU, QWidget* pParent)
    : QWidget(pParent)
    , m_associatedHDU(std::move(associatedHDU))
{

}

}
