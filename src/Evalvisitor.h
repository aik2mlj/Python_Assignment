#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H


#include "Python3BaseVisitor.h"
#include "typetools.h"


class EvalVisitor: public Python3BaseVisitor {
public:

  virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override;

  virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override;

  virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override;

  virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override;

  virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override { return ctx->NAME()->toString(); }

  virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override;

  virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override;

  virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override;

  virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override;

  virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override;

  virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override;

  virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override;

  virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override;

  virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override;

  virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override;

  virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override;

  virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override;

  virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override;

  virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override;

  virtual antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override;

  virtual antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override;

  virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override;

  virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override;

  virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override;

  virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override;

  virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override;

  virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override;

  virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;

  virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override;

  virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override;

  // return a vector storing the Vars or ALLTYPEs
  virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override;

  virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override;

  virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override;
//todo:override all methods of Python3BaseVisitor
};

#include <map>
#include <stack>
#include <set>

// all those FUNCTIONS
typedef std::map<std::string, Python3Parser::FuncdefContext*> FuncList;

// all those VARIABLES
// using stack to simulate the locate&static area.
// memall is *alltype, so that copying from the lower layer to the upper layer
// in the stack is easy.
// the varpool stores all the variables in order to release the memory once for all when destructing.
typedef std::map<std::string, memall> Variables;
typedef std::stack<Variables> VarList;

FuncList funclist;
VarList varlist;
Variables *globalvar;

// **************************不得不用的全局变量*******************************
std::stack<std::vector<string>> arg_names; // 形参名称表
std::stack<Variables> arg_vars;
bool is_default_function = 0;
// *************************************************************************

inline void newstack() {
    Variables new_vars = *globalvar;
    if(!arg_vars.top().empty()) {
        for(auto &x: arg_vars.top()) new_vars[x.first] = x.second;
    }
    varlist.push(new_vars);
}
inline void popstack() { varlist.pop(); }

inline void new_arg_names(std::vector<string> strs) { arg_names.push(strs); }
inline void new_arg_vars() { arg_vars.push(Variables()); }
inline void clear_arg_names() { arg_names.pop(); }
inline void clear_arg_vars() { arg_vars.pop(); }

inline bool has(const string &x) { return (bool)varlist.top().count(x); }

inline alltype getdata(const string &x) {
    if(has(x)) return varlist.top()[x].getData();
    else return alltype();
}

inline void renew(const string &x, const alltype &val) { //here!
    varlist.top()[x] = val;
}

inline void arg_renew(const string &x, const alltype &val) {
    arg_vars.top()[x] = val;
}

inline alltype getvalue(antlrcpp::Any x) {
    if(x.is<alltype>()) return x.as<alltype>();
    else if(x.is<Var>()) return x.as<Var>().second;
}

// *********************************** main part ******************************************

antlrcpp::Any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx) {
    Variables ini; ini.clear();
    varlist.push(ini);
    globalvar = &varlist.top();
    for(auto &x: ctx->stmt()) visit(x);
    return 0;
}

antlrcpp::Any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
    funclist.insert(FuncList::value_type(ctx->NAME()->toString(), ctx));
    return alltype();
}

// return a vector<string> of the names of the variables
antlrcpp::Any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx) {
    if(ctx->typedargslist() != nullptr) return visit(ctx->typedargslist());
    else return std::vector<string>();
}

antlrcpp::Any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx) {
    auto tfps = ctx->tfpdef();
    auto assigns = ctx->ASSIGN();
    auto tests = ctx->test();
    std::vector<string> names;
    int pos_of_assign = assigns.size() - 1; // assigns从后往前
    for(auto itfps = tfps.cend() - 1; itfps >= tfps.cbegin(); --itfps) {
        alltype now(None);
        if(pos_of_assign >= 0) {
            now = getvalue(visit(tests[pos_of_assign]));
            --pos_of_assign;
        }
        string var_name = (*itfps)->NAME()->toString();
        names.push_back(var_name);
        arg_renew(var_name, now);
    }
    std::reverse(names.begin(), names.end());
    return names;
}

