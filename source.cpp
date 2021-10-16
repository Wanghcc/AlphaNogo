// Nogo
// GameInfo：http://www.botzone.org/games#NoGo
// json

#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <math.h>
#include <time.h>
#include <algorithm>
#include "jsoncpp/json.h"
#define Time_Set 1.98
#define MAXI 1000000
using namespace std;

int board[9][9] = { {0} };
bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };

class Node {
public:
	int col;
	int board[9][9];
	double UCT_value = 0.0;
	double reward = 0;
	double visit = 0;
	int child_num = 0;
	int max_child_num = 0;
	int action = 0;
	int available_list[81];
	bool leaf_node = 1;
	int EYE = -1;
	Node* child[81];
	Node* father;
	Node* best_child;
public:
	Node();
	void MCTS(Node*);
	void Rollout();
	Node* CreateChildren();
	void Backpropagation(double);
	int Evaluate();
	double UCT();
	void FindEye();
	Node* Create();
	void Back(int);
	Node* Expansion();
	bool dfs_air(int, int);
	bool judgeAvailable(int, int);
	Node* BestChild(double);
	int CheckAvailablePosition();
};
int Color = 0;
bool inBorder(int x, int y) { return x >= 0 && y >= 0 && x < 9 && y < 9; }

void Node::Back(int reward) {
	Node* p = this;
	while (p) {
		p->reward += reward;
		p->visit++;
		p = p->father;
	}
}
int Node::CheckAvailablePosition() {
	int k = 0;
	memset(available_list, -1, sizeof(available_list));
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (judgeAvailable(i, j))
			{
				//if (i == 7)
					//printf("7");
				available_list[k] = i * 9 + j;
				k++;
			}
	max_child_num = k;
	return k;
}
Node* Node::Create() {
	//Selection&Expansion
	if (max_child_num == 0)
		return this;
	if (child_num < max_child_num)
		return Expansion();
	else {
		Node* p = BestChild(0.1);
		return p->Create();
	}

}
Node* Node::BestChild(double C) {
	double MaxUCT = -50.0;
	double score = 0;
	Node* Max = nullptr;
	Node* p = nullptr;
	for (int i = 0; i < max_child_num; i++) {
		p = child[available_list[i]];
		if (p->visit == 0)
			score = MAXI;
		else
			score = p->reward / p->visit + 2 * C * sqrt(log(2 * visit) / (p->visit));
		p->UCT_value = score;
		if (score > MaxUCT) {
			MaxUCT = score;
			Max = child[available_list[i]];
		}
	}
	return Max;
}
Node* Node::Expansion() {
	Node* achild = new Node;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			achild->board[i][j] = board[i][j];
	achild->col = -col;
	achild->father = this;
	achild->CheckAvailablePosition();
	int x = available_list[child_num] / 9;
	int y = available_list[child_num] % 9;
	child[available_list[child_num++]] = achild;
	achild->board[x][y] = col;
	achild->action = x * 9 + y;
	return achild;
}
int Node::Evaluate() {
	int s1 = 0;
	int s2 = 0;
	bool f1;
	bool f2;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++) {
			if (!board[i][j]) {
				f1 = judgeAvailable(i, j);
				col = -col;
				f2 = judgeAvailable(i, j);
				col = -col;
				if (f1 && !f2)
					s1++;
				else if (!f1 && f2)
					s2++;
			}
		}
	if (col == Color)
		return s1 - s2;
	else
		return s2 - s1;
}
double Node::UCT() {
	return (double)reward / visit + 2 * 0.1 * sqrt(log((double)father->visit / visit));
}
//Selection,Expansion,Simulation,Backprogation
void Node::MCTS(Node* node) {
	Node* current_node = node;
	//Node* current_node=current;
	bool flag = 1;
	while (flag) {
		flag = 0;
		if (current_node->child_num == 0) {
			if (current_node->visit == 0) {
				current_node->Rollout();
			}
			else {
				current_node = current_node->CreateChildren();
				current_node->Rollout();
			}
		}
		else {
			current_node = current_node->BestChild(1);
			flag = 1;
		}
	}
}
void Node::FindEye() {
	int score = 0;
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (!board[i][j]) {
				for (int dir = 0; dir < 4; dir++)
				{
					int dx = i + cx[dir], dy = j + cy[dir];
					if (inBorder(dx, dy))
					{
						if (board[dx][dy] == -col)
							score++;
						else if (board[dx][dy])
							score--;
					}
					else {
						score++;
					}
				}
				if (score == 3) {
					if (child[i * 9 + j]) {
						child[i * 9 + j]->reward += 300;
						child[i * 9 + j]->visit++;
						reward += 300;
						visit++;
					}
				}
				else if (score < 0) {
					if (child[i * 9 + j]) {
						child[i * 9 + j]->reward -= 50;
						child[i * 9 + j]->visit++;
						reward -= 50;
						visit++;
					}
				}
			}
			score = 0;
		}
	}
}

