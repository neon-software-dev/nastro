/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_LOADHDUDATAWORKER_H
#define SRC_UTIL_LOADHDUDATAWORKER_H

#include "Worker.h"

#include <expected>
#include <filesystem>

namespace Nastro
{
    class Data;

    class LoadHDUDataWorker : public Worker
    {
        Q_OBJECT

        public:

            LoadHDUDataWorker(std::filesystem::path filePath, const std::size_t& hduIndex);
            ~LoadHDUDataWorker() override;

            void DoWork() override;

            [[nodiscard]] std::filesystem::path GetFilePath() const noexcept { return m_filePath; }
            [[nodiscard]] std::size_t GetHDUIndex() const noexcept { return m_hduIndex; }
            [[nodiscard]] std::optional<std::unique_ptr<Data>>& GetResult() { return m_result; }

        private:

            std::filesystem::path m_filePath;
            std::size_t m_hduIndex;

            std::optional<std::unique_ptr<Data>> m_result;
    };
}

#endif //SRC_UTIL_LOADHDUDATAWORKER_H