antlrcpp::Any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx) {
    if(ctx->simple_stmt() != nullptr) return visit(ctx->simple_stmt());
    else return visit(ctx->compound_stmt());
}

antlrcpp::Any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
    return visit(ctx->small_stmt());
}

antlrcpp::Any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) {
    if(ctx->expr_stmt() != nullptr) return visit(ctx->expr_stmt());
    else return visit(ctx->flow_stmt());
}

// 赋值语句
antlrcpp::Any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
    auto tls = ctx->testlist();
    if(ctx->augassign() != nullptr) {
        string var = visit(tls[0]->test(0)).as<Var>().first;

        auto right_return = visit(tls[1]->test(0));
        alltype value;
        if(right_return.is<alltype>()) value = right_return.as<alltype>();
        else if(right_return.is<Var>()) value = right_return.as<Var>().second;

        Arithmetic aug = visit(ctx->augassign()).as<Arithmetic>();
        switch(aug) {
        case Add:
            renew(var, value + getdata(var)); return getdata(var);
        case Minus:
            renew(var, getdata(var) - value); return getdata(var);
        case Times:
            renew(var, getdata(var) * value); return getdata(var);
        case Divide:
            renew(var, getdata(var) / value); return getdata(var);
        case IDivide:
            renew(var, intdivide(getdata(var), value)); return getdata(var);
        default:
            renew(var, getdata(var) % value); return getdata(var);
        }
    } else if(ctx->ASSIGN().empty()) {
        return visit(ctx->testlist(0));
    } else {
        auto ass = ctx->ASSIGN();
        auto right_val = visit(tls[ass.size()]);
        std::vector<alltype> values;
        if(right_val.is<std::vector<alltype>>()) values = right_val.as<std::vector<alltype>>();
        else {
            auto tmp = right_val.as<std::vector<Var>>();
            for(auto &x: tmp) values.push_back(x.second);
        } // right values after the last ASSIGN
        for(auto itls = tls.cbegin(); itls < tls.cend() - 1; ++itls) {
            std::vector<Var> vars = visit(*itls).as<std::vector<Var>>();
            for(int i = 0; i < vars.size(); ++i) renew(vars[i].first, values[i]);
        }
        return *values.cbegin();
    }
}

antlrcpp::Any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx) {
    if(ctx->ADD_ASSIGN() != nullptr) return Add;
    else if(ctx->SUB_ASSIGN() != nullptr) return Minus;
    else if(ctx->MULT_ASSIGN() != nullptr) return Times;
    else if(ctx->DIV_ASSIGN() != nullptr) return Divide;
    else if(ctx->IDIV_ASSIGN() != nullptr) return IDivide;
    else return Mode;
}

antlrcpp::Any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
    if(ctx->break_stmt() != nullptr) return visit(ctx->break_stmt());
    else if(ctx->continue_stmt() != nullptr) return visit(ctx->continue_stmt());
    else return visit(ctx->return_stmt());
}

antlrcpp::Any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) {
    return flowtype(Break);
}

antlrcpp::Any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) {
    return flowtype(Continue);
}

antlrcpp::Any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) {
    if(ctx->testlist() != nullptr) {
        auto tmp = visit(ctx->testlist());
        if(tmp.is<std::vector<alltype>>()) {
            return flowtype(Return, tmp.as<std::vector<alltype>>());
        }
        else {
            std::vector<alltype> res;
            auto ttmp = tmp.as<std::vector<Var>>();
            for(auto &x: ttmp) res.push_back(x.second);
            return flowtype(Return, res);
        }
    }
    else return flowtype(Return);
}

antlrcpp::Any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) {
    if(ctx->while_stmt() != nullptr) return visit(ctx->while_stmt());
    else if(ctx->if_stmt() != nullptr) return visit(ctx->if_stmt());
    else return visit(ctx->funcdef());
}

antlrcpp::Any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
    alltype flag = false;
    auto tests = ctx->test();
    for(int i = 0; i < tests.size(); ++i) {
        flag = getvalue(visit(tests[i]));
        if((bool)flag) return visit(ctx->suite(i));
    }
    if(ctx->ELSE() != nullptr) return visit(*(ctx->suite().cend() - 1));
    else return flowtype(Normal);
}

