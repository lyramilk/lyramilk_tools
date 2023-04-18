#ifndef _lyramilk_ctools_ltree_h_
#define _lyramilk_ctools_ltree_h_

#ifdef __cplusplus
extern "C"{
#endif

	typedef struct ltree_iter ltree_iter;
	typedef struct ltree_bucket ltree_bucket;

	typedef struct ltree_node
	{
		const void* data;
		struct ltree_node* left;
		struct ltree_node* right;
		struct ltree_node* parent;
	} ltree_node;

	typedef int (*ltree_compator)(const void* a,const void* b);
	typedef void (*ltree_lookup)(ltree_node* node,int depth,int idx,void* userdata);

	
	typedef struct ltree_ctx
	{
		struct ltree_node* root;
		struct ltree_node* min;
		struct ltree_node* max;
		ltree_compator compator;
		long size;

		struct ltree_bucket* mpool;
		struct ltree_node* reserve;
		long capacity;
	}ltree_ctx;

	enum ltree_ec{
		ltree_ok,			//成功
		ltree_update,		//插入时发现己存在，所以更新
		ltree_fail,		//失败不解释
		ltree_oom,		//malloc失败
		ltree_notfound,	//没有找到
		ltree_end,		//迭代完成
	};


	/// root的属性将会被重置且不涉及任何指针的内存释放。
	enum ltree_ec ltree_init(struct ltree_ctx* ctx);
	enum ltree_ec ltree_destory(struct ltree_ctx* ctx);

	///	old在发生ltree_update时被赋值为替换之前的旧值。
	enum ltree_ec ltree_insert(struct ltree_ctx* ctx,const void* data,const void** old);
	enum ltree_ec ltree_remove(struct ltree_ctx* ctx,const void* key,const void** old);
	enum ltree_ec ltree_get(struct ltree_ctx* ctx,const void* key,const void** data);

	enum ltree_ec ltree_scan_init(struct ltree_ctx* root,struct ltree_iter** iter);
	enum ltree_ec ltree_scan_seek(struct ltree_iter* iter,const void* key,const void** data);
	enum ltree_ec ltree_scan_next(struct ltree_iter* iter,const void** data);
	enum ltree_ec ltree_scan_last(struct ltree_iter* iter,const void** data);
	enum ltree_ec ltree_scan_destory(struct ltree_iter* iter);

	enum ltree_ec ltree_verbose(struct ltree_ctx* ctx,ltree_lookup lookup_call_back,void* userdata);


#ifdef __cplusplus
}
#endif



#endif
