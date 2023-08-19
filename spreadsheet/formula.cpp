#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) 
        try : ast_(ParseFormulaAST(expression)) {}
        catch (...)
        {
            throw FormulaException("incorrect formula");
        }

    Value Evaluate(const SheetInterface& sheet) const override {
        try 
        {
            std::function<double(Position)> args = [&sheet](const Position pos)->double{
                if(!pos.IsValid())
                {
                    throw FormulaError(FormulaError::Category::Ref);
                }
                const auto* cell = sheet.GetCell(pos);
                if (cell == nullptr)
                {
                    return 0.0;
                }
                if (std::holds_alternative<double>(cell->GetValue()))
                {
                    return std::get<double>(cell->GetValue());
                }
                else if (std::holds_alternative<std::string>(cell->GetValue()))
                {
                    const std::string value = std::get<std::string>(cell->GetValue());
                    if (value == "")
                    {
                        return 0.0;
                    }
                    std::istringstream is_value(value);
                    double result = 0.0;
                    if (is_value.eof() && is_value >> result)
                    {
                        return result;
                    }
                    else
                    {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                else
                {
                    throw FormulaError(std::get<FormulaError>(cell->GetValue()));
                }
            };
            return ast_.Execute(args);
        }
        catch (const FormulaError& error)
        {
            return error;
        }
    }

    std::string GetExpression() const override {
        std::ostringstream to_ret;
        ast_.PrintFormula(to_ret);
        return to_ret.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> to_ret;
        for (const auto& cell : ast_.GetCells())
        {
            if (cell.IsValid())
            {
                to_ret.push_back(cell);
            }
        }
        return to_ret;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}