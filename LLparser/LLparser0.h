// C语言词法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <set>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string &prog)
{
    char c;
    while (scanf("%c", &c) != EOF)
    {
        prog += c;
    }
}
/* 你可以添加其他函数 */

/**
 * @brief 定义token变量
 *
 */

struct token
{
    string value;
    int line;
    token(){};
    token(string v, int l)
    {
        value = v;
        line = l;
    }
};

string rules = R"(program -> compoundstmt
stmt -> ifstmt
stmt -> whilestmt
stmt -> assgstmt
stmt -> compoundstmt
compoundstmt -> { stmts }
stmts -> stmt stmts
stmts -> E
ifstmt -> if ( boolexpr ) then stmt else stmt
whilestmt -> while ( boolexpr ) stmt
assgstmt -> ID = arithexpr ;
boolexpr  -> arithexpr boolop arithexpr
boolop -> <
boolop -> >
boolop -> <=
boolop -> >=
boolop -> ==
arithexpr -> multexpr arithexprprime
arithexprprime -> + multexpr arithexprprime
arithexprprime -> - multexpr arithexprprime
arithexprprime -> E
multexpr -> simpleexpr  multexprprime
multexprprime -> * simpleexpr multexprprime
multexprprime -> / simpleexpr multexprprime
multexprprime -> E
simpleexpr -> ID
simpleexpr -> NUM
simpleexpr -> ( arithexpr ))";

/**
 * @brief 获取token
 *
 * @param rule
 * @return vector<token>
 */
vector<token> get_token(string rule)
{
    int line = 0;
    vector<token> ans;
    string t = "";

    for (auto i : rule)
    {
        if (i != ' ' && i != '\n')
        {
            t += i;
        }
        else if (i == ' ')
        {
            if (t != "")
                ans.push_back(token(t, line + 1));
            t = "";
        }
        if (i == '\n')
        {
            if (t != "")
            {
                ans.push_back(token(t, line + 1));
            }
            line++;
            t = "";
        }
    }
    if (t != "")
        ans.push_back(token(t, line + 1));
    return ans;
}

class LLtable
{
public:
    set<string> nonterminal;
    set<string> terminal;

    map<string, bool> hasNull;   // 是否是直接生成E
    map<string, int> hasNULLnum; // 记录直接生成E的产生式的序号

    vector<vector<int>> table;
    vector<vector<string>> production;
    map<string, bool> nullable;
    map<string, int> nullid;
    map<string, int> id;

    vector<set<string>> first;
    vector<set<string>> follow;

    void get_null();
    void get_first();
    void get_follow();
    void get_LLtable();
    LLtable();
};

/**
 * @brief 1. 保存非终结符和终结符，并将每个字符附一个唯一的id，用于生成预测表
 *        2. 初始化预测表，将每一个值赋为-1
 *
 */
LLtable::LLtable()
{
    vector<token> temp = get_token(rules);
    vector<string> p; // 当前的生成式
    int i = 0;
    int curline = -1;

    // 保存非终结符并保存生成式
    while (i < temp.size())
    {
        nonterminal.insert(temp[i].value);
        p.push_back(temp[i].value);
        i += 2; // 去除->
        curline = temp[i].line;
        while (i < temp.size() && curline == temp[i].line)
        {
            p.push_back(temp[i++].value);
        }
        production.push_back(p);
        p.clear();
    }

    int count0 = 0;
    set<string> empty_set{};
    for (auto s : nonterminal)
    {
        first.push_back(empty_set);
        follow.push_back(empty_set);
        hasNull[s] = false;
        id[s] = count0++;
    }

    terminal.insert("$");
    terminal.insert("E");
    for (auto p : production)
    {
        for (auto s : p)
        {
            if (s != "E" && !nonterminal.count(s))
            {
                terminal.insert(s);
            }
        }
    }

    for (auto s : terminal)
    {
        id[s] = count0++;
    }

    // 为了让标号更加统一，这里多生成了terminal.size()列的表格
    vector<int> row;
    for (int i = 0; i < terminal.size() + nonterminal.size() + 1; i++)
    {
        row.push_back(-1);
    }
    for (int i = 0; i < nonterminal.size() + 1; i++)
    {
        table.push_back(row);
    }
}

