# CompilerExperiment

## Lexical Analysis
### 任务描述

本关任务：用 C/C++ 编写一个 C 语言的语法分析器程序。

### 相关知识

为了完成本关任务，你需要掌握：1.DFA NFA，2.C/C++ 编程语言基础。3. C 语言的基本结构知识

#### 自动机

在编译原理课堂上已经教授了大家相关知识。在完成本实训前，一定要先设计相关自动机，再开始相关功能的实现。切勿，想到哪里，就编程到哪里，以至于代码一团糟，可维护性与可读性都很差。

#### C/C++

本实训涉及函数、结构体，标准流输入输出，字符串等操作

#### C语言基本结构

C 语言子集。 第一类：标识符 第二类：常数 第三类：保留字(`32`)

```
auto       break    case     char        const      continuedefault    do       double   else        enum       extern float      for      goto     if          int        longregister   return   short    signed      sizeof     static struct     switch   typedef  union       unsigned   void volatile    while
```

第四类：界符  `/*`、`//`、` ()`、` { }`、`[ ]`、`" "` 、 `'` 等 第五类：运算符 `<`、`<=`、`>`、`>=`、`=`、`+`、`-`、`*`、`/`、`^`等

**所有语言元素集合在 c_keys.txt **文件中。 **注意**，C_key.txt中缺少“//注释”的情况，请也映射到编号79！

### 编程要求

请仔细阅读该部分

#### 输入

样例输入放在`prog.txt`文件中 **样例1**输入

```
int main(){    printf("HelloWorld");    return 0;    }
```

#### 输出

输出要满足以下要求

```
计数: <符号名,符号标号>
```

注意，冒号后边有一个空格 **样例1**输出

```
1: <int,17>2: <main,81>3: <(,44>4: <),45>5: <{,59>6: <printf,81>7: <(,44>8: <",78>9: <HelloWorld,81>10: <",78>11: <),45>12: <;,53>13: <return,20>14: <0,80>15: <;,53>16: <},63>
```

**注意**，输出不能有多余的空格，回车等符号。请注意样例输出最后一行后是没有回车的！输出的符号都是英文的半角符号。

### ERROR

本实训不考虑错误处理，我保证输入的所有代码块是合法的 C 语言代码。

------

开始你的任务吧，祝你成功！

## 2. ll(1)解析器
### 任务描述
#### 本关任务：用C/C++编写一个LL(1)解析器

#### 相关知识
为了完成本关任务，你需要掌握：
- LL文法
- C/C++ 编程语言基础
- C语言的基本结构知识
- LL(1)解析器
在创建解析器之前，你应该创建一个下面文法的LL(1)分析表。

#### C/C++
本实训涉及函数、结构体，标准流输入输出，字符串等操作

### 实验要求
#### 实验文法定义
```
program -> compoundstmt
stmt ->  ifstmt  |  whilestmt  |  assgstmt  |  compoundstmt
compoundstmt ->  { stmts }
stmts ->  stmt stmts   |   E
ifstmt ->  if ( boolexpr ) then stmt else stmt
whilestmt ->  while ( boolexpr ) stmt
assgstmt ->  ID = arithexpr ;
boolexpr  ->  arithexpr boolop arithexpr
boolop ->   <  |  >  |  <=  |  >=  | ==
arithexpr  ->  multexpr arithexprprime
arithexprprime ->  + multexpr arithexprprime  |  - multexpr arithexprprime  |   E
multexpr ->  simpleexpr  multexprprime
multexprprime ->  * simpleexpr multexprprime  |  / simpleexpr multexprprime  |   E
simpleexpr ->  ID  |  NUM  |  ( arithexpr )
```
起始符
`Program`

保留字
```
{ }
if ( ) then else
while ( )
ID = 
> < >= <= ==
+ -
* /
ID NUM
```
E 是'空'
#### 分隔方式
同一行的输入字符用一个空格字符分隔，例如： ID = NUM ; 红色标记为空格

#### 错误处理
本实验需要考虑错误处理，如果程序不正确（包含语法错误），它应该打印语法错误消息（与行号一起），并且程序应该修正错误，并继续解析。
例如：

语法错误,第4行,缺少";"
输入
要求：在同一行中每个输入字符用一个空格字符分隔，无其余无关符号。

