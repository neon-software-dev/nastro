/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_MDIHDUWIDGET_H
#define SRC_UI_MDIHDUWIDGET_H

#include "../Util/Common.h"

#include <QWidget>

#include <filesystem>
#include <optional>

namespace Nastro
{
    /**
     * Base class for Widgets displayed as Mdi subwindows within the main Mdi area.
     *
     * Can have an optional associated HDU, such that when the Mdi subwindow is activated,
     * other widgets which depend on the active HDU can update themselves for the now-active
     * hdu.
     */
    class MdiWidget : public QWidget
    {
        Q_OBJECT

        public:

            enum class Type
            {
                Image
            };

        public:

            explicit MdiWidget(std::optional<FileHDU> associatedHDU = std::nullopt, QWidget* pParent = nullptr);
            ~MdiWidget() override = default;

            [[nodiscard]] virtual Type GetType() const = 0;

            [[nodiscard]] virtual std::optional<FileHDU> GetAssociatedHDU() const noexcept { return m_associatedHDU; };

        private:

            std::optional<FileHDU> m_associatedHDU;
    };
}

#endif //SRC_UI_MDIHDUWIDGET_H
