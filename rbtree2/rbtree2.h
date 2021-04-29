#ifndef _lyramilk_caveoo_rbtree2_h_
#define _lyramilk_caveoo_rbtree2_h_

#ifdef __cplusplus
extern "C"{
#endif

	typedef struct lrbtree2_iter lrbtree2_iter;
	typedef struct lrbtree2_bucket lrbtree2_bucket;

	typedef struct lrbtree2_node
	{
		const void* data;
		struct lrbtree2_node* left;
		struct lrbtree2_node* right;
		struct lrbtree2_node* parent;
		unsigned char color:1;	//	0黑	1红
		long cc;	//	子结点数
	} lrbtree2_node;

	typedef int (*lrbtree2_compator)(const void* a,const void* b);
	typedef int (*lrbtree2_lookup)(lrbtree2_node* node,int depth,int idx,void* userdata);
	
	typedef struct lrbtree2_ctx
	{
		struct lrbtree2_node* root;
		struct lrbtree2_node* min;
		struct lrbtree2_node* max;
		lrbtree2_compator compator;
		long size;

		struct lrbtree2_bucket* mpool;
		struct lrbtree2_node* reserve;
		long capacity;
		int (*debug)(struct lrbtree2_ctx* ctx);
	}lrbtree2_ctx;

	enum lrbtree2_ec{
		lrbtree2_ok,			//成功
		lrbtree2_update,		//插入时发现己存在，所以更新
		lrbtree2_fail,		//失败不解释
		lrbtree2_oom,		//malloc失败
		lrbtree2_notfound,	//没有找到
		lrbtree2_end,		//迭代完成
	};


	/// root的属性将会被重置且不涉及任何指针的内存释放。
	enum lrbtree2_ec lrbtree2_init(struct lrbtree2_ctx* ctx);
	enum lrbtree2_ec lrbtree2_destory(struct lrbtree2_ctx* ctx);

	///	old在发生lrbtree2_update时被赋值为替换之前的旧值。
	enum lrbtree2_ec lrbtree2_insert(struct lrbtree2_ctx* ctx,const void* data,const void** old);
	enum lrbtree2_ec lrbtree2_remove(struct lrbtree2_ctx* ctx,const void* key,const void** old);
	enum lrbtree2_ec lrbtree2_get(struct lrbtree2_ctx* ctx,const void* key,const void** data);
	enum lrbtree2_ec lrbtree2_at(struct lrbtree2_ctx* ctx,long rank,const void** data);
	enum lrbtree2_ec lrbtree2_rank(struct lrbtree2_ctx* ctx,const void* key,long* rank);

	enum lrbtree2_ec lrbtree2_scan_init(struct lrbtree2_ctx* ctx,struct lrbtree2_iter** iter);
	enum lrbtree2_ec lrbtree2_scan_reset(struct lrbtree2_ctx* ctx,struct lrbtree2_iter* iter);
	enum lrbtree2_ec lrbtree2_scan_seek(struct lrbtree2_iter* iter,const void* key,const void** data);
	enum lrbtree2_ec lrbtree2_scan_seek_rank(struct lrbtree2_iter* iter,long rank,const void** data);
	enum lrbtree2_ec lrbtree2_scan_next(struct lrbtree2_iter* iter,const void** data);
	enum lrbtree2_ec lrbtree2_scan_last(struct lrbtree2_iter* iter,const void** data);
	enum lrbtree2_ec lrbtree2_scan_destory(struct lrbtree2_iter* iter);

	enum lrbtree2_ec lrbtree2_verbose(struct lrbtree2_ctx* ctx,lrbtree2_lookup lookup_call_back,void* userdata);


#ifdef __cplusplus
}
#endif



#endif
