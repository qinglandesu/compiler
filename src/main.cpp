#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>
#include <map>
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
int nowr = 1; // 临时寄存器名
string reg[10] = {"x0", "t0", "t1", "t2", "t3", "t4", "t5", "t6"};
map<koopa_raw_value_t, int> m;
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

void getreg(koopa_raw_value_t lhs, koopa_raw_value_t rhs, int &lr, int &rr)
{
  if (lhs->kind.tag == KOOPA_RVT_INTEGER)
  {
    if (lhs->kind.data.integer.value == 0)
      lr = 0;
    else
    {
      cout << " li " << reg[nowr] << ", ";
      Visit(lhs->kind.data.integer);
      cout << tempnum << endl;
      lr = nowr;
      nowr++;
    }
  }
  else
    lr = m[lhs];

  if (rhs->kind.tag == KOOPA_RVT_INTEGER)
  {
    if (rhs->kind.data.integer.value == 0)
      rr = 0;
    else
    {
      cout << " li " << reg[nowr] << ", ";
      Visit(rhs->kind.data.integer);
      cout << tempnum << endl;
      rr = nowr;
      nowr++;
    }
  }
  else
    rr = m[rhs];
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
    m[value] = nowr - 1;
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
    cout << " ret" << endl;
  }
  else
  {
    cout << " mv a0, " << reg[nowr - 1] << endl;
    cout << " ret" << endl;
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
  case KOOPA_RBO_EQ: // 处理等于操作
    getreg(lhs, rhs, lr, rr);
    if (rr == 0)
    {
      cout << " xor " << reg[lr] << ", " << reg[lr] << ", " << reg[rr] << endl;
      cout << " seqz " << reg[lr] << ", " << reg[lr] << endl;
    }
    else
    {
      cout << " sub " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
      cout << " seqz " << reg[nowr] << ", " << reg[nowr] << endl;
      nowr++;
    }
    break;
  case KOOPA_RBO_SUB: // 处理减法操作
    getreg(lhs, rhs, lr, rr);
    cout << " sub " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    nowr++;
    break;
  case KOOPA_RBO_ADD:
    getreg(lhs, rhs, lr, rr);
    cout << " add " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    nowr++;
    break;
  case KOOPA_RBO_MUL:
    getreg(lhs, rhs, lr, rr);
    cout << " mul " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    nowr++;
    break;
  case KOOPA_RBO_DIV:
    getreg(lhs, rhs, lr, rr);
    cout << " div " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    nowr++;
    break;
  case KOOPA_RBO_MOD:
    getreg(lhs, rhs, lr, rr);
    cout << " rem " << reg[nowr] << ", " << reg[lr] << ", " << reg[rr] << endl;
    nowr++;
    break;
  default:
    break;
  }
}