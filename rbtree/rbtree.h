#ifndef _lyramilk_ctools_rbtree_h_
#define _lyramilk_ctools_rbtree_h_

#ifdef __cplusplus
extern "C"{
#endif

	typedef struct lrbtree_iter lrbtree_iter;
	typedef struct lrbtree_bucket lrbtree_bucket;

	typedef struct lrbtree_node
	{
		const void* data;
		struct lrbtree_node* left;
		struct lrbtree_node* right;
		struct lrbtree_node* parent;
		unsigned char color:1;	//	0黑	1红
	} lrbtree_node;

	typedef int (*lrbtree_compator)(const void* a,const void* b);
	typedef void (*lrbtree_lookup)(lrbtree_node* node,int depth,int idx,void* userdata);

	
	typedef struct lrbtree_ctx
	{
		struct lrbtree_node* root;
		struct lrbtree_node* min;
		struct lrbtree_node* max;
		lrbtree_compator compator;
		long size;

		struct lrbtree_bucket* mpool;
		struct lrbtree_node* reserve;
		long capacity;
	}lrbtree_ctx;

	enum lrbtree_ec{
		lrbtree_ok,			//成功
		lrbtree_update,		//插入时发现己存在，所以更新
		lrbtree_fail,		//失败不解释
		lrbtree_oom,		//malloc失败
		lrbtree_notfound,	//没有找到
		lrbtree_end,		//迭代完成
	};


	/// root的属性将会被重置且不涉及任何指针的内存释放。
	enum lrbtree_ec lrbtree_init(struct lrbtree_ctx* ctx);
	enum lrbtree_ec lrbtree_destory(struct lrbtree_ctx* ctx);

	///	old在发生lrbtree_update时被赋值为替换之前的旧值。
	enum lrbtree_ec lrbtree_insert(struct lrbtree_ctx* ctx,const void* data,const void** old);
	enum lrbtree_ec lrbtree_remove(struct lrbtree_ctx* ctx,const void* key,const void** old);
	enum lrbtree_ec lrbtree_get(struct lrbtree_ctx* ctx,const void* key,const void** data);

	enum lrbtree_ec lrbtree_scan_init(struct lrbtree_ctx* ctx,struct lrbtree_iter** iter);
	enum lrbtree_ec lrbtree_scan_reset(struct lrbtree_ctx* ctx,struct lrbtree_iter* iter);
	enum lrbtree_ec lrbtree_scan_seek(struct lrbtree_iter* iter,const void* key,const void** data);
	enum lrbtree_ec lrbtree_scan_seek_rank(struct lrbtree_iter* iter,long rank,const void** data);
	enum lrbtree_ec lrbtree_scan_next(struct lrbtree_iter* iter,const void** data);
	enum lrbtree_ec lrbtree_scan_last(struct lrbtree_iter* iter,const void** data);
	enum lrbtree_ec lrbtree_scan_destory(struct lrbtree_iter* iter);

	enum lrbtree_ec lrbtree_verbose(struct lrbtree_ctx* ctx,lrbtree_lookup lookup_call_back,void* userdata);


#ifdef __cplusplus
}
#endif



#endif
