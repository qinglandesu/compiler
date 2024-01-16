#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
using namespace std;

static int now_ = 0;
static int if_cnt = -1;
static int now_circ = -1;
static int circ_cnt = -1;
static int parentcirc[256] = {0};
static int now_block = 0;
static int block_cnt = 0;
static int parentblock[512] = {0};
static bool block_ret = false;
static bool decl = false;
static bool paraminfunc = false;
static bool temptype = false;
static int param_num = 0;
static std::vector<std::string> param_val;
static std::vector<std::string> param_name;
static int aiv_depth = 0;
static std::vector<int> arrsize_;
static std::vector<int> arrinitval;
static std::unordered_map<std::string, bool> func_ret;
static std::unordered_map<std::string, int> func_params_num;
static std::unordered_map<std::string, int> const_vals;
static std::unordered_set<std::string> var_vals;
static std::unordered_set<std::string> arr_vals;

// 所有 AST 的基类
class BaseAST
{
public:
  bool glo = false;
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
  std::unique_ptr<BaseAST> cus;

  void Dump() const override
  {
    std::cout << "CompUnitAST { ";
    cus->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (!decl)
    {
      std::cout << "decl @getint(): i32\n\
decl @getch(): i32\n\
decl @getarray(*i32): i32\n\
decl @putint(i32)\n\
decl @putch(i32)\n\
decl @putarray(i32, *i32)\n\
decl @starttime()\n\
decl @stoptime()\n\n";
      decl = true;
      func_ret["getint"] = true;
      func_ret["getch"] = true;
      func_ret["getarray"] = true;
      func_params_num["getarray"] = 1;
      func_params_num["putint"] = 1;
      func_params_num["putch"] = 1;
      func_params_num["putarray"] = 2;
    }
    cus->GenerateIR();
  }
};

// CompUnits
class CompUnitsAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> cu, cus;
  bool s = false;

  void Dump() const override
  {
    std::cout << "CompUnitsAST { ";
    cu->Dump();
    if (s)
      cus->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    cu->GenerateIR();
    if (s)
      cus->GenerateIR();
  }
};

// SinCompUnit
class SinCompUnitAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> t, de;
  bool c;

  void Dump() const override
  {
    std::cout << "SinCompUnitAST { ";
    de->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (!c)
      t->GenerateIR();
    de->GenerateIR();
  }
};

// De
class DeAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> gf;

  void Dump() const override
  {
    std::cout << "DeAST { ";
    gf->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    gf->GenerateIR();
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
      temptype = 1;
    else
      temptype = 0;
  }
};

// FuncDef
class FuncDefAST : public BaseAST
{
public:
  std::string ident;
  std::unique_ptr<BaseAST> params;
  std::unique_ptr<BaseAST> block;
  bool p = false;

  void Dump() const override
  {
    std::cout << "FuncDefAST { ";
    std::cout << ident << " ";
    if (p)
      params->Dump();
    block->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    now_ = 0;
    std::cout << endl;
    std::cout << "fun ";
    std::cout << "@" << ident << "(";
    if (p)
      params->GenerateIR();
    func_params_num[ident] = param_num;
    param_num = 0;
    std::cout << ")";
    if (temptype)
      std::cout << ": i32";
    func_ret[ident] = temptype;
    std::cout << "{" << endl;
    std::cout << "%entry:" << endl;
    if (p)
    {
      paraminfunc = true;
      params->GenerateIR();
      paraminfunc = false;
    }
    block->GenerateIR();
    if (!block_ret)
    {
      if (func_ret[ident])
        std::cout << "  ret 0" << endl;
      else
        std::cout << "  ret" << endl;
    }
    std::cout << "}" << endl;
    std::cout << endl;
    block_ret = false;
    for (int i = 0; i < param_name.size(); i++)
    {
      auto it = var_vals.find(param_name[i]);
      if (it != var_vals.end())
      {
        var_vals.erase(it);
      }
    }
    param_name.clear();
  }
};

// FuncFParams
class FuncFParamsAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> p, ps;
  bool s = false;

  void Dump() const override
  {
    std::cout << "FuncFParamsAST { ";
    p->Dump();
    if (s)
    {
      std::cout << ",";
      ps->Dump();
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    p->GenerateIR();
    if (s)
    {
      if (!paraminfunc)
        std::cout << ",";
      ps->GenerateIR();
    }
  }
};

