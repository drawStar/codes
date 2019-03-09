#include<iostream>
#include<cstring>
#include<cstdio>
#include<map>
#include<vector>
#include<string>
#include<ctime>
#include<cmath>
#include<cstdlib>
using namespace std;

#define pi 3.1415926535897932384626433832795

bool L1_flag = 1;//L1范数,0表示L2
string version;
char buf[100000], buf1[100000];
int relation_num, entity_num;
map<string, int> relation2id, entity2id;
map<int, string> id2entity, id2relation;

map<int, map<int, int> > left_entity, right_entity;
//left_entity：在此relation下头实体对应的尾实体的个数，3个int分别表示relation_id,headentity_id,个数
//right_entity：在此relation下尾实体对应的头实体的个数，3个int分别表示relation_id,tailentity_id,个数
//主要是计算采样概率p
map<int, double> left_num, right_num;//int表示relaitonid
//leftnum：平均每个头实体对应多少个尾实体,transH的tph
//rightnum：平均每个尾实体对应多少头实体，transh的hpt


//normal distribution
double rand(double min, double max)
{//产生一个[min,max)之间的随机小数
	return min + (max - min)*rand() / (RAND_MAX + 1.0);
}
double normal(double x, double miu, double sigma)
{//高斯分布概率密度
	return 1.0 / sqrt(2 * pi) / sigma*exp(-1 * (x - miu)*(x - miu) / (2 * sigma*sigma));
}
double randn(double miu, double sigma, double min, double max)
{// 产生正态分布的随机数
	double x, y, dScope;
	do{
		x = rand(min, max);
		y = normal(x, miu, sigma);
		dScope = rand(0.0, normal(miu, miu, sigma));
	} while (dScope>y);
	return x;
}
double sqr(double x)
{
	return x*x;
}
double vec_len(vector<double>&a)
{//返回a的模
	double res = 0;
	for (int i = 0; i < a.size(); i++)
		res = res + a[i] * a[i];
	res = sqrt(res);
	return res;
}

class Train{
public:
	map<pair<int, int>, map<int, int>> ok;//4个int分别表示headID，relationID，tailid，状态
	void add(int headid, int tailid, int relationid)
	{
		fb_h.push_back(headid);
		fb_t.push_back(tailid);
		fb_r.push_back(relationid);
		ok[make_pair(headid, relationid)][tailid] = 1;
	}
	void run(int n_in, double rate_in, double margin_in, int method_in)
	{
		n = n_in; rate = rate_in; margin = margin_in;	method = method_in;
		relation_vec.resize(relation_num);
		for (int i = 0; i < relation_num; i++)
			relation_vec[i].resize(n);

		entity_vec.resize(entity_num);
		for (int i = 0; i < entity_num; i++)
			entity_vec[i].resize(n);

		relation_tmp.resize(relation_num);
		for (int i = 0; i < relation_tmp.size(); i++)
			relation_tmp[i].resize(n);
		entity_tmp.resize(entity_num);
		for (int i = 0; i < entity_tmp.size(); i++)
			entity_tmp[i].resize(n);

		for (int i = 0; i < relation_num; i++)
		{
			for (int j = 0; j < n; j++)
				relation_vec[i][j] = randn(0, 1.0 / n, -6 / sqrt(n), 6 / sqrt(n));
		}
		for (int i = 0; i < entity_num; i++)
		{
			for (int j = 0; j < n; j++)
				entity_vec[i][j] = randn(0, 1.0 / n, -6 / sqrt(n), 6 / sqrt(n));
			norm(entity_vec[i]);///限制每个实体向量的模在1以内,论文算法第5行
		}

		bfgs();
	}

private:
	int n, method;
	double rate, margin;
	double res;//?
	//double count, count1;//?
	//double belta;//?
	vector<int> fb_h, fb_t, fb_r;//fb_h保存train.txt中每行的headentity的id
	vector<vector<int> > feature;
	vector<vector<double> > relation_vec, entity_vec;//embedding?
	//相当于二维数组，第一维表示num（即id），第二维表示dim
	vector<vector<double> > relation_tmp, entity_tmp;

	double norm(vector<double>&a)
	{//L2-norm of the embeddings of the entities is 1
		double mo = vec_len(a);
		if (mo > 1)
		{
			for (int i = 0; i < a.size(); i++)
				a[i] = a[i] / mo;
		}
		return 0;
	}
	int rand_max(int x)//返回一[0,x)的整数
	{
		int j = (rand()*rand()) % x;
		while (j < 0)
			j += x;
		return j;
	}

