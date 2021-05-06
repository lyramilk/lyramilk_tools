#include "rbtree2.h"
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

int lsbtree_watch_test(lrbtree2_node* node,int depth,int idx,void* userdata)
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
	int linecount = (depth + 1) * 3;

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
		wi->strs.push_back(t);
	}

	//	前面节点数*每节点数据宽度	+	(前面节点数+1)*分片宽度
	int offset = idx * 12 + (idx+1)*sp_width;

	char buff[256],buff2[256];
	if(node->color == 0){
		snprintf(buff,256,"\x1b[32m%s\x1b[0m",node->data);
		snprintf(buff2,256,"%d",node->cc);
	}else{
		snprintf(buff,256,"\x1b[31m%s\x1b[0m",node->data);
		snprintf(buff2,256,"%d",node->cc);
	}

	wi->strs[linecount - 3][idx+(idx+1)] = buff;
	wi->strs[linecount - 2][idx+(idx+1)] = buff2;

	if(node->left){
		wi->strs[linecount - 1][idx+(idx+1)][0]	=	'/';
	}
	if(node->right){
		wi->strs[linecount - 1][idx+(idx+1)][2] = '\\';
	}

	return 1;
}



int stl_times = 0;
int ltree_times = 0;


/**/
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
/*/

int str_compator(const void* a,const void* b)
{
	++ltree_times;
	long la = (long)a;
	long lb = (long)b;
	if(la > lb) return 1;
	if(la < lb) return -1;
	return 0;
}

class str_less_compator{
  public:
	bool operator()(long a,long b) const
	{
		++stl_times;
		return a < b;
	}
};

/**/



int int_compator(const void* a,const void* b)
{
	long la = (long)a;
	long lb = (long)b;
	if(la > lb) return 1;
	if(la < lb) return -1;
	return 0;
}





int tree_verbose_test(struct lrbtree2_ctx* ctx)
{
	watch_infos wi;
	ioctl(0, TIOCGWINSZ, &wi.ws);
	lrbtree2_verbose(ctx,lsbtree_watch_test,&wi);
	std::vector<std::vector<std::string> >::const_iterator it = wi.strs.begin();
	for(int i=0;it != wi.strs.end();++it,++i){
		std::vector<std::string>::const_iterator it2 = it->begin();
		for(;it2!=it->end();++it2){
			printf(it2->c_str());
		}
		printf("\n");
	}
	return 1;
}

struct verify_info
{
	bool r;
	lrbtree2_node* node;
};

int lsbtree_watch_verifycc(lrbtree2_node* node,int depth,int idx,void* userdata)
{
	long k = 1;
	if(node->left){
		k += node->left->cc;
	}
	if(node->right){
		k += node->right->cc;
	}
	if(k != node->cc){
		verify_info* p = (verify_info*)userdata;
		p->r = true;
		p->node = node;
printf("节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错节点出错=%p\n",node);
		return 0;
	}

	return 1;
}

verify_info tree_verify_cc(struct lrbtree2_ctx* ctx)
{
	verify_info vi;
	vi.r = false;
	vi.node = nullptr;
	lrbtree2_verbose(ctx,lsbtree_watch_verifycc,&vi);
	return vi;
}

