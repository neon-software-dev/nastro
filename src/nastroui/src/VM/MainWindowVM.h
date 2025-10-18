/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_VM_MAINWINDOWVM_H
#define SRC_VM_MAINWINDOWVM_H

#include "../Util/Common.h"

#include <NFITS/HDU.h>

#include <QObject>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <optional>

namespace Nastro
{
    class Worker;

    class MainWindowVM : public QObject
    {
        Q_OBJECT

        public:

            explicit MainWindowVM(QWidget* pParent);

            [[nodiscard]] std::optional<FileHDU> GetActivatedHDU() const noexcept { return m_activatedHDU; }
            [[nodiscard]] const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& GetImportedFiles() const noexcept { return m_importedFiles; }
            [[nodiscard]] std::optional<NFITS::HDU> GetImportedFileHDU(const std::filesystem::path& filePath, uintmax_t hduIndex) const;

            void OnImportDirectory(const std::filesystem::path& directoryPath);
            void OnImportFiles(const std::vector<std::filesystem::path>& filePaths);
            void OnHDUActivated(const std::optional<FileHDU>& activatedHDU);
            void OnPixelHovered(const std::optional<PixelDetails>& pixelDetails);

        signals:

            void Signal_FilesImported(const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& files);
            void Signal_OnActivatedHDUChanged(const std::optional<FileHDU>& activatedHDU);
            void Signal_OnPixelHoveredChanged(const std::optional<PixelDetails>& pixelDetails);

        private slots:

            void Slot_ImportFiles_WorkFinished(Nastro::Worker* pWorker);

        private:

            QWidget* m_pParent;

            std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>> m_importedFiles;
            std::optional<FileHDU> m_activatedHDU;
            std::optional<PixelDetails> m_hoveredPixelDetails;
    };
}

#endif //SRC_VM_MAINWINDOWVM_H