	void bfgs()
	{
		res = 0;//loss
		int nbatches = 100;
		int nepoch = 1000;
		int batchsize = fb_h.size() / nbatches; //fb_h.size()==train.txt样本个数
		for (int epoch = 0; epoch < nepoch; epoch++)
		{
			res = 0;
			for (int batch = 0; batch < nbatches; batch++)
			{
				relation_tmp = relation_vec;
				entity_tmp = entity_vec;
				for (int k = 0; k < batchsize; k++)
				{
					int i = rand_max(fb_h.size());
					int j = rand_max(entity_num);//随机选择一entity id
					double pr = 1000 * right_num[fb_r[i]] / (right_num[fb_r[i]] + left_num[fb_r[i]]);
					//以概率pr替换三元组的尾实体，多对一关系更大概率替换尾实体
					if (method == 0)//均匀采样,将概率调为50%
						pr = 500;//若均匀采样，下面的if和else则随机选择替换头实体还是尾实体
					if (rand() % 1000 < pr)
					{//替换尾实体，注意是小于pr
						while (ok[make_pair(fb_h[i], fb_r[i])].count(j)>0)//有返回1,选择负样本尾实体
							j = rand_max(entity_num);//若train.txt中包含，则换一个尾实体
						train_kb(fb_h[i], fb_t[i], fb_r[i], fb_h[i], j, fb_r[i]);
					}
					else
					{//替换头实体
						while (ok[make_pair(j, fb_r[i])].count(fb_t[i])>0)
							j = rand_max(entity_num);
						train_kb(fb_h[i], fb_t[i], fb_r[i], j, fb_t[i], fb_r[i]);
						//计算loss，梯度下降，tmp中的值改变
					}
					//norm(relation_tmp[fb_r[i]]);我觉得这一行多余的，Transe论文中不需要限制relation
					//由于entitytmp的值改变，重新限制L2-norm of the embeddings of the entities is 1
					norm(entity_tmp[fb_h[i]]);
					norm(entity_tmp[fb_t[i]]);
					norm(entity_tmp[j]);
				}
				relation_vec = relation_tmp;
				entity_vec = entity_tmp;
			}

			cout << "epoch:" << epoch << ' ' << res << endl;

			//将向量写入文件
			FILE* f2 = fopen(("relation2vec." + version).c_str(), "w");
			FILE* f3 = fopen(("entity2vec." + version).c_str(), "w");
			for (int i = 0; i < relation_num;i++)
			{
				for (int j = 0; j<n; j++)
					fprintf(f2, "%.6lf\t", relation_vec[i][j]);
				fprintf(f2, "\n");
			}
			for (int i = 0; i < entity_num; i++)
			{
				for (int j = 0; j<n; j++)
					fprintf(f3, "%.6lf\t", entity_vec[i][j]);
				fprintf(f3, "\n");
			}
			fclose(f2);
			fclose(f3);
		}
	}

	double calc_sum(int h, int t, int r)
	{//计算h+r-t
		double sum = 0;
		if (L1_flag)
		for (int i = 0; i < n; i++)
			sum += fabs(entity_vec[h][i] + entity_vec[r][i] - entity_vec[t][i]);//(h+r-t)L1
		else
		for (int i = 0; i < n; i++)
			sum += sqr(entity_vec[h][i] + entity_vec[r][i] - entity_vec[t][i]);//(h+r-t)欧式距离的平方
		return sum;
	}
	void gardient(int h_a, int t_a, int r_a, int h_b, int t_b, int r_b)
	{//Loss=|h+r-t|L12+margin-|h+r-t|L12
		for (int i = 0; i < n; i++)
		{
			//计算正样本梯度
			double x = 2 * (entity_vec[h_a][i] + entity_vec[r_a][i] - entity_vec[t_a][i]);
			//这里计算的L2范数的平方
			if (L1_flag)//以绝对值作为loss
			{
				if (x>0) x = 1;//（h+r-t>0）
				else x = -1;
			}
			relation_tmp[r_a][i] -= rate*x;
			entity_tmp[h_a][i] -= rate*x;
			entity_tmp[t_a][i] -= rate*x*(-1);

			//计算负样本梯度
			x = 2 * (entity_vec[h_b][i] + entity_vec[r_b][i] - entity_vec[t_b][i]);
			if (L1_flag)//绝对值作为loss
			{
				if (x>0) x = 1;//（h+r-t>0）
				else x = -1;
			}
			relation_tmp[r_b][i] -=-1*rate*x;//注意要乘-1
			entity_tmp[h_b][i] -= -1*rate*x;
			entity_tmp[t_b][i] -= -1*rate*x*(-1);
		}
	}

