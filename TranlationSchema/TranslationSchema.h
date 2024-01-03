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
		if (c == '#')
			break;
		prog += c;
	}
}
/* 你可以添加其他函数 */
/// SLR parsing table表项对应的枚举类型
enum
{
	SHIFT,
	REDUCE,
	GOTO,
	ACC
};

enum
{
	REAL,
	INT,
	ID
};

enum Comparator
{
	greater,
	greaterEqual,
	less,
	lessEqual,
	equal
};

enum Operator
{
	plus,
	sub,
	divide,
	mult
};

enum
{
	EMPTY,
	ASSIGN,
	BOOL,
	ARITH,
	IF,
	DECLARE,
	ARITHPRIME,
	INSTANT,
	IDVALUE,
	BOOLOP
};

/**
 * @brief 定义token变量
 *
 */
struct token
{
	string s;
	double value;
	string name;
	int type;
	int line;

	token() {}
	token(string st, double v, int t, int ln)
	{
		s = st;
		value = v;
		type = t;
		line = ln;
	}
	token(string st, string nn, int t, int ln)
	{
		s = st;
		name = nn;
		type = t;
		line = ln;
	}
};

struct entry
{
	int type; // 项的类型-移入、归约、接受
	int num;  // 执行该动作之后转移到的状态或归约所使用的产生式的编号等
	entry(){};
	entry(int t, int n)
	{
		type = t;
		num = n;
	}
};

struct item
{
	int id;	   // 产生式的索引
	int index; // 点的索引
	item(){};
	item(int d, int in)
	{
		id = d;
		index = in;
	}
};

struct expr
{
	entry op;
	vector<token> tokens;

	expr() {}
	expr(entry o, vector<token> ts)
	{
		op = o;
		tokens = ts;
	}
};

/**
 * @brief 定义错误类型
 *
 */
enum
{
	DivideByZero,
	int2num,
	real2int,
	Delimetre,
	UNDECLARED_ID,
	UNDECLARED_ID_ASSIGN,
	UNKONW
};

struct error
{
	int Errortype;
	int line;
	error(){};
	error(int l, int type)
	{
		line = l;
		Errortype = type;
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
	bool report();
	void add_error(int type, int l);
};

bool errorHandler::report()
{
	if (errors.empty())
	{
		return false;
	}
	else
	{
		for (auto e : errors)
		{
			cout << "error message:line " << e.line << ",";
			switch (e.Errortype)
			{
			case DivideByZero:
				cout << "division by zero" << endl;
				break;
			case UNDECLARED_ID:
				cout << "cannot get the value of undeclared id" << endl;
				break;
			case UNDECLARED_ID_ASSIGN:
				cout << "cannot assign value to undeclared id" << endl;
				break;
			case real2int:
				cout << "realnum can not be translated into int type" << endl;
				break;

			default:
				cout << "unkown error" << endl;
			}
		}
		return true;
	}
}

void errorHandler::add_error(int type, int l)
{
	// cout << "type=" << type;
	errors.push_back(error(l, type));
}

string rules = R"(program' -> program
program -> decls compoundstmt
decls -> decl ; decls
decls -> E
decl -> int ID = INTNUM
decl -> real ID = REALNUM
stmt -> ifstmt
stmt -> assgstmt
stmt -> compoundstmt
compoundstmt -> { stmts }
stmts -> stmt stmts
stmts -> E
ifstmt -> if ( boolexpr ) then stmt else stmt
assgstmt -> ID = arithexpr ;
boolexpr -> arithexpr boolop arithexpr
boolop -> <
boolop -> >
boolop -> <=
boolop -> >=
boolop -> ==
arithexpr -> multexpr arithexprprime
arithexprprime -> + multexpr arithexprprime
arithexprprime -> - multexpr arithexprprime
arithexprprime -> E
multexpr -> simpleexpr multexprprime
multexprprime -> * simpleexpr multexprprime
multexprprime -> / simpleexpr multexprprime
multexprprime -> E
simpleexpr -> ID
simpleexpr -> INTNUM
simpleexpr -> REALNUM
simpleexpr -> ( arithexpr ))";

/**
 * @brief 获取token
 *
 * @param rule
 * @return vector<token>
 */
