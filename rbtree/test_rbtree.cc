#include "rbtree.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <set>
#include <vector>
#include <string>

#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>

void test_init(struct timespec& timestamp)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, &timestamp);
}
double test1(struct timespec& timestamp)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	double s = ts.tv_sec - timestamp.tv_sec;
	double n = ts.tv_nsec - timestamp.tv_nsec;
	return (s * 1000000000 + n)/1000000 ;
}

struct watch_infos
{
	struct winsize ws;
	std::vector<std::vector<std::string> > strs;
};

std::string nspace(int k,char c=' ')
{
	std::string str;
	while(k > 0){
		str.push_back(c);
		--k;
	}
	return str;
}

void lsbtree_watch_test(lrbtree_node* node,int depth,int idx,void* userdata)
{
	struct watch_infos* wi = (struct watch_infos*)userdata;
	struct winsize* ws = &wi->ws;

	//	每行数据数量
	int datacount = pow(2,depth);

	//	每行空隙数量
	int sp_count = datacount + 1;

	//	每行空隙宽度
	int sp_width = (ws->ws_col - datacount * 3) / sp_count;

	//	这一行需要的展示行数
	int linecount = (depth + 1) * 2;

	if(linecount > wi->strs.size()){
		std::vector<std::string> t;
		t.resize(sp_count + datacount);

		for(int i=0;i<t.size();++i){
			if(i % 2 == 0){
				t[i] = nspace(sp_width);
			}else{
				t[i] = "   ";
			}
		}

		wi->strs.push_back(t);
		wi->strs.push_back(t);
	}

	//	前面节点数*每节点数据宽度	+	(前面节点数+1)*分片宽度
	int offset = idx * 12 + (idx+1)*sp_width;

	char buff[256];
	if(node->color == 0){
		snprintf(buff,256,"\x1b[32m%s\x1b[0m",node->data);
	}else{
		snprintf(buff,256,"\x1b[31m%s\x1b[0m",node->data);
	}

	wi->strs[linecount - 2][idx+(idx+1)] = buff;

	if(node->left){
		wi->strs[linecount - 1][idx+(idx+1)][0]	=	'/';
	}
	if(node->right){
		wi->strs[linecount - 1][idx+(idx+1)][2] = '\\';
	}
}



int stl_times = 0;
int ltree_times = 0;

int str_compator(const void* a,const void* b)
{
	++ltree_times;
	return strcmp((const char*)a,(const char*)b);
}

class str_less_compator{
  public:
	bool operator()(const char* a,const char* b) const
	{
		++stl_times;
		return strcmp((const char*)a,(const char*)b) < 0;
	}
};

int int_compator(const void* a,const void* b)
{
	if(a > b) return 1;
	if(a < b) return -1;
	return 0;
}



void tree_verbose_test(struct lrbtree_ctx* ctx)
{
	watch_infos wi;
	ioctl(0, TIOCGWINSZ, &wi.ws);
	lrbtree_verbose(ctx,lsbtree_watch_test,&wi);
	std::vector<std::vector<std::string> >::const_iterator it = wi.strs.begin();
	for(int i=0;it != wi.strs.end();++it,++i){
		std::vector<std::string>::const_iterator it2 = it->begin();
		for(;it2!=it->end();++it2){
			printf(it2->c_str());
		}
		printf("\n");
	}
}