int main(int argc,const char* argv[])
{

#if 1
	typedef char* item_type;
	typedef std::vector<item_type> vec_type;

	vec_type vec;
	vec_type vec_del;
	vec_type vec_del_add2;
	{
		/**/
		srand(time(0));

		int dl = rand() % 100;
		int dal = rand() % dl;

		for(int i=0;i<1000000;++i){
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
		for(int i=1;i<12;++i){
			char *buff = new char[256];
			snprintf(buff,256,"%03llu",i);
			vec.push_back(buff);
		}
		/**/
	}
#else
	typedef long item_type;
	typedef std::vector<item_type> vec_type;

	vec_type vec;
	vec_type vec_del;
	vec_type vec_del_add2;
	{
		srand(time(0));

		int dl = rand() % 100;
		int dal = rand() % dl;

		union{
			struct
			{
				unsigned char c1;
				unsigned char c2;
				unsigned char c3;
				unsigned char c4;
				unsigned char c5;
				unsigned char c6;
				unsigned char c7;
				unsigned char c8;
			};
			long i;
		}u;


		for(int i=0;i<1000000;++i){
			u.c1 = rand()%0xff;
			u.c2 = rand()%0xff;
			u.c3 = rand()%0xff;
			u.c4 = rand()%0xff;
			u.c5 = rand()%0xff;
			u.c6 = rand()%0xff;
			u.c7 = rand()%0xff;
			u.c8 = rand()%0xff;

			vec.push_back(u.i);
			if(rand() % 100 < dl){
				// 删除百分比
				vec_del.push_back(u.i);
				if(rand() % 100 < dal){
					vec_del_add2.push_back(u.i);
				}
			}
		}
	}
#endif

	struct lrbtree2_ctx ctx;
	struct winsize ws;
	{
		// 初始化lrb
		ioctl(0, TIOCGWINSZ, &ws);
		ctx.compator = str_compator;
		lrbtree2_init(&ctx);
	}


	///// 写入
	std::set<item_type,str_less_compator> qs;
	{
		// set写入
		timespec timestamp;
		test_init(timestamp);

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.insert(*it);
		}
		printf("set插入耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		//	lrb写入
		timespec timestamp;
		test_init(timestamp);

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			if(lrbtree2_insert(&ctx,(const void*)*it,nullptr) == lrbtree2_fail){
				printf("\x1b[32m出错了\x1b[0m");
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

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.find(*it);
		}
		printf("set读取耗时%f,size=%d\n",test1(timestamp),qs.size());
	}
	{
		// lrb读取
		timespec timestamp;
		test_init(timestamp);

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			const void* data;
			if(lrbtree2_get(&ctx,(const void*)*it,&data) != lrbtree2_ok){
				printf("发生了break\n");
				break;
			}

			if((item_type)data != *it){
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

		for(vec_type::const_iterator it = vec_del.begin();it!=vec_del.end();++it){
			qs.erase(*it);
		}
		printf("set删除耗时%f,size=%d\n",test1(timestamp),qs.size());
	}
	{
		// lrb删除
		timespec timestamp;
		test_init(timestamp);

		for(vec_type::const_iterator it = vec_del.begin();it!=vec_del.end();++it){
			const void* data;
			if(lrbtree2_remove(&ctx,(const void*)*it,&data) != lrbtree2_ok){
				printf("发生了break\n");
				break;
			}

			if((item_type)data != *it){
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

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			qs.insert(*it);
		}
		printf("set插入耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		//	lrb写入
		timespec timestamp;
		test_init(timestamp);

		for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			if(lrbtree2_insert(&ctx,(const void*)*it,nullptr) == lrbtree2_fail){
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

		for(std::set<item_type,str_less_compator>::const_iterator it = qs.begin();it!=qs.end();++it){
			
		}
		printf("set迭代耗时%f,size=%d\n",test1(timestamp),qs.size());
	}



	{
		// lrb迭代
		timespec timestamp;
		test_init(timestamp);

		struct lrbtree2_iter* iter;

		lrbtree2_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			while(lrbtree2_scan_next(iter,&data) == lrbtree2_ok){
				//printf("结果%s\n",(const char*)data);
			}
			lrbtree2_scan_destory(iter);
		}

		printf("lrb迭代耗时%f,size=%d\n",test1(timestamp),qs.size());
	}

	long tstl_times = stl_times;
	long tltree_times = ltree_times;

	/// 验证迭代器
	{
		bool iserror = false;
		struct lrbtree2_iter* iter;

		lrbtree2_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			std::set<item_type,str_less_compator>::const_iterator it = qs.begin();
			while(lrbtree2_scan_next(iter,&data) == lrbtree2_ok){
				if((item_type)data != *it){
					printf("\x1b[31m正向验证迭代器错误\x1b[0m\n");
					iserror = true;
					break;
				}

				++it;
			}
			lrbtree2_scan_destory(iter);
		}
		if(!iserror){
			printf("正向验证迭代器成功\n");
		}

	}
	{
	
		bool iserror = false;
		struct lrbtree2_iter* iter;

		lrbtree2_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			std::set<item_type,str_less_compator>::const_reverse_iterator it = qs.rbegin();
			while(lrbtree2_scan_last(iter,&data) == lrbtree2_ok){
				//printf("结果%s\n",(const char*)data);
				if((item_type)data != *it){
					printf("\x1b[31m反向验证迭代器错误\x1b[0m\n");
					iserror = true;
					break;
				}

				++it;
			}
			lrbtree2_scan_destory(iter);
		}
		if(!iserror){
			printf("反向验证迭代器成功\n");
		}

	}





	{
	
		struct lrbtree2_iter* iter;
		struct lrbtree2_ctx ctx;
		ctx.compator = str_compator;
		lrbtree2_init(&ctx);

		lrbtree2_scan_init(&ctx,&iter);
		if(iter){
			const void* data;
			//lrbtree2_scan_seek(iter,"a0000000",&data);
			//printf("结果%s\n",(const char*)data);



			while(lrbtree2_scan_next(iter,&data) == lrbtree2_ok){
				//printf("结果%s\n",(const char*)data);
			}
			lrbtree2_scan_destory(iter);
		}
	}

	vec_type vec_ordered;
	{
		for(std::set<item_type,str_less_compator>::const_iterator it = qs.begin();it!=qs.end();++it){
			vec_ordered.push_back(*it);
		}
	}
	#define RANK_TIMES 20

	int seed = time(nullptr);

	{
		timespec timestamp;
		test_init(timestamp);

		srand(seed);
		for(int i=0;i<RANK_TIMES;++i){
			long rank = rand() % vec_ordered.size();
			long idx = 0;
			std::set<item_type,str_less_compator>::const_iterator it = qs.begin();

			for(long idx = 0;idx < rank;++idx){
				++it;
			}
			if(*it != vec_ordered[rank]){
				printf("set rank操作结果错误,第%d个为%s，应该为%s\n",rank,*it,vec_ordered[rank]);
				break;
			}
		}

		printf("set rank操作耗时%f,size=%d\n",test1(timestamp),qs.size());
	}


	{
		const void* data;
		struct lrbtree2_iter* iter;
		timespec timestamp;
		test_init(timestamp);

		lrbtree2_scan_init(&ctx,&iter);
		srand(seed);
		for(int i=0;i<RANK_TIMES;++i){
			long rank = rand() % vec_ordered.size();

			if(lrbtree2_scan_seek_rank(iter,rank,&data) != lrbtree2_ok){
				printf("lrbtree2 rank操作错误%d\n",rank);
				break;
			}
			if((item_type)data != vec_ordered[rank]){
				printf("lrbtree2 rank操作结果错误,第%d个为%s，应该为%s\n",rank,data,vec_ordered[rank]);
				break;
			}
		}
		lrbtree2_scan_destory(iter);

		printf("lrbtree2 rank操作耗时%f,size=%d\n",test1(timestamp),qs.size());
	}


	printf("======== 验证rank操作 ========\n");
	//tree_verbose_test(&ctx);
	{
		const void* data;
		struct lrbtree2_iter* iter;
		timespec timestamp;
		test_init(timestamp);

		lrbtree2_scan_init(&ctx,&iter);
		for(int rank=0;rank<vec_ordered.size();++rank){
			long rank2;

			lrbtree2_rank(&ctx,(const void*)vec_ordered[rank],&rank2);

			if(rank2 != rank){
				printf("lrbtree2 rank操作结果错误,%s应该在%d而不是%d\n",data,rank,rank2);
				break;
			}
		}
		lrbtree2_scan_destory(iter);

		printf("lrbtree2 rank验证耗时%f,size=%d\n",test1(timestamp),qs.size());
	}


	{
		/*for(vec_type::const_iterator it = vec.begin();it!=vec.end();++it){
			delete[] *it;
		}*/
		printf("销毁完成,stl比较次数=%d,lrbtree2比较次数=%d\n",tstl_times,tltree_times);
	}

	lrbtree2_destory(&ctx);

	return 0;
}