/**
 * @brief 记录所有可能产生E的非终结符
 *
 */
void LLtable::get_null()
{
    // 对于A->E类型的产生式，A直接标记为可以产生空串
    int count = -1;
    for (auto p : production)
    {
        count++;
        if (p.size() == 2 && p[1] == "E")
        {
            hasNull[p[0]] = true;
            hasNULLnum[p[0]] = count;
        }
    }

    // 对于A->BC类型的产生式，若B可能产生空串且C也可能产生空串的话，则将A标记为可能产生空串
    for (auto s : nonterminal)
    {
        int pnum = -1;
        for (auto p : production)
        {
            pnum++;
            int isNull = true;
            for (int i = 1; i < p.size(); i++)
            {
                if ((terminal.count(p[i]) && p[i] != "E") || !hasNull[p[i]])
                {
                    isNull = false;
                    break;
                }
            }
            if (isNull)
            {
                hasNull[p[0]] = true;
                hasNULLnum[p[0]] = pnum;
            }
        }
    }
}

/**
 * @brief 计算每个非终结符的first集合(每个first集中都不包含E)，并初步填写预测表
 *
 */
void LLtable::get_first()
{
    // 对于每个非终结符都再遍历一遍，计算每个产生式的非终结符的first集
    for (int i = 0; i < nonterminal.size(); i++)
    {
        // cout<<i<<endl;
        int pnum = -1;
        for (auto p : production)
        {
            pnum++;
            int j = 0;
            // 计算每个terminal对应的first集合
            do
            {
                j++;
                if (j >= p.size())
                    break;

                // 如果产生式右边为终结符
                if (terminal.count(p[j]))
                {
                    // 如果p[j]为空的话，继续求之后的first集
                    if (p[j] == "E")
                    {
                        continue;
                    }
                    // 如果是终结符的话，将其产生式的id填入到表中
                    table[id[p[0]]][id[p[j]]] = pnum;
                    first[id[p[0]]].insert(p[j]);
                    break;
                }

                for (auto f : first[id[p[j]]])
                {
                    // cout<<p[0]<<" "<<f<<endl;
                    table[id[p[0]]][id[f]] = pnum;
                    // cout<<id[p[0]]<<" "<<id[f]-14<<" "<<pnum<<endl;
                    first[id[p[0]]].insert(f);
                }
                // 如果产生式的右边为可以生成E的非终结符或终结符E的话，要继续递归计算
            } while (p[j] == "E" || hasNull[p[j]]);
        }
    }
    // for (int a = 0; a < nonterminal.size()+1; a++)
    // {
    //     for (int b = 14; b < terminal.size() + nonterminal.size() + 1; b++)
    //     {
    //         if(table[a][b]<10 && table[a][b]>=0)
    //         {
    //             cout<<" ";
    //         }
    //         cout <<" "<< table[a][b];
    //     }
    //     cout << endl;
    // }
    // cout << "-----------" << endl;
}

/**
 * @brief 计算follow集合，并按照规则将其添加到预测表中
 *
 */
