/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_IMPORTFILESWORKER_H
#define SRC_UTIL_IMPORTFILESWORKER_H

#include "Worker.h"

#include <NFITS/HDU.h>

#include <filesystem>
#include <vector>
#include <unordered_map>

namespace Nastro
{
    class ImportFilesWorker : public Worker
    {
        public:

            explicit ImportFilesWorker(std::vector<std::filesystem::path> filePaths);

            void DoWork() override;

            const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& GetResult() const { return m_importedHDUs; }

        private:

            void ImportFile(const std::filesystem::path& filePath);

        private:

            std::vector<std::filesystem::path> m_filePaths;

            std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>> m_importedHDUs;
    };
}

#endif //SRC_UTIL_IMPORTFILESWORKER_H
