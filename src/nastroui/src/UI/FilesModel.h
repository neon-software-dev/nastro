/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UI_FILESMODEL_H
#define SRC_UI_FILESMODEL_H

#include <NFITS/HDU.h>

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include <memory>
#include <optional>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <map>

namespace Nastro
{
    class FilesTreeItem
    {
        public:

            enum class Type
            {
                Root,
                FITS,
                HDU
            };

        public:

            explicit FilesTreeItem(std::optional<const FilesTreeItem*> parent = std::nullopt);
            virtual ~FilesTreeItem() = default;

            [[nodiscard]] virtual Type GetType() const { return Type::Root; };
            [[nodiscard]] virtual QVariant GetDisplayData(int column) const;
            [[nodiscard]] virtual std::size_t GetColumnCount() const;

            [[nodiscard]] std::optional<const FilesTreeItem*> GetParent() const;

            void AddChild(std::unique_ptr<FilesTreeItem> child);
            [[nodiscard]] const FilesTreeItem* GetChild(const std::size_t& index) const;
            [[nodiscard]] std::size_t NumChildren() const;
            [[nodiscard]] std::size_t GetIndexOfChild(const FilesTreeItem* pFileTreeItem) const;
            void RemoveAllChildren();

            [[nodiscard]] std::size_t GetRow() const;

        private:

            std::optional<const FilesTreeItem*> m_parent;
            std::vector<std::unique_ptr<FilesTreeItem>> m_children;
    };

    class FITSFilesTreeItem : public FilesTreeItem
    {
        public:

            FITSFilesTreeItem(std::filesystem::path filePath, const FilesTreeItem* parent);

            [[nodiscard]] Type GetType() const override { return Type::FITS; };
            [[nodiscard]] QVariant GetDisplayData(int column) const override;
            [[nodiscard]] std::size_t GetColumnCount() const override { return 2; };

            [[nodiscard]] std::filesystem::path GetFilePath() const noexcept { return m_filePath; }
            [[nodiscard]] std::optional<std::filesystem::file_time_type> GetLastModifiedTime() const;

        private:

            std::filesystem::path m_filePath;
    };

    class HDUFilesTreeItem : public FilesTreeItem
    {
        public:

            HDUFilesTreeItem(NFITS::HDU hdu, std::size_t hduIndex, const FilesTreeItem* parent);

            [[nodiscard]] Type GetType() const override { return Type::HDU; };
            [[nodiscard]] QVariant GetDisplayData(int column) const override;
            [[nodiscard]] std::size_t GetColumnCount() const override { return 1; };

            [[nodiscard]] std::size_t GetHDUIndex() const noexcept { return m_hduIndex; }
            [[nodiscard]] NFITS::HDU::Type GetHDUType() const noexcept { return m_hdu.type; }

        private:

            NFITS::HDU m_hdu;
            std::size_t m_hduIndex;
    };

    class FilesModel : public QAbstractItemModel
    {
        Q_OBJECT

        public:

            FilesModel();

            void AddFiles(const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& importedFiles);

            //
            // QAbstractItemModel
            //
            [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
            [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;
            [[nodiscard]] QModelIndex index(int row, int column, const QModelIndex &parent) const override;
            [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;
            [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
            [[nodiscard]] int columnCount(const QModelIndex &parent) const override;
            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        private:

            std::unique_ptr<FilesTreeItem> m_rootItem;
    };

    class FilesModelSortProxy : public QSortFilterProxyModel
    {
        protected:

            [[nodiscard]] bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    };
}

#endif //SRC_UI_FILESMODEL_H