// return a flowtype
antlrcpp::Any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
    auto flag = visit(ctx->test()).as<alltype>();
    while((bool)flag) {
        auto now = visit(ctx->suite());
        if(now.is<flowtype>()) {
            if(now.as<flowtype>()._type == Break) return flowtype(Normal);
            else if(now.as<flowtype>()._type == Return) return now;
        }
        flag = visit(ctx->test()).as<alltype>();
    }
    return flowtype(Normal);
}

antlrcpp::Any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
    if(ctx->simple_stmt() != nullptr) return visit(ctx->simple_stmt());
    else {
        for(auto &x: ctx->stmt()) {
            auto now = visit(x);
            if(now.is<flowtype>() && now.as<flowtype>()._type != Normal) {
                return now;
            }
        }
    }
    return flowtype(Normal);
}

antlrcpp::Any EvalVisitor::visitTest(Python3Parser::TestContext *ctx) {
    auto v = visit(ctx->or_test());
    // if(v.is<alltype>()) std::cout << "In test " << v.as<alltype>() << std::endl;
    return v;
}

antlrcpp::Any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
    if(ctx->and_test().size() == 1) {
        return visit(ctx->and_test(0));
    } else {
        for(auto &x: ctx->and_test()) {
            alltype now = getvalue(visit(x));
            if((bool)now) return alltype(true);
        }
        return alltype(false);
    }
}

antlrcpp::Any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
    if(ctx->not_test().size() == 1) {
        return visit(ctx->not_test(0));
    } else {
        for(auto &x: ctx->not_test()) {
            alltype now = getvalue(visit(x));
            if(!((bool)now)) return alltype(false);
        }
        return alltype(true);
    }
}

antlrcpp::Any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
    if(ctx->not_test() != nullptr) return !getvalue(visit(ctx->not_test()));
    else return visit(ctx->comparison());
}

antlrcpp::Any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
    auto ariths = ctx->arith_expr();
    if(!ctx->comp_op().empty()) {
        auto comps = ctx->comp_op();
        std::vector<alltype> values;
        values.push_back(getvalue(visit(ariths[0])));
        for(int i = 0; i < comps.size(); ++i) {
            Comp_Op op = visit(comps[i]).as<Comp_Op>();
            values.push_back(getvalue(visit(ariths[i + 1])));
            switch(op) {
            case L:
                if(values[i] >= values[i + 1]) return alltype(false); break;
            case M:
                if(values[i] <= values[i + 1]) return alltype(false); break;
            case E:
                if(values[i] != values[i + 1]) return alltype(false); break;
            case LE:
                if(values[i] > values[i + 1]) return alltype(false); break;
            case ME:
                if(values[i] < values[i + 1]) return alltype(false); break;
            default:
                if(values[i] == values[i + 1]) return alltype(false); break;
            }
        }
        return alltype(true);
    } else return visit(ctx->arith_expr(0));
}

antlrcpp::Any EvalVisitor::visitComp_op(Python3Parser::Comp_opContext *ctx) {
    if(ctx->LESS_THAN() != nullptr) return L;
    else if(ctx->GREATER_THAN() != nullptr) return M;
    else if(ctx->EQUALS() != nullptr) return E;
    else if(ctx->LT_EQ() != nullptr) return LE;
    else if(ctx->GT_EQ() != nullptr) return ME;
    else return NE;
}

antlrcpp::Any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
    auto terms = ctx->term();
    if(terms.size() > 1) {
        alltype res = getvalue(visit(terms[0]));
        auto adds = ctx->ADD();
        auto minus = ctx->MINUS();
        int pos_add = 0, pos_minus = 0;
        for(int i = 1; i < terms.size(); ++i) {
            if(pos_add < adds.size() && pos_minus < minus.size()) {
                if(adds[pos_add]->getSymbol()->getTokenIndex() < minus[pos_minus]->getSymbol()->getTokenIndex()) {
                    res += getvalue(visit(terms[i]));
                    ++pos_add;
                } else {
                    res -= getvalue(visit(terms[i]));
                    ++pos_minus;
                }
            } else if(pos_add < adds.size()) {
                res += getvalue(visit(terms[i]));
                ++pos_add;
            } else if(pos_minus < minus.size()) {
                res -= getvalue(visit(terms[i]));
                ++pos_minus;
            }
        }
        return res;
    }
    else return visit(ctx->term(0));
}

