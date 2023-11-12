// C++词法分析器
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;
/* 不要修改这个标准输入函数 */
void read_prog(string &prog) {
  char c;
  while (scanf("%c", &c) != EOF) {
    prog += c;
  }
}

/* 你可以添加其他函数 */

bool isDigit(char c) { return c >= '0' && c <= '9'; }

// 判断是不是字母或下划线
bool isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ;
}

bool isDelimiter(char c) {
  string delimiters = "(),.;[]{}\"\\:";

  for (int i = 0; i < delimiters.length(); i++) {
    if (c == delimiters[i]) {
      return true;
    }
  }
  return false;
}

bool isOperator(char c) {
  string operators = "<>%=+-*/^!&|?~";

  for (int i = 0; i < operators.length(); i++) {
    if (c == operators[i]) {
      return true;
    }
  }
  return false;
}

// 判断是不是标识符
bool isIdentifier(string word) {
  if (!isLetter(word[0]) || word[0] == '_')
    return false;

  for (int i = 1; i < word.length(); i++) {
    if (!isLetter(word[i]) && !isDigit(word[i])) {
      return false;
    }
  }
  return true;
}


struct token {
  int type;
  int id;
  string keyWord;
  token() {}
  token(string word, int id1) {
    keyWord = word;
    id = id1;
  }
};

class LexicalAnalysis {
public:
  string prog;
  vector<token> res;
  map<string, int> tokenMap;
  LexicalAnalysis() {
    tokenMap = {
        {"auto", 1},       {"break", 2},    {"case", 3},      {"char", 4},
        {"const", 5},      {"continue", 6}, {"default", 7},   {"do", 8},
        {"double", 9},     {"else", 10},    {"enum", 11},     {"extern", 12},
        {"float", 13},     {"for", 14},     {"goto", 15},     {"if", 16},
        {"int", 17},       {"long", 18},    {"register", 19}, {"return", 20},
        {"short", 21},     {"signed", 22},  {"sizeof", 23},   {"static", 24},
        {"struct", 25},    {"switch", 26},  {"typedef", 27},  {"union", 28},
        {"unsigned", 29},  {"void", 30},    {"volatile", 31}, {"while", 32},
        {"-", 33},         {"--", 34},      {"-=", 35},       {"->", 36},
        {"!", 37},         {"!=", 38},      {"%", 39},        {"%=", 40},
        {"&", 41},         {"&&", 42},      {"&=", 43},       {"(", 44},
        {")", 45},         {"*", 46},       {"*=", 47},       {",", 48},
        {".", 49},         {"/", 50},       {"/=", 51},       {":", 52},
        {";", 53},         {"?", 54},       {"[", 55},        {"]", 56},
        {"^", 57},         {"^=", 58},      {"{", 59},        {"|", 60},
        {"||", 61},        {"|=", 62},      {"}", 63},        {"~", 64},
        {"+", 65},         {"++", 66},      {"+=", 67},       {"<", 68},
        {"<<", 69},        {"<<=", 70},     {"<=", 71},       {"=", 72},
        {"==", 73},        {">", 74},       {">=", 75},       {">>", 76},
        {">>=", 77},       {"\"", 78},      {"Comment", 79},  {"Constant", 80},
        {"Identifier", 81}};
  };
  void token_scan(string raw_prog);

  int get_operator(int pos, string word);
  int get_number(int pos, string word);
  int get_letter(int pos, string word);
};

