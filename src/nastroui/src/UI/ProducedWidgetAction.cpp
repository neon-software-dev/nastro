/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ProducedWidgetAction.h"

namespace Nastro
{

ProducedWidgetAction::ProducedWidgetAction(QObject* pOwner, WidgetProducer producer)
    : QWidgetAction(pOwner)
    , m_pOwner(pOwner)
    , m_producer(std::move(producer))
{

}

QWidget* ProducedWidgetAction::createWidget(QWidget* parent)
{
    return std::invoke(m_producer, m_pOwner, parent);
}

}