int main(int argc,const char* argv[])
{
	std::vector<char*> vec;
	std::vector<char*> vec_del;
	std::vector<char*> vec_del_add2;
	{
		/**/
		srand(time(0));

		int dl = rand() % 100;
		int dal = rand() % dl;

		for(int i=0;i<100000;++i){
			char *buff = new char[256];
			snprintf(buff,256,"%04u%04u%04u%04u",rand()%10000,rand()%10000,rand()%10000,rand()%10000);
			vec.push_back(buff);
			if(rand() % 100 < dl){
				// 删除百分比
				vec_del.push_back(buff);
				if(rand() % 100 < dal){
					vec_del_add2.push_back(buff);
				}
			}
		}
		
		/*/
		for(int i=1;i<10;++i){
			char *buff = new char[256];
			snprintf(buff,256,"%03llu",i);
			vec.push_back(buff);
		}
		/**/
	}


	struct lrbtree_ctx ctx;
	{
		// 初始化lrb
		struct winsize ws;
		ioctl(0, TIOCGWINSZ, &ws);
		ctx.compator = str_compator;
		lrbtree_init(&ctx);
	}


	///// 写入
	std::set<char*,str_less_compator> qs;
	{
		// set写入
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.insert(*it);
		}
		printf("set插入耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		//	lrb写入
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			if(lrbtree_insert(&ctx,(const void*)*it,nullptr) == lrbtree_fail){
				printf("发生了break\n");
				break;
			}
			//printf("插入%s%s\n",*it,nspace(ws.ws_col - 7,'=').c_str());
			//tree_verbose_test(&ctx);

		}
		printf("lrb插入耗时%f,大小=%d\n",test1(timestamp),ctx.size);
	}


	////	读取
	{
		// set读取
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.find(*it);
		}
		printf("set读取耗时%f,size=%d\n",test1(timestamp),qs.size());
	}
	{
		// lrb读取
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			const void* data;
			if(lrbtree_get(&ctx,(const void*)*it,&data) != lrbtree_ok){
				printf("发生了break\n");
				break;
			}

			if(data != *it){
				printf("读取错误\n");
			}
		}
		printf("lrb读取耗时%f,size=%d\n",test1(timestamp),qs.size());
	}





	////	删除
	{
		// set删除
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec_del.begin();it!=vec_del.end();++it){
			qs.erase(*it);
		}
		printf("set删除耗时%f,size=%d\n",test1(timestamp),qs.size());
	}
	{
		// lrb删除
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec_del.begin();it!=vec_del.end();++it){
			const void* data;
			if(lrbtree_remove(&ctx,(const void*)*it,&data) != lrbtree_ok){
				printf("发生了break\n");
				break;
			}

			if(data != *it){
				printf("删除错误\n");
			}
		}
		printf("lrb删除耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	///// 删除后再次写入
	{
		// set写入
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.insert(*it);
		}
		printf("set插入耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		//	lrb写入
		timespec timestamp;
		test_init(timestamp);

		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			if(lrbtree_insert(&ctx,(const void*)*it,nullptr) == lrbtree_fail){
				printf("发生了break\n");
				break;
			}
			//printf("插入%s%s\n",*it,nspace(ws.ws_col - 7,'=').c_str());
			//tree_verbose_test(&ctx);

		}
		printf("lrb插入耗时%f,大小=%d\n",test1(timestamp),ctx.size);
	}








	////	迭代
	{
		// set迭代
		timespec timestamp;
		test_init(timestamp);

		for(std::set<char*,str_less_compator>::const_iterator it = qs.begin();it!=qs.end();++it){
			
		}
		printf("set迭代耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		// lrb迭代
		timespec timestamp;
		test_init(timestamp);

		struct lrbtree_iter* iter;

		lrbtree_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			while(lrbtree_scan_next(iter,&data) == lrbtree_ok){
				//printf("结果%s\n",(const char*)data);
			}
			lrbtree_scan_destory(iter);
		}

		printf("lrb迭代耗时%f,size=%d\n",test1(timestamp),qs.size());
	}

	/// 验证迭代器
	{
		bool iserror = false;
		struct lrbtree_iter* iter;

		lrbtree_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			std::set<char*,str_less_compator>::const_iterator it = qs.begin();
			while(lrbtree_scan_next(iter,&data) == lrbtree_ok){
				//printf("结果%s\n",(const char*)data);
				if(data != *it){
					printf("\x1b[31m正向验证迭代器错误\x1b[0m\n");
					iserror = true;
					break;
				}

				++it;
			}
			lrbtree_scan_destory(iter);
		}
		if(!iserror){
			printf("正向验证迭代器成功\n");
		}

	}
	{
	
		bool iserror = false;
		struct lrbtree_iter* iter;

		lrbtree_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			std::set<char*,str_less_compator>::const_reverse_iterator it = qs.rbegin();
			while(lrbtree_scan_last(iter,&data) == lrbtree_ok){
				//printf("结果%s\n",(const char*)data);
				if(data != *it){
					printf("\x1b[31m反向验证迭代器错误\x1b[0m\n");
					iserror = true;
					break;
				}

				++it;
			}
			lrbtree_scan_destory(iter);
		}
		if(!iserror){
			printf("反向验证迭代器成功\n");
		}

	}




	{
	
		struct lrbtree_iter* iter;
		struct lrbtree_ctx ctx;
		ctx.compator = str_compator;
		lrbtree_init(&ctx);

		lrbtree_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			//lrbtree_scan_seek(iter,"a0000000",&data);
			//printf("结果%s\n",(const char*)data);



			while(lrbtree_scan_next(iter,&data) == lrbtree_ok){
				//printf("结果%s\n",(const char*)data);
			}
			lrbtree_scan_destory(iter);
		}
	}




	lrbtree_destory(&ctx);
	{
		for(std::vector<char*>::const_iterator it = vec.begin();it!=vec.end();++it){
			delete[] *it;
		}
	}
	printf("销毁完成,stl比较次数=%d,lrbtree比较次数=%d\n",stl_times,ltree_times);
	return 0;
}