void LLtable::get_follow()
{
    // 将$加入到开始符的集合中
    follow[id[production[0][0]]].insert("$");
    table[id[production[0][0]]][id["$"]] = 0;

    // int c = -1;
    // for (auto p : first)
    // {
    //     c++;
    //     cout << c << ":" << endl;
    //     for (auto s : p)
    //     {
    //         cout << s << " ";
    //     }
    //     cout << endl;
    // }

    for (int i = 0; i < nonterminal.size(); i++)
    {
        int count = -1; // 生成式的标号
        for (auto p : production)
        {
            count++;
            bool is_end = true;
            // 从后往前查看follow集，如果是p的最后一项，则一定是后面为空的
            for (int j = p.size() - 1; j >= 2; j--)
            {
                // 如果A→ αBβ的B是非终结符的话
                if (nonterminal.count(p[j - 1]))
                {
                    // 若β也是非终结符，就要看β的first集包不包括E
                    if (nonterminal.count(p[j]))
                    {
                        // 如果不包括E，则B的follow集=β的first集-E
                        for (auto f : first[id[p[j]]])
                        {
                            // 所有集合first集都不包括E，所以可以直接加
                            follow[id[p[j - 1]]].insert(f);
                        }
                        // 如果包括E，则B的follow集还有加上A的follow集
                        if (hasNull[p[j]])
                        {
                            for (auto f : follow[id[p[0]]])
                            {
                                follow[id[p[j - 1]]].insert(f);
                            }
                        }
                    }
                    else
                    {
                        follow[id[p[j - 1]]].insert(p[j]);
                        // 如果说B的first集合中有E，就把table[B][β]设置为B->E所在的产生式
                        if (hasNull[p[j - 1]])
                        {
                            table[id[p[j - 1]]][id[p[j]]] = hasNULLnum[p[j - 1]];
                        }
                    }
                }
                // 若β是最后一个字符，则对于b∈FOLLOW（A），将A->a加到M[A,b]中
                if (nonterminal.count(p[j]) && is_end)
                {
                    for (auto f : follow[id[p[0]]])
                    {
                        follow[id[p[j]]].insert(f);
                        if (hasNull[p[j]])
                        {
                            table[id[p[j]]][id[f]] = hasNULLnum[p[j]];
                        }
                    }
                }
                if (!hasNull[p[j]])
                {
                    is_end = false;
                }
            }
            // 对于产生式右边只存在一个式子，如果它是非终结符的话，把A的follow集加入到B的follow集中
            if (nonterminal.count(p[1]) && is_end)
            {
                for (auto f : follow[id[p[0]]])
                {
                    follow[id[p[1]]].insert(f);
                    if (hasNull[p[1]])
                    {
                        table[id[p[1]]][id[f]] = hasNULLnum[p[1]];
                    }
                }
            }
        }
    }
    // for (int a = 0; a < nonterminal.size() + 1; a++)
    // {
    //     for (int b = 14; b < terminal.size() + nonterminal.size() + 1; b++)
    //     {
    //         if (table[a][b] < 10 && table[a][b] >= 0)
    //         {
    //             cout << " ";
    //         }
    //         cout << " " << table[a][b];
    //     }
    //     cout << endl;
    // }
    // cout << "-----------" << endl;
}

/**
 * @brief 构造预测表
 *
 */
void LLtable::get_LLtable()
{
    get_null();
    get_first();
    get_follow();
}

/**
 * @brief 定义错误类型
 * 
 */
struct error
{
    string error_string;
    int line;
    error(){};
    error(int l, string s)
    {
        line = l;
        error_string = s;
    }
};

/**
 * @brief 错误处理
 * 
 */
class errorHandler
{
    vector<error> errors;

public:
    void report();
    void add_error(int l, string s);
};

void errorHandler::report()
{
    if (errors.empty())
    {
        return;
    }
    else
    {
        for (auto e : errors)
        {
            cout << "语法错误,第" << e.line << "行,缺少\"" << e.error_string << "\"" << endl;
        }
    }
}

void errorHandler::add_error(int l, string s)
{
    errors.push_back(error(l, s));
}

class LLparser
{
private:
    // 输入：先进先出
    deque<token> input;
    deque<token> p_stack;
    deque<int> depth;
    string parse_res;
    vector<token> tokens;
    LLtable g;

public:
    LLparser(){};
    void generate_LLtable();
    void execute(string input);
};