// FuncFParam
class FuncFParamAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> bt;
  std::string ident;

  void Dump() const override
  {
    std::cout << "VarDeclAST { ";
    bt->Dump();
    std::cout << ", " << ident;
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (paraminfunc)
    {
      std::cout << "  %" << ident << " = alloc i32" << endl;
      std::cout << "  store @" << ident << ", %" << ident << endl;
    }
    else
    {
      std::cout << "@" << ident << ": i32";
      param_num++;
      var_vals.insert(ident);
      param_name.push_back(ident);
    }
  }
};

// GloDecl
class GloDeclAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> d;

  void Dump() const override
  {
    std::cout << "GloDeclAST { ";
    d->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    d->glo = true;
    d->GenerateIR();
  }
};

// Decl
class DeclAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> d;

  void Dump() const override
  {
    std::cout << "DeclAST { ";
    d->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    d->glo = glo;
    d->GenerateIR();
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
    cdl->glo = glo;
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
      {
        def->glo = glo;
        def->GenerateIR();
      }
    }
  }
};

// ConstDef
class ConstDefAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> civ, size;
  std::string ident;
  bool arr = false;
  bool init = true;

  void Dump() const override
  {
    std::cout << "ConstDefAST { ";
    std::cout << ident << ", ";
    if (arr)
      size->Dump();
    civ->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (arr)
    {
      string ident_ = ident + "_" + std::to_string(now_block);
      arr_vals.insert(ident_);
      if (glo)
      {
        std::cout << "global";
      }
      std::cout << "  @" << ident_ << " = alloc ";
      size->GenerateIR();
      for (int i = 0; i < arrsize_.size(); i++)
      {
        std::cout << "[";
      }
      std::cout << "i32";
      for (int it = arrsize_.size() - 1; it >= 0; it--)
      {
        std::cout << ", " << arrsize_[it] << "]";
      }
      if (init)
      {
        civ->glo = glo;
        civ->GenerateIR();
        if (glo)
        {
          std::cout << ", ";
          std::cout << "{";
          for (int i = 0; i < arrinitval.size() - 1; i++)
            std::cout << arrinitval[i];
          std::cout << arrinitval[arrinitval.size() - 1] << "}";
        }
        else
        {
          std::cout << endl;
          int i = 0;
          for (i; i < arrinitval.size(); i++)
          {
            std::cout << "  %" << now_ << " = getelemptr @" << ident_ << ", " << i << endl;
            std::cout << "  store " << arrinitval[i] << ", %" << now_ << endl;
            now_++;
          }
          for (i; i < arrsize_.back(); i++)
          {
            std::cout << "  %" << now_ << " = getelemptr @" << ident_ << ", " << i << endl;
            std::cout << "  store " << 0 << ", %" << now_ << endl;
            now_++;
          }
        }
      }
      else
      {
        if (glo)
          std::cout << "zeroinit";
        else
        {
          for (int i = 0; i < arrsize_.back(); i++)
          {
            std::cout << "  %" << now_ << " = getelemptr @" << ident_ << ", " << i;
            std::cout << "  store " << 0 << ", %" << now_ << endl;
            now_++;
          }
        }
      }
      std::cout << endl;
      arrsize_.clear();
      arrinitval.clear();
    }
    else
    {
      string ident_ = ident + "_" + std::to_string(now_block);
      const_vals[ident_] = civ->calc();
    }
  }
};

// ArraySize
class ArraySizeAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> ce, as;
  bool s = false;

  void Dump() const override
  {
    std::cout << "ArraySizeAST { ";
    ce->Dump();
    if (s)
      as->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    arrsize_.push_back(ce->calc());
    if (s)
      as->GenerateIR();
  }
};

// ConstArrayInitVal
class ConstArrayInitValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> av;
  bool empty = false;

  void Dump() const override
  {
    std::cout << "ConstArrayInitValAST { ";
    if (!empty)
      av->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (empty)
      std::cout << "zeroinit";
    else
      av->GenerateIR();
  }
};

