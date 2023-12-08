#pragma once

#include <iostream>
#include <memory>
#include <string>
using namespace std;

static int nowt = 0;
/*

CompUnit    ::= FuncDef;

FuncDef     ::= FuncType IDENT "(" ")" Block;
FuncType    ::= "int";

Block       ::= "{" Stmt "}";
Stmt        ::= "return" Exp ";";

Exp         ::= AddExp;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";

MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;

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
    if(exp->isnum())
    {
      std::cout << "  ret ";
      exp->GenerateIR();
      std::cout << endl;
    }
    else{
      exp->GenerateIR();
      std::cout << "  ret %" << nowt - 1 << endl;
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
  std::unique_ptr<BaseAST> ae;

  void Dump() const override
  {
    std::cout << "ExpAST { ";
    ae->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    ae->GenerateIR();
  }
  bool isnum() const override { return ae->isnum(); }
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

enum UnaryOp
{
  UOP_NONE,
  UOP_PLUS,
  UOP_MINUS,
  UOP_NOT
};
// UnaryExp
class UnaryExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> pe_or_uoue;
  UnaryOp op;

  void Dump() const override
  {
    std::cout << "UnaryExpAST { ";
    switch (op)
    {
    case UOP_NONE:
      break;
    case UOP_PLUS:
      std::cout << "+, ";
      break;
    case UOP_MINUS:
      std::cout << "-, ";
      break;
    case UOP_NOT:
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
    case UOP_NONE:
      pe_or_uoue->GenerateIR();
      break;
    case UOP_PLUS:
      pe_or_uoue->GenerateIR();
      break;
    case UOP_MINUS:
      if (pe_or_uoue->isnum())
      {
        std::cout << "  %" << nowt << " = sub 0 , ";
        pe_or_uoue->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else
      {
        pe_or_uoue->GenerateIR();
        std::cout << "  %" << nowt << " = sub 0 , %" << nowt - 1 << endl;
        nowt++;
      }
      break;
    case UOP_NOT:
      if (pe_or_uoue->isnum())
      {
        std::cout << "  %" << nowt << " = eq ";
        pe_or_uoue->GenerateIR();
        std::cout << ", 0" << endl;
        nowt++;
      }
      else
      {
        pe_or_uoue->GenerateIR();
        std::cout << "  %" << nowt << " = eq %" << nowt - 1 << ", 0" << endl;
        nowt++;
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
    case UOP_NONE:
      return pe_or_uoue->isnum();
    case UOP_PLUS:
      return false;
      break;
    case UOP_MINUS:
      return false;
      break;
    case UOP_NOT:
      return false;
      break;
    default:
      break;
    }
  }
};

enum MulOp
{
  MOP_NONE,
  MOP_MUL,
  MOP_DIV,
  MOP_MOD
};
// MulExp
class MulExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ue, me;
  MulOp op;

  void Dump() const override
  {
    std::cout << "MulExpAST { ";
    switch (op)
    {
    case MOP_NONE:
      ue->Dump();
      break;
    case MOP_MUL:
      me->Dump();
      std::cout << " ,*, ";
      ue->Dump();
      break;
    case MOP_DIV:
      me->Dump();
      std::cout << " ,/, ";
      ue->Dump();
      break;
    case MOP_MOD:
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
    case MOP_NONE:
      ue->GenerateIR();
      break;
    case MOP_MUL:
      if (me->isnum() && ue->isnum())
      {
        std::cout << "  %" << nowt << " = mul ";
        me->GenerateIR();
        std::cout << ", ";
        ue->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else if (me->isnum())
      {
        ue->GenerateIR();
        std::cout << "  %" << nowt << " = mul ";
        me->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else if (ue->isnum())
      {
        me->GenerateIR();
        std::cout << "  %" << nowt << " = mul ";
        ue->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else
      {
        me->GenerateIR();
        int tempm = nowt - 1;
        ue->GenerateIR();
        int tempu = nowt - 1;
        std::cout << "  %" << nowt << " = mul %" << tempm << ", %" << tempu << endl;
        nowt++;
      }
      break;
    case MOP_DIV:
      if (me->isnum() && ue->isnum())
      {
        std::cout << "  %" << nowt << " = div ";
        me->GenerateIR();
        std::cout << ", ";
        ue->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else if (me->isnum())
      {
        ue->GenerateIR();
        std::cout << "  %" << nowt << " = div ";
        me->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else if (ue->isnum())
      {
        me->GenerateIR();
        std::cout << "  %" << nowt << " = div ";
        ue->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else
      {
        me->GenerateIR();
        int tempm = nowt - 1;
        ue->GenerateIR();
        int tempu = nowt - 1;
        std::cout << "  %" << nowt << " = div %" << tempm << ", %" << tempu << endl;
        nowt++;
      }
      break;
    case MOP_MOD:
      if (me->isnum() && ue->isnum())
      {
        std::cout << "  %" << nowt << " = mod ";
        me->GenerateIR();
        std::cout << ", ";
        ue->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else if (me->isnum())
      {
        ue->GenerateIR();
        std::cout << "  %" << nowt << " = mod ";
        me->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else if (ue->isnum())
      {
        me->GenerateIR();
        std::cout << "  %" << nowt << " = mod ";
        ue->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else
      {
        me->GenerateIR();
        int tempm = nowt - 1;
        ue->GenerateIR();
        int tempu = nowt - 1;
        std::cout << "  %" << nowt << " = mod %" << tempm << ", %" << tempu << endl;
        nowt++;
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
    case MOP_NONE:
      return ue->isnum();
    case MOP_MUL:
      return false;
      break;
    case MOP_DIV:
      return false;
      break;
    case MOP_MOD:
      return false;
      break;
    default:
      break;
    }
  }
};

enum AddOp
{
  AOP_NONE,
  AOP_PLUS,
  AOP_MINUS,
};
// AddExp
class AddExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> me, ae;
  AddOp op;

  void Dump() const override
  {
    std::cout << "AddExpAST { ";
    switch (op)
    {
    case AOP_NONE:
      me->Dump();
      break;
    case AOP_PLUS:
      ae->Dump();
      std::cout << " ,+, ";
      me->Dump();
      break;
    case AOP_MINUS:
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
    case AOP_NONE:
      me->GenerateIR();
      break;
    case AOP_PLUS:
      if (me->isnum() && ae->isnum())
      {
        std::cout << "  %" << nowt << " = add ";
        ae->GenerateIR();
        std::cout << ", ";
        me->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else if (me->isnum())
      {
        ae->GenerateIR();
        std::cout << "  %" << nowt << " = add ";
        me->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else if (ae->isnum())
      {
        me->GenerateIR();
        std::cout << "  %" << nowt << " = add ";
        ae->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else
      {
        ae->GenerateIR();
        int tempm = nowt - 1;
        me->GenerateIR();
        int tempu = nowt - 1;
        std::cout << "  %" << nowt << " = add %" << tempm << ", %" << tempu << endl;
        nowt++;
      }
      break;
    case AOP_MINUS:
      if (me->isnum() && ae->isnum())
      {
        std::cout << "  %" << nowt << " = sub ";
        ae->GenerateIR();
        std::cout << ", ";
        me->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else if (me->isnum())
      {
        ae->GenerateIR();
        std::cout << "  %" << nowt << " = sub ";
        me->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else if (ae->isnum())
      {
        me->GenerateIR();
        std::cout << "  %" << nowt << " = sub ";
        ae->GenerateIR();
        std::cout << ", %" << nowt - 1 << endl;
        nowt++;
      }
      else
      {
        ae->GenerateIR();
        int tempm = nowt - 1;
        me->GenerateIR();
        int tempu = nowt - 1;
        std::cout << "  %" << nowt << " = sub %" << tempm << ", %" << tempu << endl;
        nowt++;
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
    case AOP_NONE:
      return me->isnum();
    case AOP_PLUS:
      return false;
      break;
    case AOP_MINUS:
      return false;
      break;
    default:
      break;
    }
  }
};