vector<token> get_token(string rule)
{
	vector<string> split_temp;
	vector<token> split_res;
	split_res.clear();
	string line = "";
	string word = "";
	for (auto c : rule)
	{
		if (c == '\n')
		{
			if (line != "")
				split_temp.push_back(line);
			line = "";
		}
		else
			line += c;
	}
	if (line != "")
		split_temp.push_back(line);
	for (int i = 0; i < split_temp.size(); i++)
	{
		word = "";
		for (auto c : split_temp[i])
		{
			if (c == ' ')
			{
				if (word != "")
					split_res.push_back(token(word, -1, -1, i + 1));
				word = "";
			}
			else
				word += c;
		}
		if (word != "")
			split_res.push_back(token(word, -1, -1, i + 1));
	}
	return split_res;
}

class LRtable
{
public:
	set<string> nonterminal;
	set<string> terminal;

	map<string, bool> hasNull;	 // 是否是直接生成E
	map<string, int> hasNULLnum; // 记录直接生成E的产生式的序号

	vector<vector<int>> table; // LL(1)表
	vector<vector<string>> production;
	vector<vector<entry>> SLRtable;
	map<string, bool> nullable;
	map<string, int> nullid;
	map<string, int> id; // 非终结符和终结符的id映射

	vector<set<string>> first;
	vector<set<string>> follow;
	vector<vector<item>> collection;

	void get_null();
	void get_first();
	void get_follow();
	void get_LLtable();

	vector<vector<item>> state;
	void get_SLRtable();
	void get_state();

	bool isRepeat(vector<item> p, int i);
	int state_id(vector<item> target);
	vector<item> closure(vector<item> initState);
	vector<item> get_goto(vector<item> initState, string s);
	void get_collection();

	LRtable();
};

/**
 * @brief 1. 保存非终结符和终结符，并将每个字符附一个唯一的id，用于生成预测表
 *        2. 初始化预测表，将每一个值赋为-1
 *
 */