/// @brief 接受运算符或注释
/// @param pos
/// @param word
/// @return 下一个token的前一个位置
int LexicalAnalysis::get_operator(int pos, string word) {
  int i = pos;
  // int state = 0;
  //  剔除空白和注释,预读处理
  if (i < prog.size() - 1 && !isspace(prog[i+1])) {
    int peek = prog[i + 1];
    switch (prog[i]) {
    case '>':
      if (peek == '=') {
        word = ">=";
        break;
      } else if (peek == '>') {
        if (prog[i + 2] == '=') {
          word = ">>=";
          break;
        }
        word = ">>";
        break;
      }
      break;
    case '<':
      if (peek == '=') {
        word = "<=";
        break;
      } else if (peek == '<') {
        if (prog[i + 2] == '=') {
          word = "<<=";
          break;
        }
        word = "<<";
        break;
      }
      break;
    case '=':
      if (peek == '=') {
        word += peek;
      }
      break;
    case '+':
      if (peek == '+' || peek == '=')
        word += peek;
      break;
    case '-':
      if (peek == '-' || peek == '=' || peek == '>')
        word += peek;
      break;
    case '*':
      if (peek == '=')
        word += peek;
      break;
    case '!':
      if (peek == '=')
        word += peek;
      break;
    case '&':
      if (peek == '=' || peek == '&')
        word += peek;
      break;
    case '|':
      if (peek == '=' || peek == '|')
        word += peek;
      break;
    case '^':
      if (peek == '=')
        word += peek;
      break;
    case '%':
      if (peek == '=')
        word += peek;
      else if (isLetter(peek)) {
        word += peek;
        // 标识符
        res.push_back(token(word, 81));
        return pos + word.length() - 1;
      }
      break;
    case '/':
      if (peek == '/') {
        int temp = i;
        while (prog[temp + 1] != '\n') {
          temp++;
          word += prog[temp];
        }
        // 注释
        res.push_back(token( word, 79));
        return pos + word.length() - 1;
      } else if (peek == '*') {
        int temp = i;
        temp++;
        while (!(prog[temp] == '*' && prog[temp + 1] == '/'))
          word += prog[temp++];
        word += "*";
        word+="/";
        // 注释
        res.push_back(token( word, 79));
        return pos + word.length() - 1;
      } else if (peek == '=')
        word += peek;
      break;
    default:
      break;
    }
  }
  res.push_back(token( word, tokenMap[word]));
  return pos + word.length() - 1;
}

/// @brief 接受（浮点型）数字--定义有3个状态的自动机，0-整數 1-浮点数 2-其他
/// @param pos
/// @param word
/// @return 下一个token的前一个位置
int LexicalAnalysis::get_number(int pos, string word) {
  int temp = pos;
  int state = 0;
  while (temp < prog.length() - 1) {
    temp++;
    if (isDigit(prog[temp]) && (state == 1 || state == 0))
      ;
    else if (prog[temp] == '.' && state == 0) // 小数
    {
      state = 1;
    } else {
      break;
    }
    word += prog[temp];
  }
  res.push_back(token(word, 80));
  return pos + word.length() - 1;
}

unordered_set<string> reserved = {"auto",     "break",   "case",   "char",     "const",
                        "continue", "default", "do",     "double",   "else",
                        "enum",     "extern",  "float",  "for",      "goto",
                        "if",       "int",     "long",   "register", "return",
                        "short",    "signed",  "sizeof", "static",   "struct",
                        "switch",   "typedef", "union",  "unsigned", "void",
                        "volatile", "while"};

/// @brief 接受保留字或标识符
/// @param pos
/// @param word
/// @return 下一个token的前一个位置
int LexicalAnalysis::get_letter(int pos, string word) {
  int temp = pos;
  // int state = 0;
  while (temp + 1 < prog.length()) {
    if (isLetter(prog[temp + 1]) || isDigit(prog[temp + 1])) {
      word += prog[++temp];
    } else {
      break;
    }
  }
  if (reserved.count(word)) {
    res.push_back(token(word, tokenMap[word]));
  } else {
    res.push_back(token(word, 81));
  }
  return pos + word.length() - 1;
}

/// @brief 识别代码段并分割为token
/// @param raw_prog
/// @return
void LexicalAnalysis::token_scan(string raw_prog) {
  prog = raw_prog;
  string word = "";
  for (int i = 0; i < prog.length(); i++) {

    if (!isspace(prog[i])) {
      word = "";
      word += prog[i];

      // 根据接收的token首字符决定接受的方式
      if (isDigit(prog[i]))
        i = get_number(i, word);
      else if (isOperator(prog[i]))
        i = get_operator(i, word);
      else if (isLetter(prog[i]))
        i = get_letter(i, word);
      else if (isDelimiter(prog[i]))
        res.push_back(token(word, tokenMap[word]));
    }
  }
}

void Analysis() {
  string prog;
  read_prog(prog);
  /* 骚年们 请开始你们的表演 */
  /********* Begin *********/
  LexicalAnalysis lexicalAnalysis = LexicalAnalysis();
  lexicalAnalysis.token_scan(prog);
  for (const auto& token : lexicalAnalysis.res) {
      cout << "<" << token.keyWord << "," << token.id << ">" << endl;
  }

  /********* End *********/
}