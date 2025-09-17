/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_LOADHDUDATAWORKER_H
#define SRC_UTIL_LOADHDUDATAWORKER_H

#include "Worker.h"
#include "Common.h"

#include <expected>
#include <filesystem>

namespace NFITS
{
    class Data;
}

namespace Nastro
{
    class LoadHDUDataWorker : public Worker
    {
        Q_OBJECT

        public:

            LoadHDUDataWorker(std::vector<FileHDU> hdus);
            ~LoadHDUDataWorker() override;

            void DoWork() override;

            [[nodiscard]] const std::vector<FileHDU>& GetHDUs() const noexcept { return m_hdus; }
            [[nodiscard]] std::optional<std::vector<std::unique_ptr<NFITS::Data>>>& GetResult() noexcept { return m_result; }

        private:

            [[nodiscard]] std::expected<std::unique_ptr<NFITS::Data>, bool> LoadHDU(const FileHDU& hdu);

        private:

            std::vector<FileHDU> m_hdus;

            std::optional<std::vector<std::unique_ptr<NFITS::Data>>> m_result;
    };
}

#endif //SRC_UTIL_LOADHDUDATAWORKER_H