antlrcpp::Any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
    auto factors = ctx->factor();
    if(factors.size() > 1) {
        alltype res = getvalue(visit(*factors.cbegin()));
        std::vector<antlr4::tree::TerminalNode *> ops[4] = {ctx->STAR(), ctx->DIV(), ctx->IDIV(), ctx->MOD()};
        int pos[4] = {0, 0, 0, 0}, index[4] = {0};
        for(int i = 1; i < factors.size(); ++i) {
            int min_index = __INT_MAX__, min_op = 0;
            for(int j = 0; j < 4; ++j) if(pos[j] < ops[j].size()) {
                index[j] = ops[j][pos[j]]->getSymbol()->getTokenIndex();
                if(min_index > index[j]) min_index = index[j], min_op = j;
            }
            switch(min_op) {
            case 0: res *= getvalue(visit(factors[i])); break;
            case 1: res /= getvalue(visit(factors[i])); break;
            case 2: res = intdivide(res, getvalue(visit(factors[i]))); break;
            default: res %= getvalue(visit(factors[i])); break;
            }
            ++pos[min_op];
        }
        return res;
    }
    else return visit(ctx->factor(0));
}

antlrcpp::Any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
    if(ctx->atom_expr() != nullptr) return visit(ctx->atom_expr());
    else {
        if(ctx->MINUS() != nullptr) return (alltype)((-1) * getvalue(visit(ctx->factor())));
        else return getvalue(visit(ctx->factor()));
    }
}


// Be ware of the functions
// if it's a funtion, return vector<alltype>(size > 1) || alltype(size == 1)
antlrcpp::Any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
    if(ctx->trailer() == nullptr) return visit(ctx->atom());
    else {
        string func = visit(ctx->atom()).as<Var>().first;
        bool yes_its_default_function = 0;
        // print()
        if(func == "print") {
            yes_its_default_function = is_default_function; is_default_function = 1;
            std::vector<alltype> values = visit(ctx->trailer()).as<std::vector<alltype>>();
            if(values.empty()) std::cout << std::endl;
            else {
                for(int i = 0; i < values.size() - 1; ++i) std::cout << values[i] << " ";
                std::cout << *(values.cend() - 1) << std::endl;
            }
            is_default_function = yes_its_default_function;
            return alltype();
        }
        else if(func == "int") { // int()
            yes_its_default_function = is_default_function; is_default_function = 1;
            alltype res; res._type = Int;
            std::vector<alltype> values = visit(ctx->trailer()).as<std::vector<alltype>>();
            if(values[0]._type == Float) res.val_int = float_to_int(values[0].val_float);
            else res.val_int = values[0].val_int;
            is_default_function = yes_its_default_function;
            return res;
        }
        else if(func == "float") { // float()
            yes_its_default_function = is_default_function; is_default_function = 1;
            alltype res; res._type = Float;
            std::vector<alltype> values = visit(ctx->trailer()).as<std::vector<alltype>>();
            // std::cout << values[0] << std::endl;
            if(values[0]._type == Bool || values[0]._type == Int) res.val_float = (double)values[0].val_int;
            else res.val_float = values[0].val_float;
            is_default_function = yes_its_default_function;
            return res;
        }
        else if(func == "bool") { // bool()
            yes_its_default_function = is_default_function; is_default_function = 1;
            std::vector<alltype> values = visit(ctx->trailer()).as<std::vector<alltype>>();
            is_default_function = yes_its_default_function;
            return alltype((bool)values[0]);
        }
        else if(func == "str") { // str()
            yes_its_default_function = is_default_function; is_default_function = 1;
            std::vector<alltype> values = visit(ctx->trailer()).as<std::vector<alltype>>();
            is_default_function = yes_its_default_function;
            return alltype(String, values[0].toString());
        } else {
            yes_its_default_function = is_default_function;
            is_default_function = 0;
            auto func_node = funclist[func];
            new_arg_vars();
            new_arg_names(visit(func_node->parameters()).as<std::vector<string>>());

            auto num = visit(ctx->trailer()).as<std::vector<alltype>>();
            // for(auto &x: num) std::cout << x << std::endl;

            newstack();
            std::vector<alltype> res = visit(func_node->suite()).as<flowtype>().val;
            popstack();
            is_default_function = yes_its_default_function;

            clear_arg_vars();
            clear_arg_names();

            if(res.size() > 1) return res;
            else if(res.size() == 1) return res[0];
            else return alltype();
        }
    }
}

