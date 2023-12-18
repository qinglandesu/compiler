#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "koopa.h"
#include "../include/AST.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int32_t tempnum;
string reg[16] = {"x0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a1", "a2", "a3", "a4", "a5", "a6"};
int reg_used[16] = {0};
int tempr = 1;
int dr = 1;
unordered_map<koopa_raw_value_t, int> regnum;
int off = 0;
unordered_map<koopa_raw_value_t, int> offset;
int findreg();
void getreg(const koopa_raw_value_t lhs, const koopa_raw_value_t rhs, int &lr, int &rr);

// 访问 raw program
void Visit(const koopa_raw_program_t &program);
// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice);
// 访问函数
void Visit(const koopa_raw_function_t &func);
// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb);
// 访问指令
void Visit(const koopa_raw_value_t &value);
// 访问ret
void Visit(const koopa_raw_return_t &ret);
// 访问integer
void Visit(const koopa_raw_integer_t &integer);
// 访问binary
void Visit(const koopa_raw_binary_t &binary);
// 访问load
void Visit(const koopa_raw_load_t &load);
// 访问store
void Visit(const koopa_raw_store_t &store);

int main(int argc, const char *argv[])
{
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto option = argv[3];
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  fclose(yyin);

  if (string(mode) == "-ast")
  {
    if (string(option) == "-o")
    {
      ofstream outputFile(output);
      assert(outputFile.is_open());

      streambuf *coutbuf = cout.rdbuf();
      // 重定向标准输出到输出文件
      cout.rdbuf(outputFile.rdbuf());

      ast->Dump();

      outputFile.flush();
      outputFile.close();
      // 恢复标准输出
      cout.rdbuf(coutbuf);
    }
  }
  else if (string(mode) == "-koopa")
  {
    if (string(option) == "-o")
    {
      ofstream outputFile(output);
      assert(outputFile.is_open());

      streambuf *coutbuf = cout.rdbuf();
      // 重定向标准输出到输出文件
      cout.rdbuf(outputFile.rdbuf());

      ast->GenerateIR();

      outputFile.flush();
      outputFile.close();
      // 恢复标准输出
      cout.rdbuf(coutbuf);
    }
  }
  else if (string(mode) == "-riscv")
  {
    if (string(option) == "-o")
    {
      const char *temp = "temp.koopa";
      ofstream tempFile(temp);
      assert(tempFile.is_open());

      streambuf *coutbuf = cout.rdbuf();
      // 重定向标准输出到输出文件
      cout.rdbuf(tempFile.rdbuf());

      ast->GenerateIR();

      tempFile.flush();
      tempFile.close();

      // 恢复标准输出
      cout.rdbuf(coutbuf);

      // 重新打开文件以读取内容
      ifstream IRfile(temp);
      assert(IRfile.is_open());
      stringstream buffer;
      buffer << IRfile.rdbuf();
      string s = buffer.str();
      const char *str = s.c_str();
      IRfile.close();

      // 解析字符串 str, 得到 Koopa IR 程序
      koopa_program_t program;
      koopa_error_code_t retk = koopa_parse_from_string(str, &program);
      assert(retk == KOOPA_EC_SUCCESS); // 确保解析时没有出错
      // 创建一个 raw program builder, 用来构建 raw program
      koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
      // 将 Koopa IR 程序转换为 raw program
      koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
      // 释放 Koopa IR 程序占用的内存
      koopa_delete_program(program);

      ofstream outputFile(output);
      assert(outputFile.is_open());

      coutbuf = cout.rdbuf();
      // 重定向标准输出到输出文件
      cout.rdbuf(outputFile.rdbuf());

      Visit(raw);

      outputFile.flush();
      outputFile.close();
      // 恢复标准输出
      cout.rdbuf(coutbuf);

      // 处理完成, 释放 raw program builder 占用的内存
      // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
      // 所以不要在 raw program 处理完毕之前释放 builder
      koopa_delete_raw_program_builder(builder);
    }
  }
  return 0;
}

