#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

static int now_ = 0;
static int now_block = -1;
static int blockn = -1;
static int parentblock[32] = {0};
static std::unordered_map<std::string, int> const_vals;
static std::unordered_set<std::string> var_vals;

// 所有 AST 的基类
class BaseAST
{
public:
  virtual ~BaseAST() = default;
  virtual bool isnum() const { return false; }
  virtual void Dump() const = 0;
  virtual void GenerateIR() const = 0;
  virtual int calc() const { return 0; };
};

static void printcalc(const std::unique_ptr<BaseAST> &l, const std::unique_ptr<BaseAST> &r, string op_)
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
    std::cout << "{" << endl;
    std::cout << "%"
              << "entry:" << endl;
    block->GenerateIR();
    std::cout << "}";
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

// Decl
class DeclAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> cv;

  void Dump() const override
  {
    std::cout << "DeclAST { ";
    cv->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    cv->GenerateIR();
  }
};

// BType
class BTypeAST : public BaseAST
{
public:
  std::string type;

  void Dump() const override
  {
    std::cout << "BTypeAST { " << type << " }";
  }
  void GenerateIR() const override
  {
    if (type == "int")
    {
      std::cout << "i32 ";
    }
  }
};

// ConstDecl
class ConstDeclAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> bt, cdl;

  void Dump() const override
  {
    std::cout << "ConstDeclAST { ";
    bt->Dump();
    std::cout << " , ";
    cdl->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    cdl->GenerateIR();
  }
};

// ConstDefList
class ConstDefListAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> constdeflist;

  void AddConstDef(std::unique_ptr<BaseAST> &&cd)
  {
    constdeflist.push_back(std::move(cd));
  }
  void Dump() const override
  {
    std::cout << "ConstDefListAST { ";
    for (const auto &def : constdeflist)
    {
      if (def) // 确保指针非空
        def->Dump();
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    for (const auto &def : constdeflist)
    {
      if (def) // 确保指针非空
        def->GenerateIR();
    }
  }
};

// ConstDef
class ConstDefAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> civ;
  std::string ident;

  void Dump() const override
  {
    std::cout << "ConstDefAST { ";
    std::cout << ident << ", ";
    civ->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    string ident_ = ident + "_" + std::to_string(now_block);
    const_vals[ident_] = civ->calc();
  }
};

// ConstInitVal
class ConstInitValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ce;

  void Dump() const override
  {
    std::cout << "ConstInitValAST { ";
    ce->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
  }
  int calc() const override
  {
    return ce->calc();
  }
};

// VarDecl
class VarDeclAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> bt, vdl;

  void Dump() const override
  {
    std::cout << "VarDeclAST { ";
    bt->Dump();
    std::cout << " , ";
    vdl->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    vdl->GenerateIR();
  }
};

// VarDefList
class VarDefListAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> vardeflist;

  void AddVarDef(std::unique_ptr<BaseAST> &&cd)
  {
    vardeflist.push_back(std::move(cd));
  }
  void Dump() const override
  {
    std::cout << "VarDefListAST { ";
    for (const auto &def : vardeflist)
    {
      if (def) // 确保指针非空
        def->Dump();
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    for (const auto &def : vardeflist)
    {
      if (def) // 确保指针非空
        def->GenerateIR();
    }
  }
};

// VarDef
class VarDefAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> iv;
  std::string ident;
  bool init = false;

  void Dump() const override
  {
    std::cout << "ConstDefAST { ";
    std::cout << ", " << ident << ", ";
    if (init)
      iv->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    string ident_ = ident + "_" + std::to_string(now_block);
    var_vals.insert(ident_);
    std::cout << "  @" << ident_ << " = alloc i32" << endl;
    if (init)
    {
      if (iv->isnum())
        std::cout << "  store " << iv->calc() << ", @" << ident_ << endl;
      else
      {
        iv->GenerateIR();
        std::cout << "  store %" << now_ - 1 << ", @" << ident_ << endl;
      }
    }
  }
};

