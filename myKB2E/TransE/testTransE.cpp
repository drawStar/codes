#include<iostream>
#include<cstring>
#include<cstdio>
#include<map>
#include<vector>
#include<string>
#include<ctime>
#include<algorithm>
#include<cmath>
#include<cstdlib>
using namespace std;

//bool debug = false;
bool L1_flag = 1;//L1范数,0表示L2
string version;
char buf[100000], buf1[100000];
int entity_num, relation_num;
int n = 100;
map<string, int> relation2id, entity2id;
map<int, string>id2relation, id2entity;

double vec_len(vector<double>a)
{
	double res = 0;
	for (int i = 0; i < a.size(); i++)
		res += a[i]*a[i];
	return sqrt(res);
}
double sqr(double x)
{
	return x*x;
}
double cmp(pair<int, double> a, pair<int, double>b)
{//从小到大排序
	return a.second < b.second;
}
class Test{

	vector<vector<double>> relation_vec, entity_vec;
	//vector<int>h, r, t;
	vector<int>fb_h, fb_r, fb_t;
	map<pair<int, int>, map<int, int> >ok;
	double res;

public:
	void add(int h, int t, int r, bool flag)
	{
		if (flag)
		{
			fb_h.push_back(h);
			fb_t.push_back(t);
			fb_r.push_back(r);
			ok[make_pair(h, r)][t] = 1;
		}
	}
	double cal_sum(int h, int t, int r)
	{//KB2E计算的负值，想不通
		double sum = 0;
		if (L1_flag)//L1
		for (int i = 0; i < n; i++)
			sum += fabs(entity_vec[h][i] + relation_vec[r][i] - entity_vec[t][i]);
		else
		for (int i = 0; i < n; i++)
			sum += sqr(entity_vec[h][i] + relation_vec[r][i] - entity_vec[t][i]);
		return sum;
	}
	int rand_max(int x)
	{
		int res = (rand()*rand()) % x;
		if (res<0)
			res += x;
		return res;
	}
	void run()
	{
		FILE* f1 = fopen(("relation2vec." + version).c_str(), "r");
		FILE* f3 = fopen(("entity2vec." + version).c_str(), "r");
		cout <<"relation_num="<< relation_num << ', ' << "entity_num="<<entity_num << endl;

		relation_vec.resize(relation_num);//relation_num应该比relation_vec的size要大吧
		for (int i = 0; i < relation_num; i++)
		{//读取文件中的relation embedding，保存到relation_vec中
			relation_vec[i].resize(n);
			for (int j = 0; j < n; j++)
				fscanf(f1, "%lf", &relation_vec[i][j]);
		}

		entity_vec.resize(entity_num);
		for (int i = 0; i < entity_num; i++)
		{//读取文件中的entity embedding，保存到entity_vec中
			entity_vec[i].resize(n);
			for (int j = 0; j < n; j++)
				fscanf(f3, "%lf", &entity_vec[i][j]);
			if (vec_len(entity_vec[i]) - 1>1e-3)
				cout << "wrong_entity" << i << ' ' << vec_len(entity_vec[i]) << endl;
		}
		fclose(f1); fclose(f3);
		//map<int, int> rel_num;//relationid,number

		double hrank = 0,hrank_filter=0;//替换头实体排名
		double hrank10num = 0,hrank10numfilter=0;//替换头实体rank10
		double m = 0;//正确样本个数 用于filter

		for (int testid = 0; testid < fb_h.size(); testid++)
		{//对test.txt中的每一行
			int h = fb_h[testid];//head_entity id
			int t = fb_t[testid];
			int rel = fb_r[testid];
			//rel_num[rel] += 1;
			vector<pair<int, double>>a;//head_entityid,score
			for (int i = 0; i < entity_num; i++)
			{
				double score=cal_sum(i, t, rel);//头实体被每个实体替代
				a.push_back(make_pair(i, score));
			}
			sort(a.begin(), a.end(), cmp);//升序排序


			m = 0;
			for (int i = a.size() - 1; i >= 0; i--)
			{
				if (ok[make_pair(a[i].first, rel)].count(t) > 0)//存在正确样本
					m++;

				if (a[i].first == h)//正确样本
				{
					hrank += a.size() - i;//raw 排名
					hrank_filter += a.size() - i - m;
					if (a.size() - i < 10)
						hrank10num += 1;
					if (a.size() - i - m < 10)
						hrank10numfilter += 1;

				}
			
			}
			a.clear();
		}
		cout << "替换头实体raw mean rank=" << hrank / fb_h.size();
		cout << "替换头实体raw rank10=" << hrank10num / fb_h.size();
		cout << "替换头实体filter mean rank=" << hrank_filter / fb_h.size();
		cout << "替换头实体filter rank10=" << hrank10numfilter / fb_h.size();


	}

};