int findreg()
{
  int r = 0, r1 = 0;
  for (int i = 1; i <= 13; i++)
  {
    if (reg_used[i] == 0)
    {
      r = i;
      break;
    }
    if (r1 == 0) // 还未找到
    {
      if (reg_used[i] == 1)
      {
        r1 = i;
      }
    }
  }
  if (r)
    return r;
  else if (r1)
    return r1;
  else
    return 14;
}

void getreg(koopa_raw_value_t lhs, koopa_raw_value_t rhs, int &lr, int &rr)
{
  if (lhs->kind.tag == KOOPA_RVT_INTEGER)
  {
    if (lhs->kind.data.integer.value == 0)
      lr = 0;
    else
    {
      /*
      tempr = findreg();
      cout << " li " << reg[tempr] << ", " << lhs->kind.data.integer.value << tempnum << endl;
      lr = tempr;
      reg_used[tempr] = 2;
      */
      cout << "   li t0, " << lhs->kind.data.integer.value << endl;
      lr = 1;
    }
  }
  else
  {
    // lr = regnum[lhs];
    cout << " lw t0, " << offset[lhs] << "(sp)" << endl;
    lr = 1;
  }

  if (rhs->kind.tag == KOOPA_RVT_INTEGER)
  {
    if (rhs->kind.data.integer.value == 0)
      rr = 0;
    else
    {
      /*
      tempr = findreg();
      cout << " li " << reg[tempr] << ", " << rhs->kind.data.integer.value << tempnum << endl;
      rr = tempr;
      reg_used[tempr] = 2;
      */
      cout << " li t1, " << rhs->kind.data.integer.value << endl;
      rr = 2;
    }
  }
  else
  {
    // rr = regnum[rhs];
    cout << " lw t1, " << offset[rhs] << "(sp)" << endl;
    rr = 2;
  }
  /*
  if (lr == 0 && rr == 0)
  {
    tempr = findreg();
    dr = tempr;
    reg_used[tempr] = 2;
  }
  else if (lr == 0)
  {
    dr = rr;
  }
  else
  {
    dr = lr;
  }
  */
  dr = 1;
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(program.values);
  cout << " .text" << endl;
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice)
{
  for (size_t i = 0; i < slice.len; ++i)
  {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind)
    {
    case KOOPA_RSIK_FUNCTION:
      // 访问函数
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      // 访问基本块
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      // 访问指令
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      // 我们暂时不会遇到其他内容, 于是不对其做任何处理
      assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func)
{
  // 执行一些其他的必要操作
  const char *name = func->name;
  cout << " .globl " << name + 1 << endl;
  cout << name + 1 << ":" << endl;
  cout << " addi sp, sp, -256" << endl;
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb)
{
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value)
{
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag)
  {
  case KOOPA_RVT_RETURN: // 访问 return 指令
    Visit(kind.data.ret);
    break;
  case KOOPA_RVT_INTEGER: // 访问 integer 指令
    Visit(kind.data.integer);
    break;
  case KOOPA_RVT_BINARY: // 访问 binary 指令
    Visit(kind.data.binary);
    regnum[value] = dr;
    if (offset.find(value) == offset.end())
    {
      offset[value] = off;
      off += 4;
    }
    break;
  case KOOPA_RVT_LOAD: // 访问 load 指令
    Visit(kind.data.load);
    if (offset.find(value) == offset.end())
    {
      offset[value] = off;
      off += 4;
    }
    break;
  case KOOPA_RVT_STORE: // 访问 load 指令
    Visit(kind.data.store);
    break;
  case KOOPA_RVT_ALLOC: // 访问 alloc 指令
    break;
  default:
    // 其他类型暂时遇不到
    assert(false);
  }
}