// InitVal
class InitValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> e;

  void Dump() const override
  {
    std::cout << "ConstInitValAST { ";
    e->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    e->GenerateIR();
  }
  int calc() const override
  {
    return e->calc();
  }
  bool isnum() const override
  {
    return e->isnum();
  }
};

// Block 也是 BaseAST
class BlockAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> bis;
  bool empty = false;

  void Dump() const override
  {
    if (!empty)
    {
      blockn++;
      parentblock[blockn] = now_block;
      now_block = blockn;
      std::cout << "BlockAST { " << now_block << " ";
      bis->Dump();
      std::cout << " }";
      now_block = parentblock[now_block];
      std::cout << now_block;
    }
  }
  void GenerateIR() const override
  {
    if (!empty)
    {
      blockn++;
      parentblock[blockn] = now_block;
      now_block = blockn;

      bis->GenerateIR();
      now_block = parentblock[now_block];
    }
  }
};

// BlockItems
class BlockItemsAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> bi, bis;
  bool s = false;

  void Dump() const override
  {
    std::cout << "BlockItemsAST { ";
    bi->Dump();
    if (s)
      bis->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    bi->GenerateIR();
    if (s)
      bis->GenerateIR();
  }
};

// BlockItem
class BlockItemAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ds;

  void Dump() const override
  {
    std::cout << "BlockItemAST { ";
    ds->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    ds->GenerateIR();
  }
};