	void train_kb(int h_a,int t_a,int r_a,int h_b,int t_b,int r_b)
	{//计算loss，梯度下降
	 //a是正样本，b是负样本
		double posLoss = calc_sum(h_a, t_a, r_a);
		double negLoss = calc_sum(h_b, t_b, r_b);
		if (posLoss + margin - negLoss > 0)
		{//<=0时，loss=0
			res += margin + posLoss - negLoss;
			gardient( h_a,  t_a,  r_a,  h_b,  t_b,  r_b);//更新梯度
		}
	}
};

Train train;
void prepare()
{
	int mycount=0;//记录读取进度
	FILE* f1 = fopen("../data/FB15k/entity2id.txt", "r");
	FILE* f2 = fopen("../data/FB15k/relation2id.txt", "r");
	int x;
	while (fscanf(f1, "%s%d", buf, &x) == 2)//==2指的是正确读入的参数个数
	{
		mycount++;
		if (mycount % 200 == 0)
			cout << "读取第"<<mycount<<"个entity" << endl;
		string st = buf;
		entity2id[st] = x;
		id2entity[x] = st;
		entity_num++;
	}
	mycount = 0;
	while (fscanf(f2, "%s%d", buf, &x) == 2)
	{
		mycount++;
		if (mycount % 200 == 0)
			cout << "读取第" << mycount << "个relation" << endl;
		string st = buf;
		relation2id[st] = x;
		id2relation[x] = st;
		relation_num++;
	}
	mycount = 0;
	FILE* f_kb = fopen("../data/FB15k/train.txt", "r");
	while (fscanf(f_kb, "%s", buf) == 1)
	{
		mycount++;
		if (mycount % 1000 == 0)
			cout << "读取第" << mycount << "个train样本" << endl;
		string s1 = buf;//entity1
		fscanf(f_kb, "%s", buf);
		string s2 = buf;//entity2
		fscanf(f_kb, "%s", buf);
		string s3 = buf;//relation
		if (entity2id.count(s1) == 0)
			cout << "miss entity:" << s1 << endl;
		if (entity2id.count(s2) == 0)
			cout << "miss entity:" << s2 << endl;
		if (relation2id.count(s3) == 0)
		{//若缺少此relation，补上
			relation2id[s3] = relation_num;
			relation_num++;
		}
		left_entity[relation2id[s3]][entity2id[s1]]++;
		right_entity[relation2id[s3]][entity2id[s2]]++;
		train.add(entity2id[s1], entity2id[s2], relation2id[s3]);//将h,r,t联系起来
	}
	for (int i = 0; i < relation_num; i++)
	{
		double sum1 = 0, sum2 = 0;
		for (map<int, int>::iterator it = left_entity[i].begin(); it != left_entity[i].end(); it++)

		{
			sum1++;//此relation下不同的headentity的个数
			sum2 = sum2 + it->second;
		}
		left_num[i] = sum2 / sum1;
	}
	for (int i = 0; i < relation_num; i++)
	{
		double sum1 = 0, sum2 = 0;
		for (map<int, int>::iterator it = right_entity[i].begin(); it != right_entity[i].end(); it++)
		{
			sum1++; sum2 = sum2 + it->second;
		}
		right_num[i] = sum2 / sum1;
	}

	cout << "relation_num=" << relation_num << endl;
	cout << "entity_num=" << entity_num << endl;
	fclose(f_kb);
}

int ArgPos(char *str, int argc, char **argv)
{
	int i;
	for ( i = 1; i < argc; i++)
	{
		if (!strcmp(str, argv[i]))//若两者相同
		{
			if (i == argc - 1)
			{
				cout << "Argument missing for " << str << endl;
				exit(1);
			}
			return i;
		}
	}
	return -1;
}
int main(int argc, char **argv)
{
	//运行方法
	//D:\codes\vs\kb2e\Debug>kb2e.exe -size 111 -margin 22 -method 3
	//上述argc=7

	srand((unsigned)time(NULL));//提供随机数种子
	int method = 1;//1表示伯努利采样
	int n = 100;//dim
	double rate = 0.001;//lr
	double margin = 1;
	int i;

	if ((i = ArgPos((char *)"-size", argc, argv)) > 0) n = atoi(argv[i + 1]);//atoi 字符串转int
	if ((i = ArgPos((char *)"-margin", argc, argv)) > 0) margin = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-method", argc, argv)) > 0) method = atoi(argv[i + 1]);
	cout << "dim="<<n << "margin=" << margin;
	if (method)
		version = "bern";//伯努利采样，详见transh
	else
		version = "unif";//均匀采样
	cout << "method = " << version << endl;
	prepare();
	train.run(n, rate, margin, method);
	return 0;
}
