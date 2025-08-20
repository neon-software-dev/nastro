/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_MDIHDUWIDGET_H
#define SRC_UI_MDIHDUWIDGET_H

#include <QWidget>

#include <filesystem>

namespace Nastro
{
    class MdiHDUWidget : public QWidget
    {
        Q_OBJECT

        public:

            enum class Type
            {
                Image
            };

        public:

            MdiHDUWidget(std::filesystem::path filePath, uintmax_t hduIndex, QWidget* pParent = nullptr);
            ~MdiHDUWidget() override = default;

            [[nodiscard]] virtual Type GetType() const = 0;

            [[nodiscard]] std::filesystem::path GetFilePath() const noexcept { return m_filePath; }
            [[nodiscard]] uintmax_t GetHDUIndex() const noexcept { return m_hduIndex; }

        protected:

            std::filesystem::path m_filePath;
            uintmax_t m_hduIndex;
    };
}

#endif //SRC_UI_MDIHDUWIDGET_H