/**
 * @brief 生成预测表
 *
 */
void LLparser::generate_LLtable()
{
    g = LLtable();
    g.get_LLtable();
    // for (int i = 0; i < g.nonterminal.size() + 1; i++)
    // {
    //     for (int j = 12; j < g.nonterminal.size() + g.terminal.size() + 1; j++)
    //     {
    //         if (g.table[i][j] < 10 && g.table[i][j] >= 0)
    //         {
    //             cout << " ";
    //         }
    //         cout << " " << g.table[i][j];
    //     }
    //     cout << endl;
    // }
    // cout << "---------------------" << endl;
}

/**
 * @brief 输出结果
 *
 * @param s
 */
void LLparser::execute(string s)
{
    vector<token> split_s = get_token(s);
    for (int i = 0; i < split_s.size(); i++)
    {
        input.push_back(split_s[i]);
    }

    p_stack.push_front(token("program", -1));
    depth.push_front(0);
    string res;
    int depthhead;
    int curline = 1;
    token phead, inputhead;

    errorHandler err = errorHandler();
    while (!input.empty())
    {
        // cout<<"---------------"<<endl;
        // cout<<"input= "<<input.front().value <<endl;
        // cout<<"depth= "<<depth.front()<<endl;
        // cout<<"parse_res= "<<parse_res<<endl;

        res = parse_res;
        inputhead = input.front();
        depthhead = depth.front();
        depth.pop_front();
        phead = p_stack.front();
        p_stack.pop_front();

        for (int i = 0; i < depthhead; i++)
        {
            parse_res += '\t';
        }
        parse_res += phead.value;
        parse_res += '\n';

        // cout<<p_stack.size()<<' '<<input.size()<<endl;
        // cout<<phead.value<<' '<<inputhead.value<<endl;
        // 如果stack中存储的为终结符且和输入的相同，直接弹出
        if (inputhead.value == phead.value && g.terminal.count(phead.value))
        {
            curline = inputhead.line;
            input.pop_front();
        }
        // 如果stack中存储的是终结符，且可以在表中找到相对应的生成式
        else if (g.nonterminal.count(phead.value) && g.table[g.id[phead.value]][g.id[inputhead.value]] != -1)
        {
            int p = g.table[g.id[phead.value]][g.id[inputhead.value]];
            // cout<<"p= "<<p<<" "<<phead.value<<"->"<<inputhead.value<<endl;
            vector<string> pr = g.production[p];
            // 如果存储的对应的生成式是A->E的形式，则直接输出E
            if (pr.size() == 2 && pr[1] == "E")
            {
                for (int i = 0; i < depthhead + 1; i++)
                {
                    parse_res += '\t';
                }
                parse_res += "E\n";
            }
            else
            {
                // 否则将产生式倒着输入
                for (int i = pr.size() - 1; i >= 1; i--)
                {
                    depth.push_front(depthhead + 1);
                    p_stack.push_front(token(pr[i], curline));
                }
            }
        }
        else
        {
            // 错误处理
            if (g.terminal.count(phead.value))
            {
                err.add_error(curline, phead.value);
                input.push_front(token(phead.value, curline));
            }
            else if (g.hasNull[phead.value])
            {
                for (int i = 0; i < depthhead + 1; i++)
                {
                    parse_res += '\t';
                }
                parse_res += "E\n";
                continue;
            }
            depth.push_front(depthhead);
            p_stack.push_front(token(phead.value, curline));
            parse_res = res;
        }
    }
    if (!p_stack.empty())
    {
        cout << "代码不完整" << endl;
    }
    err.report();
    cout << parse_res;
}

void Analysis()
{
    string prog;
    read_prog(prog);
    /* 请开始 */
    /********* Begin *********/
    LLparser l = LLparser();
    l.generate_LLtable();
    l.execute(prog);
    /********* End *********/
}
