#pragma once

#include <iostream>
#include <memory>
#include <string>
using namespace std;

static int now_ = 0;
/*

CompUnit    ::= FuncDef;

FuncDef     ::= FuncType IDENT "(" ")" Block;
FuncType    ::= "int";

Block       ::= "{" Stmt "}";
Stmt        ::= "return" Exp ";";

Exp         ::= LOrExp;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";

MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;

RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
*/

// 所有 AST 的基类
class BaseAST
{
public:
  virtual ~BaseAST() = default;
  virtual bool isnum() const { return false; }
  virtual void Dump() const = 0;
  virtual void GenerateIR() const = 0;
};

static void cal(const std::unique_ptr<BaseAST> &l, const std::unique_ptr<BaseAST> &r, string op_)
{
  if (l->isnum() && r->isnum())
  {
    std::cout << "  %" << now_ << " = " + op_ + " ";
    l->GenerateIR();
    std::cout << ", ";
    r->GenerateIR();
    std::cout << endl;
    now_++;
  }
  else if (l->isnum())
  {
    r->GenerateIR();
    std::cout << "  %" << now_ << " = " + op_ + " ";
    l->GenerateIR();
    std::cout << ", %" << now_ - 1 << endl;
    now_++;
  }
  else if (r->isnum())
  {
    l->GenerateIR();
    std::cout << "  %" << now_ << " = " + op_ + " %" << now_ - 1 << ", ";
    r->GenerateIR();
    std::cout << endl;
    now_++;
  }
  else
  {
    l->GenerateIR();
    int tempm = now_ - 1;
    r->GenerateIR();
    int tempu = now_ - 1;
    std::cout << "  %" << now_ << " = " + op_ + " %" << tempm << ", %" << tempu << endl;
    now_++;
  }
}

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override
  {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    func_def->GenerateIR();
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override
  {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    std::cout << "fun ";
    std::cout << "@" << ident << "(): ";
    func_type->GenerateIR();
    block->GenerateIR();
  }
};

// FuncType 也是 BaseAST
class FuncTypeAST : public BaseAST
{
public:
  std::string type;

  void Dump() const override
  {
    std::cout << "FuncTypeAST { " << type << " }";
  }
  void GenerateIR() const override
  {
    if (type == "int")
    {
      std::cout << "i32 ";
    }
  }
};

// Block 也是 BaseAST
class BlockAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override
  {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    std::cout << "{" << endl;
    std::cout << "%"
              << "entry:" << endl;
    stmt->GenerateIR();
    std::cout << "}";
  }
};

// Stmt 也是 BaseAST
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    std::cout << "StmtAST { ";
    exp->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (exp->isnum())
    {
      std::cout << "  ret ";
      exp->GenerateIR();
      std::cout << endl;
    }
    else
    {
      exp->GenerateIR();
      std::cout << "  ret %" << now_ - 1 << endl;
    }
  }
};

// Number
class NumberAST : public BaseAST
{
public:
  int num;

  void Dump() const override
  {
    std::cout << num;
  }
  void GenerateIR() const override
  {
    std::cout << num;
  }
  bool isnum() const override { return true; }
};

// Exp
class ExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> loe;

  void Dump() const override
  {
    std::cout << "ExpAST { ";
    loe->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    loe->GenerateIR();
  }
  bool isnum() const override { return loe->isnum(); }
};

// PrimaryExp
class PrimaryExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> n_or_e;

  void Dump() const override
  {
    std::cout << "PrimaryExpAST { ";
    n_or_e->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    n_or_e->GenerateIR();
  }
  bool isnum() const override { return n_or_e->isnum(); }
};