// 访问对应类型指令的函数定义
// 访问ret
void Visit(const koopa_raw_return_t &ret)
{
  // 于是我们可以按照处理 return 指令的方式处理这个 value
  // return 指令中, value 代表返回值
  koopa_raw_value_t ret_value = ret.value;
  if (ret_value->kind.tag == KOOPA_RVT_INTEGER) // 按照处理 integer 的方式处理 ret_value
  {
    Visit(ret_value);
    cout << " li a0, " << tempnum << endl;
    cout << " addi sp, sp, 256" << endl;
    cout << " ret" << endl;
  }
  else
  {
    // cout << " mv a0, " << reg[regnum[ret_value]] << endl;
    cout << " lw a0, " << offset[ret_value] << "(sp)" << endl;
    cout << " addi sp, sp, 256" << endl;
    cout << " ret" << endl;
    reg_used[regnum[ret_value]] = 1;
  }
}

// 访问integer
void Visit(const koopa_raw_integer_t &integer)
{
  tempnum = integer.value;
}

// 访问binary
void Visit(const koopa_raw_binary_t &binary)
{
  auto lhs = binary.lhs;
  auto rhs = binary.rhs;
  int lr, rr;
  // 根据操作符类型选择不同的操作
  switch (binary.op)
  {
  case KOOPA_RBO_SUB:
    getreg(lhs, rhs, lr, rr);
    cout << " sub " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_ADD:
    getreg(lhs, rhs, lr, rr);
    cout << " add " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_MUL:
    getreg(lhs, rhs, lr, rr);
    cout << " mul " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_DIV:
    getreg(lhs, rhs, lr, rr);
    cout << " div " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_MOD:
    getreg(lhs, rhs, lr, rr);
    cout << " rem " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_EQ:
    getreg(lhs, rhs, lr, rr);
    cout << " xor " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    cout << " seqz " << reg[dr] << ", " << reg[dr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_NOT_EQ:
    getreg(lhs, rhs, lr, rr);
    cout << " xor " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    cout << " snez " << reg[dr] << ", " << reg[dr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_GT:
    getreg(lhs, rhs, lr, rr);
    cout << " slt " << reg[dr] << ", " << reg[rr] << ", " << reg[lr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_LT:
    getreg(lhs, rhs, lr, rr);
    cout << " slt " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_GE:
    getreg(lhs, rhs, lr, rr);
    cout << " slt " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    cout << " xori " << reg[dr] << ", " << reg[dr] << ", 1" << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_LE:
    getreg(lhs, rhs, lr, rr);
    cout << " slt " << reg[dr] << ", " << reg[rr] << ", " << reg[lr] << endl;
    cout << " xori " << reg[dr] << ", " << reg[dr] << ", 1" << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_AND:
    getreg(lhs, rhs, lr, rr);
    cout << " and " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  case KOOPA_RBO_OR:
    getreg(lhs, rhs, lr, rr);
    cout << " or " << reg[dr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    reg_used[lr] = 1;
    reg_used[rr] = 1;
    reg_used[dr] = 2;
    cout << " sw t0, " << off << "(sp)" << endl;
    break;
  default:
    break;
  }
}

// 访问load
void Visit(const koopa_raw_load_t &load)
{
  auto src = load.src;
  cout << " lw t0, " << offset[src] << "(sp)" << endl;
  cout << " sw t0, " << off << "(sp)" << endl;
}

// 访问store(顺便alloc)
void Visit(const koopa_raw_store_t &store)
{
  auto value = store.value;
  auto dest = store.dest;
  if (value->kind.tag == KOOPA_RVT_INTEGER)
  {
    cout << " li t0, " << value->kind.data.integer.value << endl;
  }
  else
  {
    cout << " lw t0, " << offset[value] << "(sp)" << endl;
  }
  if (offset.find(dest) == offset.end())
  {
    offset[dest] = off;
    off += 4;
  }
  cout << " sw t0, " << offset[dest] << "(sp)" << endl;
}
