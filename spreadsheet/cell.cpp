#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <set>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;
    if (text.empty())
    {
        impl = std::make_unique<EmptyImpl>();
    }
    else if (text[0] == FORMULA_SIGN && text.size() > 1)
    {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else
    {
        impl = std::make_unique<TextImpl>(std::move(text));
    }
    if (!used_cells_.empty())
    {
        for (Cell* used_cell : used_cells_)
        {
            used_cell->users_.erase(this);
        }
        used_cells_.clear();
    }
    const auto used_cells = impl->GetReferencedCells();
    if (!used_cells.empty())
    {
        std::unordered_set<Cell*> used_set;
        for (const auto pos_of_used : used_cells)
        {
            used_set.insert(sheet_.GetCellRef(pos_of_used));
        }
        if (HasLoop(used_set))
        {
            throw CircularDependencyException("circular dependency");
        }
        used_cells_ = std::move(used_set);
        for (Cell* cell : used_cells_)
        {
            cell->users_.insert(this);
        }
    }
    impl_ = std::move(impl);
    InvalidateCache(true);
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

std::unordered_set<Cell*> Cell::GetRefCells() const {
    return this->used_cells_;
}

bool Cell::IsReferenced() const {
    return !users_.empty();
}

bool LoopFinder(const Cell* start, std::set<Cell*>& buffer, const std::unordered_set<Cell*>& used_cells) {
    for (Cell* cell : used_cells)
    {
        if (*buffer.lower_bound(cell) == cell)
        {
            continue;
        }
        if (cell == start)
        {
            return true;
        }
        buffer.insert(cell);
        if (!cell->GetRefCells().empty())
        {
            return LoopFinder(start, buffer, cell->GetRefCells());
        }
    }
    return false;
}

bool Cell::HasLoop(const std::unordered_set<Cell*>& used_cells) const {
    std::set<Cell*> buffer;
    return LoopFinder(this, buffer, used_cells);
}

void Cell::InvalidateCache(bool flag = false) {
    if (this->users_.empty())
    {
        return;
    }
    if (this->impl_->Cache())
    {
        this->impl_->InvalidateCache();
        for (Cell* cell : users_)
        {
            cell->InvalidateCache();
        }
    }
}