enum Op
{
  NONE,
  PLUS,
  MINUS,
  NOT,
  MUL,
  DIV,
  MOD,
  GT_,
  LT_,
  GEQ_,
  LEQ_,
  EQ_,
  NEQ_,
  LAND_,
  LOR_
};
// UnaryExp
class UnaryExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> pe_or_uoue;
  Op op;

  void Dump() const override
  {
    std::cout << "UnaryExpAST { ";
    switch (op)
    {
    case NONE:
      break;
    case PLUS:
      std::cout << "+, ";
      break;
    case MINUS:
      std::cout << "-, ";
      break;
    case NOT:
      std::cout << "!, ";
      break;
    default:
      break;
    }
    pe_or_uoue->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      pe_or_uoue->GenerateIR();
      break;
    case PLUS:
      pe_or_uoue->GenerateIR();
      break;
    case MINUS:
      if (pe_or_uoue->isnum())
      {
        std::cout << "  %" << now_ << " = sub 0 , ";
        pe_or_uoue->GenerateIR();
        std::cout << endl;
        now_++;
      }
      else
      {
        pe_or_uoue->GenerateIR();
        std::cout << "  %" << now_ << " = sub 0 , %" << now_ - 1 << endl;
        now_++;
      }
      break;
    case NOT:
      if (pe_or_uoue->isnum())
      {
        std::cout << "  %" << now_ << " = eq ";
        pe_or_uoue->GenerateIR();
        std::cout << ", 0" << endl;
        now_++;
      }
      else
      {
        pe_or_uoue->GenerateIR();
        std::cout << "  %" << now_ << " = eq %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return pe_or_uoue->isnum();
    case PLUS:
      return pe_or_uoue->isnum();
    case MINUS:
    case NOT:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// MulExp
class MulExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ue, me;
  Op op;

  void Dump() const override
  {
    std::cout << "MulExpAST { ";
    switch (op)
    {
    case NONE:
      ue->Dump();
      break;
    case MUL:
      me->Dump();
      std::cout << " ,*, ";
      ue->Dump();
      break;
    case DIV:
      me->Dump();
      std::cout << " ,/, ";
      ue->Dump();
      break;
    case MOD:
      me->Dump();
      std::cout << " ,%, ";
      ue->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      ue->GenerateIR();
      break;
    case MUL:
      cal(me, ue, "mul");
      break;
    case DIV:
      cal(me, ue, "div");
      break;
    case MOD:
      cal(me, ue, "mod");
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return ue->isnum();
    case MUL:
    case DIV:
    case MOD:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// AddExp
class AddExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> me, ae;
  Op op;

  void Dump() const override
  {
    std::cout << "AddExpAST { ";
    switch (op)
    {
    case NONE:
      me->Dump();
      break;
    case PLUS:
      ae->Dump();
      std::cout << " ,+, ";
      me->Dump();
      break;
    case MINUS:
      ae->Dump();
      std::cout << " ,-, ";
      me->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      me->GenerateIR();
      break;
    case PLUS:
      cal(ae, me, "add");
      break;
    case MINUS:
      cal(ae, me, "sub");
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return me->isnum();
    case PLUS:
    case MINUS:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// RelExp
class RelExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ae, re;
  Op op;

  void Dump() const override
  {
    std::cout << "RelExpAST { ";
    switch (op)
    {
    case NONE:
      ae->Dump();
      break;
    case LT_:
      re->Dump();
      std::cout << " ,<, ";
      ae->Dump();
      break;
    case GT_:
      re->Dump();
      std::cout << " ,>, ";
      ae->Dump();
      break;
    case LEQ_:
      re->Dump();
      std::cout << " ,<=, ";
      ae->Dump();
      break;
    case GEQ_:
      re->Dump();
      std::cout << " ,>=, ";
      ae->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      ae->GenerateIR();
      break;
    case LT_:
      cal(re, ae, "lt");
      break;
    case GT_:
      cal(re, ae, "gt");
      break;
    case LEQ_:
      cal(re, ae, "le");
      break;
    case GEQ_:
      cal(re, ae, "ge");
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return ae->isnum();
    case LT_:
    case GT_:
    case LEQ_:
    case GEQ_:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// EqExp
class EqExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> re, ee;
  Op op;

  void Dump() const override
  {
    std::cout << "EqExpAST { ";
    switch (op)
    {
    case NONE:
      re->Dump();
      break;
    case EQ_:
      ee->Dump();
      std::cout << " ,==, ";
      re->Dump();
      break;
    case NEQ_:
      ee->Dump();
      std::cout << " ,!=, ";
      re->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      re->GenerateIR();
      break;
    case EQ_:
      cal(ee, re, "eq");
      break;
    case NEQ_:
      cal(ee, re, "ne");
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return re->isnum();
    case EQ_:
    case NEQ_:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// LAndExp
class LAndExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ee, lae;
  Op op;

  void Dump() const override
  {
    std::cout << "LAndExpAST { ";
    switch (op)
    {
    case NONE:
      ee->Dump();
      break;
    case LAND_:
      lae->Dump();
      std::cout << " ,&&, ";
      ee->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      ee->GenerateIR();
      break;
    case LAND_:
      if (ee->isnum() && lae->isnum())
      {
        std::cout << "  %" << now_ << " = ne ";
        lae->GenerateIR();
        std::cout << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne ";
        lae->GenerateIR();
        std::cout << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = and %" << now_ - 2 << ", %" << now_ - 1 << endl;
        now_++;
      }
      else if (lae->isnum())
      {
        std::cout << "  %" << now_ << " = ne ";
        lae->GenerateIR();
        std::cout << ", 0" << endl;
        int tempnow_ = now_;
        now_++;
        ee->GenerateIR();
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = and %" << tempnow_ << ", %" << now_ - 1 << endl;
        now_++;
      }
      else if (ee->isnum())
      {
        lae->GenerateIR();
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne ";
        ee->GenerateIR();
        std::cout << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = and %" << now_ - 2 << ", %" << now_ - 1 << endl;
        now_++;
      }
      else
      {
        lae->GenerateIR();
        int templ = now_ - 1;
        ee->GenerateIR();
        int tempr = now_ - 1;
        std::cout << "  %" << now_ << " = ne %" << templ << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne %" << tempr << ", 0" << endl;
        now_++;
        std::cout << "  %" << now_ << " = and %" << now_ - 2 << ", %" << now_ - 1 << endl;
        now_++;
      }
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return ee->isnum();
    case LAND_:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};

// LOrExp
class LOrExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> lae, loe;
  Op op;

  void Dump() const override
  {
    std::cout << "LOrExpAST { ";
    switch (op)
    {
    case NONE:
      lae->Dump();
      break;
    case LOR_:
      loe->Dump();
      std::cout << " ,||, ";
      lae->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      lae->GenerateIR();
      break;
    case LOR_:
      if (loe->isnum() && lae->isnum())
      {
        std::cout << "  %" << now_ << " = or ";
        loe->GenerateIR();
        std::cout << ", ";
        lae->GenerateIR();
        std::cout << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      else if (loe->isnum())
      {
        lae->GenerateIR();
        std::cout << "  %" << now_ << " = or ";
        loe->GenerateIR();
        std::cout << ", %" << now_ - 1 << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      else if (lae->isnum())
      {
        loe->GenerateIR();
        std::cout << "  %" << now_ << " = or %" << now_ - 1 << ", ";
        lae->GenerateIR();
        std::cout << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      else
      {
        loe->GenerateIR();
        int templ = now_ - 1;
        lae->GenerateIR();
        int tempr = now_ - 1;
        std::cout << "  %" << now_ << " = or %" << templ << ", %" << tempr << endl;
        now_++;
        std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      break;
    default:
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return lae->isnum();
    case LOR_:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
};
