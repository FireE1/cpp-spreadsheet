#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) < (pos.row + 1))
    {
        data_.resize(pos.row + 1);
    }
    if (int(data_[pos.row].size()) < (pos.col + 1))
    {
        data_[pos.row].resize(pos.col + 1);
    }
    if (data_[pos.row][pos.col] == nullptr)
    {
        data_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    data_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) > pos.row && int(data_[pos.row].size()) > pos.col)
    {
        if (data_[pos.row][pos.col] == nullptr || data_[pos.row][pos.col]->GetText() == "")
        {
            return nullptr;
        }
        return data_[pos.row][pos.col].get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) > pos.row && int(data_[pos.row].size()) > pos.col)
    {
        if (data_[pos.row][pos.col] == nullptr || data_[pos.row][pos.col]->GetText() == "")
        {
            return nullptr;
        }
        return data_[pos.row][pos.col].get();
    }
    return nullptr;
}

Cell* Sheet::GetCellRef(Position pos) {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) > pos.row && int(data_[pos.row].size()) > pos.col)
    {
        return data_[pos.row][pos.col].get();
    }
    return nullptr;
}

const Cell* Sheet::GetCellRef(Position pos) const {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) > pos.row && int(data_[pos.row].size()) > pos.col)
    {
        return data_[pos.row][pos.col].get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid())
    {
        throw InvalidPositionException("invalid position");
    }
    if (int(data_.size()) > pos.row && int(data_[pos.row].size()) > pos.col)
    {
        if (data_[pos.row][pos.col] != nullptr)
        {
            data_[pos.row][pos.col]->Clear();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size to_ret;
    for (int row = 0; row < int(data_.size()); ++row)
    {
        for (int col = int(data_[row].size()) - 1; col >= 0 ; --col)
        {
            if (data_[row][col] != nullptr)
            {
                if (!data_[row][col]->GetText().empty())
                {
                    to_ret.rows = std::max(to_ret.rows, row + 1);
                    to_ret.cols = std::max(to_ret.cols, col + 1);
                    break;
                }
            }
        }
    }
    return to_ret;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row)
    {
        for (int col = 0; col < GetPrintableSize().cols; ++col)
        {
            if (col > 0)
            {
                output << '\t';
            }
            if (col < int(data_[row].size()))
            {
                if (data_[row][col] != nullptr)
                {
                    std::visit([&output](const auto& obj){output << obj;}, data_[row][col]->GetValue());
                }
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row < GetPrintableSize().rows; ++row)
    {
        for (int col = 0; col < GetPrintableSize().cols; ++col)
        {
            if (col > 0)
            {
                output << '\t';
            }
            if (col < int(data_[row].size()))
            {
                if (data_[row][col] != nullptr)
                {
                    output << data_[row][col]->GetText();
                }
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}