Test test;
void prepare()
{
	FILE* f1 = fopen("../data/FB15k/entity2id.txt","r");
	FILE* f2 = fopen("../data/FB15k/relation2id.txt", "r");
	int x;
	while (fscanf(f1, "%s%d", buf, &x) == 2)
	{
		string s = buf;
		entity2id[s] = x;
		id2entity[x] = s;
		entity_num++;
	}
	while (fscanf(f2, "%s%d", buf, &x) == 2)
	{
		string s = buf;
		relation2id[s] = x;
		id2entity[x] = s;
		relation_num++;
	}
	FILE* f_kb = fopen("../data/FB15k/test.txt", "r");
	while (fscanf(f_kb, "%s", buf) == 1)
	{
		string s1 = buf;//h
		fscanf(f_kb, "%s", buf);
		string s2 = buf;//t
		fscanf(f_kb, "%s", buf);
		string s3 = buf;//r
		if (entity2id.count(s1)==0)
			cout << "miss entity:" << s1 << endl;
		if (entity2id.count(s2) == 0)
			cout << "miss entity:" << s2 << endl;
		if (relation2id.count(s3) == 0)
		{
			cout << "miss relation:" << s3 << endl;
			relation2id[s3] = relation_num;
			relation_num++;
		}
		test.add(entity2id[s1],entity2id[s2],entity2id[s3],true);
		
	}
	fclose(f_kb);

	FILE* f_kb1 = fopen("../data/FB15k/train.txt", "r");
	while (fscanf(f_kb1, "%s", buf) == 1)
	{
		string s1 = buf;
		fscanf(f_kb1, "%s", buf);
		string s2 = buf;
		fscanf(f_kb1, "%s", buf);
		string s3 = buf;
		if (entity2id.count(s1) == 0)
			cout << "miss entity:" << s1 << endl;
		if (entity2id.count(s2) == 0)
			cout << "miss entity:" << s2 << endl;
		if (relation2id.count(s3) == 0)
		{
			relation2id[s3] = relation_num;
			relation_num++;
		}
		test.add(entity2id[s1], entity2id[s2], entity2id[s3], true);
		//应该为true，论文中提到移除train，val，test“错误”三元组
	}
	fclose(f_kb1);

	FILE* f_kb2 = fopen("../data/FB15k/valid.txt", "r");
	while (fscanf(f_kb2, "%s", buf) == 1)
	{
		string s1 = buf;
		fscanf(f_kb2, "%s", buf);
		string s2 = buf;
		fscanf(f_kb2, "%s", buf);
		string s3 = buf;
		if (entity2id.count(s1) == 0)
		{
			cout << "miss entity:" << s1 << endl;
		}
		if (entity2id.count(s2) == 0)
		{
			cout << "miss entity:" << s2 << endl;
		}
		if (relation2id.count(s3) == 0)
		{
			relation2id[s3] = relation_num;
			relation_num++;
		}
		test.add(entity2id[s1], entity2id[s2], relation2id[s3], true);
		//应该为true，论文中提到移除train，val，test“错误”三元组
	}
	fclose(f_kb2);
}

int main(int argc,  char** argv)
{
	if (argc < 2)
		return 0;
	else
	{
		version = argv[1];
		prepare();
		test.run();
	}
}