样例1输入
```
{
ID = NUM ;
}
```
样例2输入
```
{ 
If E1 
then
s1
else
If E2
Then
S2
else 
S3
}
并没有E1，E2等符号，这只是指代表达式
```
#### 输出
样例1输出
输出要求：在语法树同一层的叶子节点，在以下格式中有相同的缩进，用tab来控制缩减。如样例所示，相同颜色表示在语法树种他们在同一层。
![image](https://github.com/yueeeeeeeeeeeee-7/CompilerExperiment/assets/121533663/b1f23022-1b0e-4dfc-938a-8564c55e92b5)

## 3. LRparser
### 任务描述

本关任务：根据给定文法，用C/C++语言编写SLR(1) 或LR(1)语法分析器

### 相关知识

为了完成本关任务，你需要掌握：

1. LR文法
2. C/C++ 编程语言基础
3. C语言的基本结构知识

### LR分析器

在动手设计分析器之前，你应该先设计好下面文法的SLR(1)或LR(1)分析表。

#### C/C++

本实训涉及函数、结构体，标准流输入输出，字符串等操作

### 实验要求

实验文法定义

1. `program -> compoundstmt`
2. `stmt -> ifstmt | whilestmt | assgstmt | compoundstmt`
3. `compoundstmt -> { stmts }`
4. `stmts -> stmt stmts | E`
5. `ifstmt -> if ( boolexpr ) then stmt else stmt`
6. `whilestmt -> while ( boolexpr ) stmt`
7. `assgstmt -> ID = arithexpr ;`
8. `boolexpr -> arithexpr boolop arithexpr`
9. `boolop -> < | > | <= | >= | ==`
10. `arithexpr -> multexpr arithexprprime`
11. `arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E`
12. `multexpr -> simpleexpr multexprprime`
13. `multexprprime -> * simpleexpr multexprprime | / simpleexpr multexprprime | E`
14. `simpleexpr -> ID | NUM | ( arithexpr )`

#### 起始符

program

#### 保留字

1. `{ }`
2. `if ( ) then else`
3. `while ( )`
4. `ID = `
5. `> < >= <= ==`
6. `+ -`
7. `* /`
8. `ID NUM`
9. `E 是'空'`

#### 分隔方式

同一行的输入字符用一个空格字符分隔，例如： ID = NUM ; 

#### 错误处理

本实验需要考虑错误处理，如果程序不正确（包含语法错误），它应该打印语法错误消息（与行号一起），并且程序应该修正错误，并继续解析。 例如：

1. `语法错误,第4行,缺少";"`

#### 输入

要求：在同一行中每个输入字符用一个空格字符分隔，无其余无关符号。

#### 样例输入：

```
{ID = NUM ;}
{If E1 thens1elseIf E2ThenS2else S3}
```

**并没有E1，E2等符号，这只是指代表达式**

#### 输出

样例一输出

对于正确的程序，输出该程序的最右推导过程

对于有错误的的程序，输出错误问题并改正，继续输出正确的最右推导

每一组串之间均有一个空格符相隔开，分号，括号，=>符号后均有一个空格符隔开，每一句推导只占一行


## 4. 语义分析器
### 任务描述

本关任务：根据给定文法，用C/C++语言编写Translation Schema，执行程序并给出结果

#### 相关知识

为了完成本关任务，你需要掌握：

1. Translation Schema相关方法理论
2. C/C++ 编程语言基础
3. C语言的基本结构知识

#### Translation Schema

在动手设计之前，你应该先做好Translation Schema的相关准备工作。

你应该在你的程序中进行类型检查，以便对算术表达式(无论是整数算术还是实数算术)执行正确的操作。

#### C/C++

本实训涉及函数、结构体，标准流输入输出，字符串等操作

#### 实验要求

实验文法定义

1. `program -> decls compoundstmt`
2. `decls -> decl ; decls | E`
3. `decl -> int ID = INTNUM | real ID = REALNUM`
4. `stmt -> ifstmt | assgstmt | compoundstmt`
5. `compoundstmt -> { stmts }`
6. `stmts -> stmt stmts | E`
7. `ifstmt -> if ( boolexpr ) then stmt else stmt`
8. `assgstmt -> ID = arithexpr ;`
9. `boolexpr -> arithexpr boolop arithexpr`
10. `boolop -> < | > | <= | >= | ==`
11. `arithexpr -> multexpr arithexprprime`
12. `arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E`
13. `multexpr -> simpleexpr multexprprime`
14. `multexprprime -> * simpleexpr multexprprime | / simpleexpr multexprprime | E`
15. `simpleexpr -> ID | INTNUM | REALNUM | ( arithexpr )`

#### 起始符

program

#### 保留字

1. `{ }`
2. `if ( ) then else`
3. `ID = `
4. `> < >= <= ==`
5. `+ -`
6. `* /`
7. `ID INTNUM REALNUM`
8. `int ID = `
9. `real ID = `
10. `; `
11. `E 是'空'`

ID为标识符，均以小写字母表示，例如：a，b，c.....

INTNUM是正整数

REALNUM是一个正实数（即INTNUM . INTNUM）

#### 分隔方式

同一行的输入字符之间用一个空格字符分隔，例如：int a = 1 ; int b = 2 ;

#### 错误处理

本实验需要考虑错误处理，如果程序不正确，它应该输出语义错误信息（与行号一起）并退出，不需要进行错误改正。 例如：

1. `error message:line 1,realnum can not be translated into int type`

#### 输入

要求：在同一行中每个输入字符之间用一个空格字符分隔，无其余无关符号，输入输出全部为英文状态下字符。

#### 样例输入：

```
int a = 1 ; int b = 2 ; real c = 3.0 ;{a = a + 1 ;b = b * a ;if ( a < b ) then c = c / 2 ; else c = c / 4 ;}
```

#### 输出

a: 2

b: 4

c: 1.5

输出变量名及其数值，中间相隔一个空格