// ConstArrayVal
class ConstArrayValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> iv, av, aiv;
  int type;

  void Dump() const override
  {
    std::cout << "ConstArrayValAST { ";
    switch (type)
    {
    case 0:
      iv->Dump();
      break;
    case 1:
      iv->Dump();
      av->Dump();
      break;
    case 2:
      aiv->Dump();
      break;
    case 3:
      aiv->Dump();
      av->Dump();
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (type)
    {
    case 0:
      iv->GenerateIR();
      break;
    case 1:
      iv->GenerateIR();
      av->GenerateIR();
      break;
    case 2:
      aiv->GenerateIR();
      break;
    case 3:
      aiv->GenerateIR();
      av->GenerateIR();
      break;
    }
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
    arrinitval.push_back(ce->calc());
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
    vdl->glo = glo;
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
      {
        def->glo = glo;
        def->GenerateIR();
      }
    }
  }
};

// VarDef
class VarDefAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> iv, size;
  std::string ident;
  bool init = false;
  bool arr = false;

  void Dump() const override
  {
    std::cout << "VarDefAST { ";
    std::cout << ident << ", ";
    if (arr)
      size->Dump();
    if (init)
      iv->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (arr)
    {
      string ident_ = ident + "_" + std::to_string(now_block);
      arr_vals.insert(ident_);
      if (glo)
      {
        std::cout << "global";
      }
      std::cout << "  @" << ident_ << " = alloc ";
      size->GenerateIR();
      for (int i = 0; i < arrsize_.size(); i++)
      {
        std::cout << "[";
      }
      std::cout << "i32";
      for (int it = arrsize_.size() - 1; it >= 0; it--)
      {
        std::cout << ", " << arrsize_[it] << "]";
      }
      if (init)
      {
        iv->glo = glo;
        iv->GenerateIR();
        if (glo)
        {
          std::cout << ", ";
          std::cout << "{";
          for (int i = 0; i < arrinitval.size() - 1; i++)
            std::cout << arrinitval[i];
          std::cout << arrinitval[arrinitval.size() - 1] << "}";
        }
        else
        {
          std::cout << endl;
          int i = 0;
          for (i; i < arrinitval.size(); i++)
          {
            std::cout << "  %" << now_ << " = getelemptr @" << ident_ << ", " << i << endl;
            std::cout << "  store " << arrinitval[i] << ", %" << now_ << endl;
            now_++;
          }
          for (i; i < arrsize_.back(); i++)
          {
            std::cout << "  %" << now_ << " = getelemptr @" << ident_ << ", " << i << endl;
            std::cout << "  store " << 0 << ", %" << now_ << endl;
            now_++;
          }
        }
      }
      else
      {
        if (glo)
        {
          std::cout << ", zeroinit" << endl;
        }
      }
      std::cout << endl;
      arrsize_.clear();
      arrinitval.clear();
    }
    else
    {
      string ident_ = ident + "_" + std::to_string(now_block);
      var_vals.insert(ident_);
      if (glo)
      {
        std::cout << "global  @" << ident_ << " = alloc i32, ";
        if (init)
          std::cout << iv->calc() << endl;
        else
          std::cout << "zeroinit" << endl;
      }
      else
      {
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
    }
  }
};

// ArrayInitVal
class ArrayInitValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> av;
  bool empty = false;

  void Dump() const override
  {
    std::cout << "ArrayInitValAST { ";
    if (!empty)
      av->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (!empty)
      av->GenerateIR();
  }
};

