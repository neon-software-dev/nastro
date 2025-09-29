/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "FilesModel.h"

#include <NFITS/KeywordCommon.h>
#include <NFITS/Data/BinTableImageData.h>

#include <cassert>
#include <ranges>

namespace Nastro
{

static constexpr int COL_FILENAME = 0;
static constexpr int COL_MODIFIED = 1;
static constexpr int NUM_COLUMNS = 2;

FilesTreeItem::FilesTreeItem(std::optional<const FilesTreeItem*> parent)
    : m_parent(parent)
{

}

QVariant FilesTreeItem::GetDisplayData(int) const
{
    return {};
}

std::optional<const FilesTreeItem*> FilesTreeItem::GetParent() const
{
    return m_parent ? std::optional<const FilesTreeItem*>(m_parent) : std::nullopt;
}

void FilesTreeItem::AddChild(std::unique_ptr<FilesTreeItem> child)
{
    m_children.push_back(std::move(child));
}

const FilesTreeItem* FilesTreeItem::GetChild(const std::size_t& index) const
{
    return m_children.at(index).get();
}

std::size_t FilesTreeItem::NumChildren() const
{
    return m_children.size();
}

std::size_t FilesTreeItem::GetIndexOfChild(const FilesTreeItem* pFileTreeItem) const
{
    std::size_t index = 0;

    for (const auto& child : m_children)
    {
        if (child.get() == pFileTreeItem)
        {
            return index;
        }

        index++;
    }

    assert(false);
    return 0;
}

void FilesTreeItem::RemoveAllChildren()
{
    m_children.clear();
}

std::size_t FilesTreeItem::GetRow() const
{
    return m_parent ? (*m_parent)->GetIndexOfChild(this) : 0U;
}

std::size_t FilesTreeItem::GetColumnCount() const
{
    return NUM_COLUMNS;
}

///////////////////

FITSFilesTreeItem::FITSFilesTreeItem(std::filesystem::path filePath, const FilesTreeItem* parent)
    : FilesTreeItem(parent)
    , m_filePath(std::move(filePath))
{

}

QVariant FITSFilesTreeItem::GetDisplayData(int column) const
{
    if (column == COL_FILENAME)
    {
        return QString::fromStdString(m_filePath.filename().string());
    }
    else if (column == COL_MODIFIED)
    {
        const auto lastModifiedTime = GetLastModifiedTime();
        if (!lastModifiedTime)
        {
            return "Unknown";
        }
        auto timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", *lastModifiedTime);

        return QString::fromStdString(timeStr);
    }

    return {};
}

std::optional<std::filesystem::file_time_type> FITSFilesTreeItem::GetLastModifiedTime() const
{
    std::error_code ec{};
    auto lastModifyTime = std::filesystem::last_write_time(m_filePath, ec);
    if (ec)
    {
        return std::nullopt;
    }

    return lastModifyTime;
}

///////////////////

HDUFilesTreeItem::HDUFilesTreeItem(NFITS::HDU hdu, std::size_t hduIndex, const FilesTreeItem* parent)
    : FilesTreeItem(parent)
    , m_hdu(std::move(hdu))
    , m_hduIndex(hduIndex)
{

}

std::string GetHDUTypeString(const NFITS::HDU& hdu, bool hasData)
{
    if (!hasData)
    {
        return "Empty";
    }

    switch (hdu.type)
    {
        case NFITS::HDU::Type::Image: return "Image";
        case NFITS::HDU::Type::Table: return "Table";
        case NFITS::HDU::Type::BinTable:
        {
            if (NFITS::HDUContainsBinTableImage(hdu)) { return "BinTable Image"; } else { return "BinTable"; }
        }
    }

    assert(false);
    return "Error";
}

// {800, 600, 3} -> "800x600x3"
std::string JoinNaxisnsToDimenString(const std::vector<int64_t>& naxisns) {
    std::string str;

    for (std::size_t x = 0; x < naxisns.size(); ++x)
    {
        str += std::format("{}", naxisns.at(x));

        if (x != (naxisns.size() - 1))
        {
            str += "x";
        }
    }

    return str;
}

std::string GetDetailString_Image(const NFITS::HDU& hdu)
{
    const auto naxis = hdu.header.GetFirstKeywordRecord_AsInteger(NFITS::KEYWORD_NAME_NAXIS);
    if (!naxis) { return {}; }

    std::vector<int64_t> naxisns;

    for (int64_t n = 1; n <= *naxis; ++n)
    {
        const auto naxisn = hdu.header.GetFirstKeywordRecord_AsInteger(std::format("{}{}", NFITS::KEYWORD_NAME_NAXIS, n));
        if (!naxis) { return {}; }

        naxisns.push_back(*naxisn);
    }

    return std::format("({})", JoinNaxisnsToDimenString(naxisns));
}

std::string GetDetailString_BinTableImage(const NFITS::HDU& hdu)
{
    const auto znaxis = hdu.header.GetFirstKeywordRecord_AsInteger(NFITS::KEYWORD_NAME_ZNAXIS);
    if (!znaxis) { return {}; }

    std::vector<int64_t> znaxisns;

    for (int64_t n = 1; n <= *znaxis; ++n)
    {
        const auto znaxisn = hdu.header.GetFirstKeywordRecord_AsInteger(std::format("{}{}", NFITS::KEYWORD_NAME_ZNAXIS, n));
        if (!znaxis) { return {}; }

        znaxisns.push_back(*znaxisn);
    }

    return std::format("({})", JoinNaxisnsToDimenString(znaxisns));
}

std::string GetDetailString_BinTable(const NFITS::HDU& hdu)
{
    if (NFITS::HDUContainsBinTableImage(hdu))
    {
        return GetDetailString_BinTableImage(hdu);
    }

    return {};
}

QVariant HDUFilesTreeItem::GetDisplayData(int column) const
{
    if (column == COL_FILENAME)
    {
        const bool hduHasData = m_hdu.GetDataByteSize() > 0U;
        const auto typeString = GetHDUTypeString(m_hdu, hduHasData);

        std::string detailString;

        if (hduHasData)
        {
            switch (m_hdu.type)
            {
                case NFITS::HDU::Type::Image: detailString = GetDetailString_Image(m_hdu); break;
                case NFITS::HDU::Type::BinTable: detailString = GetDetailString_BinTable(m_hdu); break;
                default: /* no-op */ break;
            }
        }

        auto displayString = std::format("HDU {} - {}", m_hduIndex, typeString);
        if (!detailString.empty())
        {
            displayString = std::format("{} {}", displayString, detailString);
        }

        return QString::fromStdString(displayString);
    }
    else if (column == COL_MODIFIED)
    {
        return {}; // no-op
    }

    return {};
}

///////////////////

FilesModel::FilesModel()
    : m_rootItem(std::make_unique<FilesTreeItem>())
{

}

QVariant FilesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return {};
    }

    auto pTreeItem = static_cast<const FilesTreeItem*>(index.internalPointer());
    return pTreeItem->GetDisplayData(index.column());
}

