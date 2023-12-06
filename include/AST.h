#pragma once

#include <iostream>
#include <memory>
#include <string>
using namespace std;

static int nowt = 0;
/*

CompUnit    ::= FuncDef;1

FuncDef     ::= FuncType IDENT "(" ")" Block;1
FuncType    ::= "int";1

Block       ::= "{" Stmt "}";1
Stmt        ::= "return" Exp ";";1

Exp         ::= UnaryExp;1
PrimaryExp  ::= "(" Exp ")" | Number;1
Number      ::= INT_CONST;1
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";


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
    exp->GenerateIR();
    std::cout << "  ret %" << nowt - 1 << endl;
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
  std::unique_ptr<BaseAST> ue;

  void Dump() const override
  {
    std::cout << "ExpAST { ";
    ue->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    ue->GenerateIR();
  }
  bool isnum() const override { return ue->isnum(); }
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
        std::cout << "  %" << nowt << " = eq 0 , ";
        pe_or_uoue->GenerateIR();
        std::cout << endl;
        nowt++;
      }
      else
      {
        pe_or_uoue->GenerateIR();
        std::cout << "  %" << nowt << " = eq 0 , %" << nowt - 1 << endl;
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