// return a Var if it's a NAME, an alltype if it's a value.
antlrcpp::Any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
    if(ctx->NAME() != nullptr) {
        return Var(ctx->NAME()->toString(), getdata(ctx->NAME()->toString()));
    }
    else if(ctx->NUMBER() != nullptr) {
        string str = ctx->NUMBER()->toString();
        if(str.find(".") == string::npos) return alltype(Int, str);
        else return alltype(Float, str);
    }
    else if(ctx->NONE() != nullptr) return alltype();
    else if(ctx->TRUE() != nullptr) return alltype(true);
    else if(ctx->FALSE() != nullptr) return alltype(false);
    else if(!ctx->STRING().empty()) {
        string str = "";
        for(auto &x: ctx->STRING()) {
            string tmp(x->toString(), 1, x->toString().length() - 2);
            str = str + tmp;
        }
        return alltype(String, str);
    }
    else return visit(ctx->test());
}

// return a vector<alltype>
antlrcpp::Any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx) {
    if(ctx->arglist() != nullptr) return visit(ctx->arglist());
    else return std::vector<alltype>();
}

// return a vector<alltype>
antlrcpp::Any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx) {
    auto args = ctx->argument();
    std::vector<alltype> values;
    int arg_name_pos = -1;
    for(int i = 0; i < args.size(); ++i) {
        auto now = visit(args[i]);
            // std::cout << "arg " << now.as<alltype>() << std::endl;
        if(now.is<alltype>()) {
            if(!is_default_function) {
                arg_renew(arg_names.top()[++arg_name_pos], now.as<alltype>());
            }
            values.push_back(now.as<alltype>());
        }
        else if(now.is<std::vector<alltype>>()) {
            std::vector<alltype> tmp = now.as<std::vector<alltype>>();
            if(!is_default_function) {
                for(auto &x: tmp) arg_renew(arg_names.top()[++arg_name_pos], x);
            }
            for(auto &x: tmp) values.push_back(x);
        }
        else values.push_back(getdata(now.as<string>()));
    }
    return values;
}

// return a alltype(be cautious about the list) or a NAME
antlrcpp::Any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
    if(ctx->NAME() == nullptr) {
        auto x = visit(ctx->test());
        // std::cout << x.as<alltype>() << std::endl;
        if(x.is<std::vector<alltype>>()) return x;
        else return getvalue(x);
    }
    else {
        string name = ctx->NAME()->toString();
        arg_renew(name, getvalue(visit(ctx->test())));
        return name;
    }
}

// return vector<Var> or vector<alltype>
antlrcpp::Any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
    auto tests = ctx->test();
    std::vector<antlrcpp::Any> nows;
    bool is_alltype = 0;
    for(auto &itest: tests) {
        auto now = visit(itest);
        nows.push_back(now);
        if(!now.is<Var>()) is_alltype = 1;
    }
    if(is_alltype) {
        std::vector<alltype> res;
        for(auto now: nows) {
            if((now.is<alltype>() || now.is<Var>())) res.push_back(getvalue(now)); 
            else { // 是函数的返回值，可能有多个
                std::vector<alltype> return_vals = now.as<std::vector<alltype>>();
                for(auto &x: return_vals) res.push_back(x);
            }
        }
        return res;
    } else {
        std::vector<Var> res;
        for(auto &now: nows) res.push_back(now.as<Var>());
        return res;
    }
}

#endif //PYTHON_INTERPRETER_EVALVISITOR_H