// ArrayVal
class ArrayValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> iv, av, aiv;
  int type;

  void Dump() const override
  {
    std::cout << "ArrayValAST { ";
    switch (type)
    {
    case 0:
      iv->Dump();
      break;
    case 1:
      iv->Dump();
      av->Dump();
      break;
    case 2:
      aiv->Dump();
      break;
    case 3:
      aiv->Dump();
      av->Dump();
      break;
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    switch (type)
    {
    case 0:
      arrinitval.push_back(iv->calc());
      break;
    case 1:
      arrinitval.push_back(iv->calc());
      av->GenerateIR();
      break;
    case 2:
      aiv->GenerateIR();
      break;
    case 3:
      aiv->GenerateIR();
      av->GenerateIR();
      break;
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
      block_cnt++;
      parentblock[block_cnt] = now_block;
      now_block = block_cnt;
      std::cout << "BlockAST { " << now_block << " ";
      bis->Dump();
      std::cout << " }";
      now_block = parentblock[now_block];
      std::cout << now_block;
    }
  }
  void GenerateIR() const override
  {
    parentblock[0] = -1;
    if (!empty)
    {
      block_cnt++;
      parentblock[block_cnt] = now_block;
      now_block = block_cnt;
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
    if (!block_ret)
    {
      bi->GenerateIR();
      if (s)
        bis->GenerateIR();
    }
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
    if (!block_ret)
    {
      ds->GenerateIR();
    }
  }
};

// AllLval
class AllLvalAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> lval;

  void Dump() const override
  {
    std::cout << "AllLvalAST { ";
    lval->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    lval->GenerateIR();
  }
  int calc() const override
  {
    return lval->calc();
  }
  bool isnum() const override
  {
    return lval->isnum();
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
    bool find = false;
    int tempblock = now_block;
    while (tempblock != 0)
    {
      if (const_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
      {
        std::cout << const_vals[ident + "_" + std::to_string(tempblock)];
        find = true;
        break;
      }
      if (var_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
      {
        std::cout << "  %" << now_ << " = load @" << ident + "_" + std::to_string(tempblock) << endl;
        now_++;
        find = true;
        break;
      }
      tempblock = parentblock[tempblock];
    }
    if (!find && var_vals.count(ident) > 0)
    {
      std::cout << "  %" << now_ << " = load %" << ident << endl;
      now_++;
      find = true;
    }
    if (!find && var_vals.count(ident + "_" + std::to_string(0)) > 0)
    {
      std::cout << "  %" << now_ << " = load @" << ident + "_" + std::to_string(0) << endl;
      now_++;
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
    if (var_vals.count(ident) > 0)
    {
      return false;
    }
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

// ArrayLval
class ArrayLvalAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> p;
  std::string ident;

  void Dump() const override
  {
    std::cout << "ArrayLvalAST { " << ident << " ";
    p->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
  }
};

// ArrayPara
class ArrayParaAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> e, ap;
  bool s = false;

  void Dump() const override
  {
    std::cout << "ArrayParaAST { ";
    e->Dump();
    if (s)
      ap->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
  }
};

// LeftVal
class LeftValAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::string ident;
  std::unique_ptr<BaseAST> ap;
  bool arr = false;

  void Dump() const override
  {
    std::cout << "LeftValAST { ";
    std::cout << ident << " ";
    if (arr)
      ap->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    bool find = false;
    int tempblock = now_block;
    while (tempblock != 0)
    {
      if (var_vals.count(ident + "_" + std::to_string(tempblock)) > 0)
      {
        string ident_ = ident + "_" + std::to_string(tempblock);
        std::cout << "@" << ident_;
        find = true;
        break;
      }
      tempblock = parentblock[tempblock];
    }
    if (!find && var_vals.count(ident) > 0)
    {
      std::cout << "%" << ident;
      find = true;
    }
    if (!find && var_vals.count(ident + "_" + std::to_string(0)) > 0)
    {
      std::cout << "  @" << ident + "_" + std::to_string(0);
    }
  }
};

// Stmt 也是 BaseAST
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp, l, b, ifs, ws, bc;
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
    case 7:
      ifs->Dump();
      break;
    case 8:
      ifs->Dump();
      break;
    case 9:
      ws->Dump();
      break;
    case 10:
      bc->Dump();
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
        std::cout << ", ";
        l->GenerateIR();
        std::cout << endl;
      }
      else
      {
        exp->GenerateIR();
        std::cout << "  store %" << now_ - 1 << ", ";
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
      block_ret = true;
      break;
    case 3:
      cout << " ret" << endl;
      block_ret = true;
      break;
    case 4:
      exp->GenerateIR();
      break;
    case 5:
      break;
    case 6:
      b->GenerateIR();
      break;
    case 7:
      ifs->GenerateIR();
      break;
    case 8:
      ifs->GenerateIR();
      break;
    case 9:
      ws->GenerateIR();
      break;
    case 10:
      bc->GenerateIR();
      break;
    default:
      break;
    }
  }
};

// IfStmt
class IfStmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> e, ifs;

  void Dump() const override
  {
    std::cout << "IfStmtAST { ";
    e->Dump();
    std::cout << ", ";
    ifs->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if_cnt++;
    int now_if = if_cnt;
    if (e->isnum())
    {
      std::cout << "  %" << now_ << " = ne ";
      e->GenerateIR();
      std::cout << ", 0" << endl;
      now_++;
    }
    else
      e->GenerateIR();
    std::cout << "  br %" << now_ - 1 << ", %if_then" << now_if << ", %if_end" << now_if << endl;
    std::cout << endl;

    std::cout << "%if_then" << now_if << ":" << endl;
    ifs->GenerateIR();
    if (!block_ret)
      std::cout << "  jump %if_end" << now_if << endl;
    block_ret = false;
    std::cout << std::endl;

    std::cout << "%if_end" << now_if << ":" << endl;
  }
};

