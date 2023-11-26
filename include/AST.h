#pragma once

#include <iostream>
#include <memory>
#include <string>
using namespace std;

/*
CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Number ";";
Number    ::= INT_CONST;

*/

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual void GenerateIR() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override {
    func_def->GenerateIR();
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override {
    std::cout << "fun ";
    std::cout << "@" << ident << "(): ";
    func_type->GenerateIR();
    block->GenerateIR();
  }
};

// FuncType 也是 BaseAST
class FuncTypeAST : public BaseAST {
 public:
  std::string type;

  void Dump() const override {
    std::cout << "FuncTypeAST { " << type << " }";
  }
  void GenerateIR() const override {
    if(type=="int"){
      std::cout << "i32 ";
    }
  }
};

// Block 也是 BaseAST
class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override {
    std::cout << "{" << endl;
    std::cout << "%" << "entry:" << endl;
    stmt->GenerateIR();
    std::cout << "}";
  }
};

// Stmt 也是 BaseAST
class StmtAST : public BaseAST {
 public:
  int number;

  void Dump() const override {
    std::cout << "StmtAST { " << number << " }";
  }
  void GenerateIR() const override {
    std::cout << "  ret " << number << endl;
  }
};