void Node::Rollout() {
	CheckAvailablePosition();
	int depth = 0;
	int temp1 = max_child_num;
	int temp2 = col;
	int temp3[9][9];
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			temp3[i][j] = board[i][j];
	int position, a, x, y;
	position = max_child_num;
	while (position && depth < 10) {
		CheckAvailablePosition();
		position = max_child_num;
		if (position == 0) {
			if (col == Color)
				Backpropagation(0);
			else
				Backpropagation(1);
			break;
		}
		if (position == 0) {
			if (col == Color)
				Backpropagation(0);
			else
				Backpropagation(5);
			break;
		}
		else {
			Backpropagation(Evaluate());
		}
		a = available_list[rand() % max_child_num];
		x = a / 9;
		y = a % 9;
		board[x][y] = col;
		col = -col;
		depth++;
	}
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			board[i][j] = temp3[i][j];
	col = temp2;
	max_child_num = temp1;
	CheckAvailablePosition();
}
void Node::Backpropagation(double score) {
	Node* p = this;
	while (p) {
		p->reward += score;
		p->visit++;
		p = p->father;
	}
}
Node* Node::CreateChildren() {
	CheckAvailablePosition();
	for (int i = 0; i < max_child_num; i++) {
		child[available_list[i]] = Expansion();
		//if (child[i]->action == EYE) {
		//	child[i]->reward += 500;
		//	reward += 500;
		//}
	}
	return child[available_list[0]];
}
int main()
{
	srand((unsigned)time(0));
	string str;
	int x, y;
	// 读入JSON
	getline(cin, str);
	//getline(cin, str);
	int timeset = (int)(Time_Set * (double)CLOCKS_PER_SEC);
	int start_time = clock();
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);
	int color;
	int zero = 0;
	if (input["requests"][zero]["x"].asInt() == -1)
		color = 1;
	else
		color = -1;
	Color = color;
	// 分析自己收到的输入和自己过往的输出，并恢复棋盘状态
	int turnID = input["responses"].size();//第一步时turnID等于0，注意是responses的size，而不是request的size
	for (int i = 0; i < turnID; i++)
	{
		x = input["requests"][i]["x"].asInt(), y = input["requests"][i]["y"].asInt();//对方，注意此处是requests
		if (x != -1) board[x][y] = -color;
		x = input["responses"][i]["x"].asInt(), y = input["responses"][i]["y"].asInt();//我方，注意此处是responses
		if (x != -1) board[x][y] = color;
	}
	x = input["requests"][turnID]["x"].asInt(), y = input["requests"][turnID]["y"].asInt();//对方，注意此处是requests
	if (x != -1) board[x][y] = -color;
	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋
	//x=-1则我方是黑子
	Node root;
	root.col = color;
	root.father = nullptr;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			root.board[i][j] = board[i][j];
	root.CheckAvailablePosition();
	root.CreateChildren();
	//root.FindEye();
	//Node* current_node;
	//int score;
	//
	//while(1){
	while (clock() - start_time < timeset) {
		root.MCTS(&root);
	}
	root.FindEye();
	root.best_child = root.BestChild(1);
	int new_x = root.best_child->action / 9;
	int new_y = root.best_child->action % 9;

	// 输出决策JSON
	Json::Value ret;
	Json::Value action;
	action["x"] = new_x; action["y"] = new_y;
	ret["response"] = action;
	ret["debug"] = turnID;//调试信息可写在这里
	Json::FastWriter writer;

	cout << writer.write(ret) << endl;
	return 0;
}
Node::Node() {
	memset(board, 0, sizeof(board));
	memset(available_list, -1, sizeof(available_list));
	memset(child, 0, sizeof(child));
}
//true: has air
bool Node::dfs_air(int fx, int fy)
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] == 0)
				flag = true;
			if (board[dx][dy] == board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(dx, dy))
					flag = true;
		}
	}
	return flag;
}

//true: available
bool Node::judgeAvailable(int fx, int fy)
{
	if (board[fx][fy]) return false;//若此处已放棋子，则决不能再放棋子
	board[fx][fy] = col;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(fx, fy))
	{
		board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(dx, dy))
				{
					board[fx][fy] = 0;
					return false;
				}
		}
	}
	board[fx][fy] = 0;
	return true;
}