// IfElseStmt
class IfElseStmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> e, ifs, els;

  void Dump() const override
  {
    std::cout << "IfElseStmtAST { ";
    e->Dump();
    std::cout << ", ";
    ifs->Dump();
    std::cout << ", ";
    els->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if_cnt++;
    int now_if = if_cnt;
    int temp_ret = false;
    if (e->isnum())
    {
      std::cout << "  %" << now_ << " = ne ";
      e->GenerateIR();
      std::cout << ", 0" << endl;
      now_++;
    }
    else
      e->GenerateIR();
    std::cout << "  br %" << now_ - 1 << ", %if_then" << now_if << ", %if_else" << now_if << std::endl;
    std::cout << std::endl;

    std::cout << "%if_then" << now_if << ":" << std::endl;
    ifs->GenerateIR();
    temp_ret = block_ret;
    if (!block_ret)
      std::cout << "  jump %if_end" << now_if << std::endl;
    block_ret = false;
    std::cout << std::endl;

    std::cout << "%if_else" << now_if << ":" << std::endl;
    els->GenerateIR();
    temp_ret = temp_ret && block_ret;
    if (!block_ret)
      std::cout << "  jump %if_end" << now_if << std::endl;
    block_ret = temp_ret;
    std::cout << std::endl;

    if (!block_ret)
      std::cout << "%if_end" << now_if << ":" << std::endl;
  }
};

// WhileStmt
class WhileStmtAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> e, ws;

  void Dump() const override
  {
    circ_cnt++;
    parentcirc[circ_cnt] = now_circ;
    now_circ = circ_cnt;
    std::cout << "WhileStmtAST { ";
    e->Dump();
    std::cout << ", ";
    ws->Dump();
    std::cout << " }";
    now_circ = parentcirc[now_circ];
    std::cout << now_circ;
  }
  void GenerateIR() const override
  {
    circ_cnt++;
    parentcirc[circ_cnt] = now_circ;
    now_circ = circ_cnt;
    int now_w = now_circ;

    std::cout << "  jump %while_entry" << now_w << std::endl;
    std::cout << std::endl;

    std::cout << "%while_entry" << now_w << ":" << std::endl;
    if (e->isnum())
    {
      std::cout << "  %" << now_ << " = ne ";
      e->GenerateIR();
      std::cout << ", 0" << endl;
      now_++;
    }
    else
      e->GenerateIR();
    std::cout << "  br %" << now_ - 1 << ", %while_body" << now_w << ", %while_end" << now_w << endl;
    std::cout << endl;

    std::cout << "%while_body" << now_w << ":" << endl;
    ws->GenerateIR();
    if (!block_ret)
      std::cout << "  jump %while_entry" << now_w << endl;
    block_ret = false;
    now_circ = parentcirc[now_circ];
    std::cout << std::endl;
    std::cout << "%while_end" << now_w << ":" << endl;
  }
};

// BreakContinueStmt
class BCAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::string bc;

  void Dump() const override
  {
    std::cout << "BCAST { " << bc << " }";
  }
  void GenerateIR() const override
  {
    if (now_circ != -1)
    {
      if (bc == "break")
      {
        std::cout << "  jump %while_end" << now_circ << std::endl;
        block_ret = 1;
      }
      else
      {
        std::cout << "  jump %while_entry" << now_circ << std::endl;
        block_ret = 1;
      }
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
  std::unique_ptr<BaseAST> pu, f;
  Op op;
  bool func = false;

  void Dump() const override
  {
    std::cout << "UnaryExpAST { ";
    if (func)
    {
      f->Dump();
    }
    else
    {
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
    }
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (func)
      f->GenerateIR();
    else
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
    if (func)
      return false;
    else
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
  }
};

// FuncExp
class FuncExpAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> param;
  std::string ident;
  bool p = false;

  void Dump() const override
  {
    std::cout << "FuncExpAST { ";
    std::cout << ident << " ";
    if (p)
      param->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (p)
      param->GenerateIR();
    if (func_ret[ident])
    {
      std::cout << "  %" << now_ << " = call @" << ident << "(";
      now_++;
      if (ident == "getarray" || p)
      {
        for (auto it = param_val.end() - func_params_num[ident]; it != param_val.end(); it++)
        {
          if (it != param_val.end() - func_params_num[ident])
            std::cout << ',';
          std::cout << *it;
        }
        for (int i = func_params_num[ident]; i > 0; i--)
        {
          param_val.pop_back();
        }
      }
      std::cout << ")" << endl;
    }
    else
    {
      std::cout << "  call @" << ident << "(";
      if (p)
      {
        for (auto it = param_val.end() - func_params_num[ident]; it != param_val.end(); it++)
        {
          if (it != param_val.end() - func_params_num[ident])
            std::cout << ',';
          std::cout << *it;
        }
        for (int i = func_params_num[ident]; i > 0; i--)
        {
          param_val.pop_back();
        }
      }
      std::cout << ")" << endl;
    }
  }
};

