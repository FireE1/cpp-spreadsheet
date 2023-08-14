#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

private:

    class Impl {
    public:
        
        virtual ~Impl() = default;
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;

    };

    class EmptyImpl : public Impl {
    public:

        Value GetValue() const override {
            return "";
        }

        std::string GetText() const override {
            return "";
        }

    };

    class TextImpl : public Impl {
    public:

        explicit TextImpl(const std::string& text) : content(text) {}

        Value GetValue() const override {
            if (content[0] == ESCAPE_SIGN)
            {
                return content.substr(1);
            }
            return content;
        }

        std::string GetText() const override {
            return content;
        }

    private:

        std::string content;

    };

    class FormulaImpl : public Impl {
    public:

        explicit FormulaImpl(const std::string& text) : content(ParseFormula(text.substr(1))) {}

        Value GetValue() const override {
            const auto value = content->Evaluate();
            if (std::holds_alternative<double>(value))
            {
                return std::get<double>(value);
            }
            else
            {
                return std::get<FormulaError>(value);
            }
        }

        std::string GetText() const override {
            return FORMULA_SIGN + content->GetExpression();
        }

    private:

        std::unique_ptr<FormulaInterface> content;

    };

    std::unique_ptr<Impl> impl_;

    SheetInterface& sheet_; // указывает к какой таблице принадлежит клетка

    std::set<Cell*> users_; // отсортированный список пользователей данной клетки

    std::set<Cell*> used_cells_; // отсортированный список клеток, которые использует данная клетка

    std::optional<Value> cache_; // при создании клетки и внесении в нее информации

    bool HasLoop(const Impl&); // проверяет наличие у пользователей используещихся в данной клетке используемых клеток, либо ее саму, в качестве пользователей

    void InvalidateCache(); // при неккоретном cache_, все пользователи так же инвалидируются

    void UpdateUsers(); // метод обновляет значение пользователей данной клетки

};