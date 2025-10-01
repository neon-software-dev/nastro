/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_BINTABLEDATA_H
#define NFITS_INCLUDE_NFITS_DATA_BINTABLEDATA_H

#include "Data.h"

#include "../Error.h"

#include <expected>
#include <memory>
#include <vector>
#include <optional>
#include <span>

namespace NFITS
{
    class FITSFile;
    struct HDU;

    using BinTableRowBytes = std::vector<std::byte>;
    using BinTableHeapBytes = std::vector<std::byte>;

    enum class BinFieldType
    {
        Logical,
        Bit,
        UnsignedByte,
        Integer16Bit,
        Integer32Bit,
        Integer64Bit,
        Character,
        FloatSinglePrecision,
        FloatDoublePrecision,
        ComplexSinglePrecision,
        ComplexDoublePrecision,
        Array32Bit,
        Array64Bit
    };

    struct BinFieldForm
    {
        uintmax_t repeatCount{1};
        BinFieldType type{};
        std::optional<BinFieldType> arrayType;
        std::optional<uintmax_t> arrayMaxCount;
    };

    struct BinField
    {
        // Name of the field
        std::optional<std::string> name;

        // Form the field takes
        BinFieldForm form;
    };

    class BinTableData : public Data
    {
        public:

            BinTableData(std::vector<BinField> fields,
                         std::vector<BinTableRowBytes> rowBytes,
                         BinTableHeapBytes heapBytes);

            [[nodiscard]] const std::vector<BinField>& GetFields() const noexcept { return m_fields; }
            [[nodiscard]] std::optional<std::pair<uintmax_t, BinField>> GetFieldByName(const std::string& fieldName) const;
            /** @return The byte size of the specified bintable field */
            [[nodiscard]] uintmax_t GetFieldByteSize(const BinField& field) const;
            /**  @return The byte size of a single variable-length array element associated with the specified bintable field */
            [[nodiscard]] uintmax_t GetVarArrayElementByteSize(const BinField& field) const;

            [[nodiscard]] std::size_t GetNumRows() const noexcept { return m_rowBytes.size(); }
            [[nodiscard]] std::optional<std::span<const std::byte>> GetRowBytes(uintmax_t rowIndex) const;
            [[nodiscard]] std::optional<std::span<const std::byte>> GetRowFieldBytes(uintmax_t rowIndex, uintmax_t fieldIndex) const;

            [[nodiscard]] const BinTableHeapBytes& GetHeapBytes() const noexcept { return m_heapBytes; }

            //
            // Data
            //
            [[nodiscard]] Type GetType() const override { return Type::BinTable; };

        private:

            std::vector<BinField> m_fields;
            std::vector<BinTableRowBytes> m_rowBytes;
            BinTableHeapBytes m_heapBytes;
    };

    [[nodiscard]] NFITS_PUBLIC std::expected<std::unique_ptr<BinTableData>, Error>
        LoadBinTableDataFromFileBlocking(const FITSFile* pFile, const HDU* pHDU);
}

#endif //NFITS_INCLUDE_NFITS_DATA_BINTABLEDATA_H
