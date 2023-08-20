#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <memory>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    std::unordered_set<Cell*> GetRefCells() const;

    bool IsReferenced() const;
    void ClearUsed();

private:

    bool HasLoop(const std::unordered_set<Cell*>& used_cells) const;
    void InvalidateCache(bool);

    class Impl {
    public:
        
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const {return {};}
        virtual void InvalidateCache() {}
        virtual bool HasCache() {return true;}

    };

    class EmptyImpl : public Impl {
    public:

        Value GetValue() const override;

        std::string GetText() const override;

    };

    class TextImpl : public Impl {
    public:

        explicit TextImpl(const std::string& text) : content(text) {}

        Value GetValue() const override;

        std::string GetText() const override;

    private:

        std::string content;

    };

    class FormulaImpl : public Impl {
    public:

        explicit FormulaImpl(const std::string& text, SheetInterface& sheet) : content(ParseFormula(text.substr(1))), sheet_(sheet) {}

        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

        void InvalidateCache();

        bool HasCache() const;

    private:
        
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> content;
        SheetInterface& sheet_;

    };

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell*> users_;
    std::unordered_set<Cell*> used_cells_;

};