// FuncRParams
class FuncRParamsAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> p, ps;
  bool s = false;

  void Dump() const override
  {
    std::cout << "FuncRParamsAST { ";
    p->Dump();
    if (s)
      ps->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    p->GenerateIR();
    if (s)
      ps->GenerateIR();
  }
};

// FuncRParam
class FuncRParamAST : public BaseAST
{
public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    std::cout << "FuncRParamAST { ";
    exp->Dump();
    std::cout << " }";
  }
  void GenerateIR() const override
  {
    if (exp->isnum())
      param_val.push_back(to_string(exp->calc()));
    else
    {
      exp->GenerateIR();
      param_val.push_back("%" + to_string(now_ - 1));
    }
  }
  bool isnum() const override
  {
    return exp->isnum();
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
      cout << "m" << endl;
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
        if (!lae->calc())
        {
          std::cout << "  %" << now_ << " = ne ";
          lae->GenerateIR();
          std::cout << ", 0" << endl;
          now_++;
        }
        else
        {
          ee->GenerateIR();
          std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
          now_++;
        }
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
        std::cout << "  @result_" << templ << " = alloc i32" << std::endl;
        std::cout << "  %" << now_ << " = ne %" << templ << ", 0" << endl;
        std::cout << "  store %" << now_ << ", @result_" << templ << std::endl;
        now_++;

        if_cnt++;
        int now_if = if_cnt;
        std::cout << "  br %" << now_ - 1 << ", %then" << now_if << ", %end" << now_if << std::endl;
        std::cout << std::endl;

        std::cout << "%then" << now_if << ":" << std::endl;
        ee->GenerateIR();
        int tempr = now_ - 1;
        std::cout << "  %" << now_ << " = ne %" << tempr << ", 0" << endl;
        std::cout << "  store " << '%' << now_ << ", @result_" << templ << std::endl;
        now_++;
        std::cout << "  jump %end" << now_if << std::endl;
        std::cout << std::endl;

        std::cout << "%end" << now_if << ":" << std::endl;
        std::cout << "  %" << now_ << "= load @result_" << templ << std::endl;
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
        if (loe->calc())
        {
          std::cout << "  %" << now_ << " = ne ";
          loe->GenerateIR();
          std::cout << ", 0" << endl;
          now_++;
        }
        else
        {
          lae->GenerateIR();
          std::cout << "  %" << now_ << " = ne %" << now_ - 1 << ", 0" << endl;
          now_++;
        }
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
        std::cout << "  @result_" << templ << " = alloc i32" << std::endl;
        std::cout << "  %" << now_ << " = ne %" << templ << ", 0" << endl;
        std::cout << "  store %" << now_ << ", @result_" << templ << std::endl;
        now_++;

        if_cnt++;
        int now_if = if_cnt;
        std::cout << "  br %" << now_ - 1 << ", %end" << now_if << ", %then" << now_if << std::endl;
        std::cout << std::endl;

        std::cout << "%then" << now_if << ":" << std::endl;
        lae->GenerateIR();
        int tempr = now_ - 1;
        std::cout << "  %" << now_ << " = ne %" << tempr << ", 0" << endl;
        std::cout << "  store " << '%' << now_ << ", @result_" << templ << std::endl;
        now_++;
        std::cout << "  jump %end" << now_if << std::endl;
        std::cout << std::endl;

        std::cout << "%end" << now_if << ":" << std::endl;
        std::cout << "  %" << now_ << "= load @result_" << templ << std::endl;
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