LRtable::LRtable()
{
	vector<token> temp = get_token(rules);
	vector<string> p; // 当前的生成式
	int i = 0;
	int curline = -1;

	// 保存非终结符并保存生成式
	while (i < temp.size())
	{
		nonterminal.insert(temp[i].s);
		p.push_back(temp[i].s);
		i += 2; // 去除->
		curline = temp[i].line;
		while (i < temp.size() && curline == temp[i].line)
		{
			p.push_back(temp[i++].s);
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
	vector<entry> slrrow;
	for (int i = 0; i < terminal.size() + nonterminal.size() + 1; i++)
	{
		row.push_back(-1);
		slrrow.push_back(entry(-1, -1));
	}
	for (int i = 0; i < nonterminal.size() + 1; i++)
	{
		table.push_back(row);
	}

	for (int i = 0; i < 200; i++)
	{
		SLRtable.push_back(slrrow);
	}
}

/**
 * @brief 记录所有可能产生E的非终结符
 *
 */
void LRtable::get_null()
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
void LRtable::get_first()
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
void LRtable::get_follow()
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
 * @brief 构造LL预测表
 *
 */
void LRtable::get_LLtable()
{
	get_null();
	get_first();
	get_follow();
}

/**
 * @brief 检测目标是否已经存在item集中
 *
 * @param p
 * @param i
 * @return true
 * @return false
 */
bool LRtable::isRepeat(vector<item> p, int i)
{
	for (auto j : p)
	{
		if (j.id == i && j.index == 0)
			return true;
	}
	return false;
}

/**
 * @brief 计算闭包
 * CLOSURE(I)是这样定义的：
 * 首先I的项目都属于CLOSURE(I)；
 * 如果A->α• Bβ,则左部为B的每个产生式中的形如B->·γ项目，也属于CLOSURE(I)；
 *
 * @param initState
 * @return initState
 */
vector<item> LRtable::closure(vector<item> initState)
{
	// 将初始项目集中的项目放入队列
	deque<item> stack;
	for (auto i : initState)
	{
		stack.push_front(i);
	}

	// 处理待处理队列中的项目
	while (!stack.empty())
	{
		item head = stack.front();
		stack.pop_front();
		// 获取对应的产生式
		vector<string> p = production[head.id];
		// 检查项目的点之后是否有非终结符
		if (head.index < p.size() - 1)
		{
			string temp = p[head.index + 1];

			// 如果点之后是非终结符
			if (nonterminal.count(temp))
			{
				// 遍历该产生式，查找以该非终结符为开头的产生式并且未被添加过的
				for (int i = 0; i < production.size(); i++)
				{
					if (production[i][0] == temp && !isRepeat(initState, i))
					{
						initState.push_back(item(i, 0));
						stack.push_back(item(i, 0));
					}
				}
			}
		}
	}
	return initState;
}

/**
 * @brief GO (I,X) = CLOSURE(J)
 * 其中，I为LR(0) FSM 的状态（闭包的项目集），X为文法符号， J={ A -> αX•β | A -> α• Xβ∈I} ；
 * 表示对于一个状态项目集中的一个项目A -> α• Xβ，在下一个输入字符是X的情况下，一定到另一个新状态 A -> αX•β
 *
 * @param s
 * @return vector<item>
 */
vector<item> LRtable::get_goto(vector<item> initState, string s)
{
	vector<item> resultState;
	for (int i = 0; i < initState.size(); i++)
	{
		int id = initState[i].id;
		int index = initState[i].index;
		// 如果产生式中符号匹配了s，则将匹配的项移动一个位置
		if (index < production[id].size() - 1 && production[id][index + 1].compare(s) == 0)
		{
			resultState.push_back(item(id, index + 1));
		}
	}
	return closure(resultState);
}

/**
 * @brief 生成LR(1)分析表所需的规范集合（Canonical Collection）。
 * 规范集合包含了所有可能的项目集。
 * 使用 LR(1) 语法分析表的构建过程中会用到这个集合。
 *
 * 这个函数会根据语法产生式生成规范集合，并计算每个产生式的项目集合。
 * 首先将起始符号的项目集合放入规范集合中，然后根据项目的闭包和移进操作，逐步扩展规范集合。
 * 通过计算闭包和移进操作，生成LR(1)分析表所需的规范集合。
 *
 */
void LRtable::get_collection()
{
	// 创建起始状态并计算闭包
	vector<item> initState;
	initState.push_back(item(0, 0));
	initState = closure(initState);
	collection.push_back(initState);

	deque<vector<item>> stateStack;
	stateStack.push_front(initState);
	deque<int> idStack;
	idStack.push_front(0);

	while (!stateStack.empty())
	{
		// 获取当前状态及id
		vector<item> statehead = stateStack.front();
		stateStack.pop_front();

		int idhead = idStack.front();
		idStack.pop_front();

		// 对于每个非终结符，计算GOTO表项
		for (auto nt : nonterminal)
		{
			vector<item> temp = get_goto(statehead, nt);
			if (temp.size() != 0)
			{
				int i = state_id(temp);
				if (i == -1)
				{
					// 如果目标状态不存在，则添加至规范集合并更新状态堆栈和状态ID堆栈
					i = collection.size();
					stateStack.push_front(temp);
					idStack.push_front(i);
					collection.push_back(temp);
				}
				// 设置 GOTO 表项
				SLRtable[idhead][id[nt]] = entry(GOTO, i);
			}
		}

		// 对于每个终结符（不包括空字符），计算 SHIFT 表项
		for (auto t : terminal)
		{
			if (t == "E")
				continue;
			vector<item> temp = get_goto(statehead, t);

			if (temp.size() != 0)
			{
				int i = state_id(temp);
				if (i == -1)
				{
					// 如果目标状态不存在，则添加至规范集合并更新状态堆栈和状态ID堆栈
					i = collection.size();
					stateStack.push_front(temp);
					idStack.push_front(i);
					collection.push_back(temp);
				}
				// 设置 SHIFT 表项
				SLRtable[idhead][id[t]] = entry(SHIFT, i);
			}
		}

		// 处理 REDUCE 表项
		for (auto it : statehead)
		{
			if (it.index == production[it.id].size() - 1 ||
				(production[it.id][1] == "E" && production[it.id].size() == 2))
			{
				for (auto f : follow[id[production[it.id][0]]])
				{
					SLRtable[idhead][id[f]] = entry(REDUCE, it.id);
				}
			}
		}
	}
	// 设置 ACC 表项
	SLRtable[0][id[production[0][1]]] = entry(ACC, -1);
}

/**
 * @brief 在规范集合中查找给定状态集合（target）的状态ID
 *
 * @param target
 * @return int 存在的话返回状态id，否则返回-1
 */
int LRtable::state_id(vector<item> target)
{
	for (int i = 0; i < collection.size(); i++)
	{
		// 如果待查找的状态集合和当前规范集合中的状态集合大小不一样，跳过
		if (collection[i].size() != target.size())
			continue;
		int same = 0;
		vector<item> temp = collection[i];
		for (int j = 0; j < temp.size(); j++)
		{
			int cnt = 0;
			// 检查当前项目是否在目标状态集合中
			for (auto t : target)
			{
				if (temp[j].id == t.id && temp[j].index == t.index)
				{
					break;
				}
				cnt++;
			}
			// 如果找到相同项目，则相同项目数加一
			if (cnt < target.size())
			{
				same++;
			}
			// 如果相同项目数与当前状态集合的大小相同，则说明找到相同的状态集合，返回状态ID
			if (same == temp.size())
				return i;
		}
	}
	// 如果未找到相同的状态集合，则返回-1
	return -1;
}

/**
 * @brief 构造SLR预测表
 *
 */
void LRtable::get_SLRtable()
{
	get_LLtable();
	get_collection();
	// for(int i=0;i<50;i++){
	// 	for(int j=0;j<nonterminal.size()+terminal.size()+1;j++)
	// 	{
	// 		cout<<SLRtable[i][j].num<<" ";
	// 	}
	// 	cout<<endl;
	// }
}

class SLRparser
{
private:
	vector<token> tokens;
	deque<token> input;
	deque<token> work;
	deque<int> state;
	vector<entry> op;
	deque<string> output;
	LRtable g;
	// 创建错误处理器
	errorHandler err = errorHandler();
	vector<expr> exprs;
	map<int, int> rtype;
	map<string, double> id_value;
	map<string, int> id_type;

public:
	SLRparser(){};
	void generate_SLRtable();
	vector<entry> execute(string input_string);
	token str_to_token(string word, int ln);
	string identifyTokenType(const string &word);
	void addToOutput(int productionNum);
	string buildOutputString();
	void show();
	void translate();
	vector<token> get_token(string rule);
};

/**
 * @brief 生成预测表
 *
 */
void SLRparser::generate_SLRtable()
{
	g = LRtable();
	g.get_SLRtable();
	rtype[4] = DECLARE;
	rtype[5] = DECLARE;
	rtype[12] = IF;
	rtype[13] = ASSIGN;
	rtype[14] = BOOL;
	rtype[20] = ARITH;
	rtype[21] = ARITHPRIME;
	rtype[22] = ARITHPRIME;
	rtype[24] = ARITH;
	rtype[25] = ARITHPRIME;
	rtype[26] = ARITHPRIME;
	rtype[28] = IDVALUE;
	rtype[29] = INSTANT;
	rtype[30] = INSTANT;
	rtype[31] = ARITH;
	rtype[15] = BOOLOP;
	rtype[16] = BOOLOP;
	rtype[17] = BOOLOP;
	rtype[18] = BOOLOP;
	rtype[19] = BOOLOP;
}

/**
 * @brief 将给定单词转换为对应的 Token
 *
 * @param word
 * @param ln
 * @return token
 */
token SLRparser::str_to_token(string word, int ln)
{
	if (g.terminal.count(word)) // 检查单词是否为终结符
		return token(word, -1, -1, ln);
	else // 识别单词的类型
	{
		// 检查第一个字符是否为数字或负号后跟数字
		if (isdigit(word[0]))
		{
			vector<double> res;
			int dot_pos = word.length();
			// 判断是否为实数（包含小数点）
			for (int i = 0; i < word.length(); i++)
			{
				if (word[i] == '.')
					dot_pos = i;
			}
			double temp = 0;
			double mult = 1;
			for (int i = dot_pos - 1; i >= 0; i--)
			{
				temp += (word[i] - '0') * mult;
				mult *= 10;
			}
			res.push_back(temp);
			temp = 0;
			mult = 0.1;
			if (dot_pos != word.length())
			{
				for (int i = dot_pos + 1; i < word.length(); i++)
				{
					temp += (word[i] - '0') * mult;
					mult /= 10;
				}
				res.push_back(temp);
			}
			int ntype = (res.size() == 1) ? INT : REAL;
			if (ntype == REAL)
				return token("REALNUM", res[0] + res[1], REAL, ln);
			else
				return token("INTNUM", res[0], INT, ln);
		}
		else
			return token("ID", word, ID, ln);
	}
}

/**
 * @brief 将输入的字符串分割成带有行号信息的token,
 * 对数字判断type和value，对标识符转换为带name的ID token
 *
 * @param rule
 * @return vector<token>
 */
vector<token> SLRparser::get_token(string rule)
{
	vector<string> split_temp;
	vector<token> split_res;
	split_res.clear();
	string line = "";
	string word = "";
	for (auto c : rule)
	{
		if (c == '\n')
		{
			if (line != "")
				split_temp.push_back(line);
			line = "";
		}
		else
			line += c;
	}
	if (line != "")
		split_temp.push_back(line);
	for (int i = 0; i < split_temp.size(); i++)
	{
		word = "";
		for (auto c : split_temp[i])
		{
			if (c == ' ')
			{
				if (word != "")
				{
					split_res.push_back(str_to_token(word, i + 1));
				}
				word = "";
			}
			else
				word += c;
		}
		if (word != "")
		{
			split_res.push_back(str_to_token(word, i + 1));
		}
	}

	return split_res;
}

/**
 * @brief 执行SLR分析器
 *
 * @param input_string 输入的字符串
 * @return vector<entry> 返回操作序列
 */
vector<entry> SLRparser::execute(string input_string)
{
	// 将输入字符串转换为令牌序列
	tokens = get_token(input_string);
	for (int i = tokens.size() - 1; i >= 0; i--)
	{
		// cout<<"tokens= "<<tokens[i].s<<endl;
		input.push_front(tokens[i]);
	}

	// 在输入末尾添加结束标识符$
	input.push_back(token("$", -1, -1, -1));
	state.push_front(0); // 初始化状态栈

	token workhead;	 // 工作栈顶
	token inputhead; // 输入串的第一个符号
	int statehead;	 // 状态栈顶
	entry curop;	 // 当前操作项
	int curline = 1; // 当前行数

	int i = 0;
	// 分析过程，直到输入串为空
	while (!input.empty())
	{
		statehead = state.front();						  // 获取状态栈顶状态
		inputhead = input.front();						  // 获取输入串的第一个符号
		curop = g.SLRtable[statehead][g.id[inputhead.s]]; // 通过SLR表获取当前操作

		// cout << "type= " << curop.type << endl;
		// cout << "statehead=" << statehead << " inputhead=" << inputhead.s << endl;

		// 如果是接受状态，分析完成
		if (curop.type == ACC)
			break;

		// 移入操作
		if (curop.type == SHIFT)
		{
			work.push_front(inputhead);
			input.pop_front();
			state.push_front(curop.num);
		}

		// 规约操作
		if (curop.type == REDUCE)
		{
			vector<token> ts;
			op.push_back(curop); // 记录规约操作
			ts.push_back(token("#", -1, -1, -1));

			// 根据规约的产生式长度进行工作栈和状态栈的回退
			for (int i = 1; i < g.production[curop.num].size(); i++)
			{
				if (g.production[curop.num][i] == "E")
					continue;
				ts.push_back(work.front());
				work.pop_front();
				state.pop_front();
			}
			i++;
			// 将规约后的非终结符推入工作栈
			exprs.push_back(expr(curop, ts));
			work.push_front(token(g.production[curop.num][0], -1, -1, inputhead.line));
			entry temp = g.SLRtable[state.front()][g.id[g.production[curop.num][0]]];
			state.push_front(temp.num);
			if (temp.type == ACC)
				break;
		}

		// 处理无效操作
		if (curop.type == -1)
		{
			// cout<<"ERROR"<<endl;
			if (inputhead.s == "REALNUM")
			{
				inputhead.s = "INTNUM";
				input.pop_front();
				// cout<<"here"<<endl;
				input.push_front(inputhead);
				err.add_error(real2int, curline);
			}
		}
		curline = inputhead.line; // 更新行号
	}
	return op;
}

/**
 * @brief  将产生式右部加入输出队列
 *
 * @param productionNum
 */
void SLRparser::addToOutput(int productionNum)
{
	for (int i = 1; i < g.production[productionNum].size(); ++i)
	{
		output.push_back(g.production[productionNum][i]);
	}
}

/**
 * @brief 构建输出字符串
 *
 * @return string
 */
string SLRparser::buildOutputString()
{
	string outputString = "";
	while (!output.empty())
	{
		// 如果栈顶符号不是终结符则跳出循环
		if (!g.terminal.count(output.back()))
			break;
		// 如果栈顶符号不是空字符则添加到输出字符串中
		if (output.back() != "E")
			outputString = output.back() + " " + outputString;
		output.pop_back();
	}
	return outputString;
}

/**
 * @brief 语法制导翻译，采用Bottom-up的语法制导翻译
 *
 */
void SLRparser::translate()
{
	vector<string> ids;
	double curvalue = 0, opvalue = 0;
	bool holding = false;
	int if_true = 0;
	string compare_op;
	for (int i = 0; i < exprs.size(); i++)
	{
		vector<token> t = exprs[i].tokens;
		int exprType = rtype[exprs[i].op.num];
		switch (exprType)
		{
		case DECLARE: // 声明处理
			int type1;
			if (t[t.size() - 1].s == "int")
				type1 = INT;
			else
				type1 = REAL;
			ids.push_back(t[t.size() - 2].name);
			id_type.insert(pair<string, int>(t[t.size() - 2].name, type1));
			id_value.insert(pair<string, double>(t[t.size() - 2].name, t[t.size() - 4].value));
			break;
		case INSTANT: // 立即数处理
			if (!holding)
				curvalue = t[t.size() - 1].value;
			else
			{
				opvalue = t[t.size() - 1].value;
			}
			holding = true;
			break;
		case IDVALUE: // 标识符值处理
			if (id_value.find(t[t.size() - 1].name) == id_value.end())
				err.add_error(UNDECLARED_ID, t[t.size() - 1].line);
			if (!holding)
				curvalue = id_value[t[t.size() - 1].name];
			else
			{
				opvalue = id_value[t[t.size() - 1].name];
			}
			holding = true;
			break;
		case ASSIGN: // 赋值处理
			if (id_value.find(t[t.size() - 1].name) == id_value.end())
				err.add_error(UNDECLARED_ID_ASSIGN, t[t.size() - 1].line);
			if (if_true != -1)
			{
				id_value[t[t.size() - 1].name] = curvalue;
				holding = false;
			}
			if (if_true == 1)
				if_true = -1;
			else if (if_true == -1)
				if_true = 0;
			curvalue = 0;
			opvalue = 0;
			holding = false;
			break;
		case ARITHPRIME: // 算术运算处理
			switch (t[3].s[0])
			{
			case '+':
				curvalue += opvalue;
				break;
			case '-':
				curvalue -= opvalue;
				break;
			case '*':
				curvalue *= opvalue;
				break;
			case '/':
				if (opvalue == 0)
					err.add_error(DivideByZero, t[3].line);
				curvalue /= opvalue;
				break;
			}
			break;
		case BOOLOP: // 比较运算符处理
			compare_op = t[1].s;
			break;
		case BOOL: // 布尔运算处理
			if (compare_op == "<=")
				if_true = (curvalue <= opvalue) ? 1 : -1;
			else if (compare_op == ">=")
				if_true = (curvalue >= opvalue) ? 1 : -1;
			else if (compare_op == ">")
				if_true = (curvalue > opvalue) ? 1 : -1;
			else if (compare_op == "<")
				if_true = (curvalue < opvalue) ? 1 : -1;
			else if (compare_op == "==")
				if_true = (curvalue == opvalue) ? 1 : -1;
			curvalue = 0;
			opvalue = 0;
			holding = false;
			break;
		}
	}
	// 错误处理或输出结果
	if (!err.report())
	{
		for (auto id : ids)
		{
			std::cout << id << ": " << id_value[id] << endl;
		}
	}
}

void Analysis()
{
	string prog;
	read_prog(prog);
	/* 请开始 */
	/********* Begin *********/
	SLRparser l = SLRparser();
	l.generate_SLRtable();
	// l.show();
	l.execute(prog);
	l.translate();

	/********* End *********/
}