Qt::ItemFlags FilesModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    if (!index.parent().isValid())
    {
        // Don't allow selection of top-level items
        return Qt::ItemIsEnabled;
    }
    else
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
}

QModelIndex FilesModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent) || !m_rootItem)
    {
        return {};
    }

    const FilesTreeItem* pParentItem = parent.isValid() ? static_cast<const FilesTreeItem*>(parent.internalPointer()) : m_rootItem.get();

    auto pChildItem = pParentItem->GetChild(static_cast<std::size_t>(row));
    return createIndex(row, column, pChildItem);
}

QModelIndex FilesModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return {};
    }

    auto pChildItem = static_cast<const FilesTreeItem*>(index.internalPointer());
    auto pParentItem = pChildItem->GetParent();

    return pParentItem != m_rootItem.get() ? createIndex(static_cast<int>((*pParentItem)->GetRow()), 0, *pParentItem) : QModelIndex{};
}

int FilesModel::rowCount(const QModelIndex& parent) const
{
    if (!m_rootItem)
    {
        return 0;
    }

    if (parent.column() > 0)
    {
        return 0;
    }

    const FilesTreeItem* pParentItem = parent.isValid() ? static_cast<const FilesTreeItem*>(parent.internalPointer()) : m_rootItem.get();

    return static_cast<int>(pParentItem->NumChildren());
}

int FilesModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? (int)static_cast<FilesTreeItem*>(parent.internalPointer())->GetColumnCount()
        : (int)m_rootItem->GetColumnCount();
}

void FilesModel::AddFiles(const std::unordered_map<std::filesystem::path, std::vector<NFITS::HDU>>& importedFiles)
{
    for (const auto& fileIt : importedFiles)
    {
        const auto newRowIndex = static_cast<int>(m_rootItem->NumChildren());

        //
        // Insert a row for the file
        //
        beginInsertRows(QModelIndex(), newRowIndex, newRowIndex);

            auto fileItem = std::make_unique<FITSFilesTreeItem>(fileIt.first, m_rootItem.get());
            auto pFileItem = fileItem.get();
            m_rootItem->AddChild(std::move(fileItem));

        endInsertRows();

        // QModelIndex of the above inserted file row
        const auto parentIndex = index(newRowIndex, 0, QModelIndex());

        //
        // Insert child rows for the file's HDUs
        //
        std::size_t hduIndex = 0;

        for (const auto& hdu : fileIt.second)
        {
            beginInsertRows(parentIndex, (int)hduIndex, (int)hduIndex);

                auto hduItem = std::make_unique<HDUFilesTreeItem>(hdu, hduIndex++, pFileItem);
                pFileItem->AddChild(std::move(hduItem));

            endInsertRows();
        }
    }
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant{};
    }

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case COL_FILENAME: return QString(tr("Filename"));
            case COL_MODIFIED: return QString(tr("Modified"));
            default: return QVariant{};
        }
    }

    return QVariant{}; // vertical headers
}

///////////////////

bool FilesModelSortProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const auto leftItem = (const FilesTreeItem*)left.internalPointer();
    const auto rightItem = (const FilesTreeItem*)right.internalPointer();

    switch (leftItem->GetType())
    {
        case FilesTreeItem::Type::Root:
            return true; // no-op
        case FilesTreeItem::Type::FITS:
        {
            const auto leftLastModified = dynamic_cast<const FITSFilesTreeItem*>(leftItem)->GetLastModifiedTime();
            const auto rightLastModified = dynamic_cast<const FITSFilesTreeItem*>(rightItem)->GetLastModifiedTime();

            // Compare time_points
            if (leftLastModified && rightLastModified)
            {
                return *leftLastModified < *rightLastModified;
            }
            // Fallback to comparing display string
            else
            {
                return leftItem->GetDisplayData(left.column()).toString() < rightItem->GetDisplayData(right.column()).toString();
            }
        }
        case FilesTreeItem::Type::HDU:
            return false; // Preserves the model's HDU order, always put HDUs in ascending index order
    }

    assert(false);
    return true;
}

}
