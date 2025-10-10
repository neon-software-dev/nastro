/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NASTROUI_SRC_SETTINGS_H
#define NASTROUI_SRC_SETTINGS_H

#include <QColor>
#include <QSettings>

namespace Nastro
{
    static constexpr auto SETTINGS_RENDERING_BLANK_COLOR = "rendering/blankColor";
    static constexpr auto DEFAULT_BLANK_COLOR = Qt::black;

    template <typename T>
    [[nodiscard]] T SafeGetSetting(const QSettings& settings, const QString& key, const T& defaultValue)
    {
        if (!settings.contains(key))
        {
            return defaultValue;
        }

        return settings.value(key).value<T>();
    }
}

#endif //NASTROUI_SRC_SETTINGS_H
