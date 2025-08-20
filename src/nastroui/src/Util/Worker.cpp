/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "Worker.h"

namespace Nastro
{

void Worker::Cancel()
{
    m_isCancelled = true;
}

}