// LVal
class LValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::string ident;

  void Dump() const override
  {
    std::cout << "LValAST { ";
    std::cout << ident;
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    int tempblock = now_block;
    while (tempblock != -1)
    {
      if (const_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
      {
        std::cout << const_vals[ident + "_" + std::to_string(tempblock)];
        break;
      }
      if (var_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
      {
        std::cout << "  %" << now_ << " = load @" << ident + "_" + std::to_string(tempblock) << endl;
        now_++;
        break;
      }
      tempblock = parentblock[tempblock];
    }
  }
  int calc() const override
  {
    int tempblock = now_block;
    while (tempblock != -1 && const_vals.count(ident + "_" + std::to_string(tempblock)) == 0)
      tempblock = parentblock[tempblock];
    string ident_ = ident + "_" + std::to_string(tempblock);
    return const_vals[ident_];
  }
  bool isnum() const override
  {
    int tempblock = now_block;
    while (tempblock != -1)
    {
      if (const_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
        return true;
      if (var_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
        return false;
      tempblock = parentblock[tempblock];
    }
    return false;
  }
};

// LeftVal
class LeftValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::string ident;

  void Dump() const override
  {
    std::cout << "LeftValAST { ";
    std::cout << ident;
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    int tempblock = now_block;
    while (tempblock != -1 && var_vals.count(ident + "_" + std::to_string(tempblock)) == 0)
      tempblock = parentblock[tempblock];
    string ident_ = ident + "_" + std::to_string(tempblock);
    std::cout << ident_;
  }
};

// Stmt 也是 BaseAST
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp, l, b;
  int type = 0;
  void Dump() const override
  {
    std::cout << "StmtAST { ";
    switch (type)
    {
    case 1:
      l->Dump();
      exp->Dump();
      break;
    case 2:
      std::cout << "return ";
      exp->Dump();
      break;
    case 3:
      std::cout << "return ";
      break;
    case 4:
      exp->Dump();
      break;
    case 5:
      break;
    case 6:
      b->Dump();
      break;
    default:
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (type)
    {
    case 1:
      if (exp->isnum())
      {
        std::cout << "  store ";
        exp->GenerateIR();
        std::cout << ", @";
        l->GenerateIR();
        std::cout << endl;
      }
      else
      {
        exp->GenerateIR();
        std::cout << "  store %" << now_ - 1 << ", @";
        l->GenerateIR();
        std::cout << endl;
      }
      break;
    case 2:
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
      break;
    case 3:
      break;
    case 4:
      exp->GenerateIR();
      break;
    case 5:
      break;
    case 6:
      b->GenerateIR();
      break;
    default:
      break;
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
  int calc() const override
  {
    return num;
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
  int calc() const override
  {
    return loe->calc();
  }
  bool isnum() const override { return loe->isnum(); }
};

// ConstExp
class ConstExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    std::cout << "ConstExpAST { ";
    exp->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
  }
  int calc() const override
  {
    return exp->calc();
  }
};

// PrimaryExp
class PrimaryExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> eln;

  void Dump() const override
  {
    std::cout << "PrimaryExpAST { ";
    eln->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    eln->GenerateIR();
  }
  int calc() const override
  {
    return eln->calc();
  }
  bool isnum() const override { return eln->isnum(); }
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
  std::unique_ptr<BaseAST> pu;
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
    pu->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (op)
    {
    case NONE:
      pu->GenerateIR();
      break;
    case PLUS:
      pu->GenerateIR();
      break;
    case MINUS:
      if (pu->isnum())
      {
        std::cout << "  %" << now_ << " = sub 0 , ";
        pu->GenerateIR();
        std::cout << endl;
        now_++;
      }
      else
      {
        pu->GenerateIR();
        std::cout << "  %" << now_ << " = sub 0 , %" << now_ - 1 << endl;
        now_++;
      }
      break;
    case NOT:
      if (pu->isnum())
      {
        std::cout << "  %" << now_ << " = eq ";
        pu->GenerateIR();
        std::cout << ", 0" << endl;
        now_++;
      }
      else
      {
        pu->GenerateIR();
        std::cout << "  %" << now_ << " = eq %" << now_ - 1 << ", 0" << endl;
        now_++;
      }
      break;
    default:
      break;
    }
  }
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return pu->calc();
      break;
    case PLUS:
      return pu->calc();
      break;
    case MINUS:
      return -(pu->calc());
      break;
    case NOT:
      return !(pu->calc());
      break;
    default:
      return 0;
      break;
    }
  }
  bool isnum() const override
  {
    switch (op)
    {
    case NONE:
      return pu->isnum();
    case PLUS:
      return pu->isnum();
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
      printcalc(me, ue, "mul");
      break;
    case DIV:
      printcalc(me, ue, "div");
      break;
    case MOD:
      printcalc(me, ue, "mod");
      break;
    default:
      break;
    }
  }
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return ue->calc();
      break;
    case MUL:
      return (me->calc()) * (ue->calc());
      break;
    case DIV:
      return (me->calc()) / (ue->calc());
      break;
    case MOD:
      return (me->calc()) % (ue->calc());
      break;
    default:
      return 0;
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
      printcalc(ae, me, "add");
      break;
    case MINUS:
      printcalc(ae, me, "sub");
      break;
    default:
      break;
    }
  }
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return me->calc();
      break;
    case PLUS:
      return (ae->calc()) + (me->calc());
      break;
    case MINUS:
      return (ae->calc()) - (me->calc());
      break;
    default:
      return 0;
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
      printcalc(re, ae, "lt");
      break;
    case GT_:
      printcalc(re, ae, "gt");
      break;
    case LEQ_:
      printcalc(re, ae, "le");
      break;
    case GEQ_:
      printcalc(re, ae, "ge");
      break;
    default:
      break;
    }
  }
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return ae->calc();
      break;
    case LT_:
      return (re->calc()) < (ae->calc());
      break;
    case GT_:
      return (re->calc()) > (ae->calc());
      break;
    case LEQ_:
      return (re->calc()) <= (ae->calc());
      break;
    case GEQ_:
      return (re->calc()) >= (ae->calc());
      break;
    default:
      return 0;
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
      printcalc(ee, re, "eq");
      break;
    case NEQ_:
      printcalc(ee, re, "ne");
      break;
    default:
      break;
    }
  }
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return re->calc();
      break;
    case EQ_:
      return (ee->calc()) == (re->calc());
      break;
    case NEQ_:
      return (ee->calc()) != (re->calc());
      break;
    default:
      return 0;
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
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return ee->calc();
    case LAND_:
      return (lae->calc()) && (ee->calc());
      break;
    default:
      return false;
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
  int calc() const override
  {
    switch (op)
    {
    case NONE:
      return lae->calc();
    case LOR_:
      return (loe->calc()) || (lae->calc());
      break;
    default:
      return false;
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
