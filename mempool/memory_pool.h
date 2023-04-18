#ifndef _lyramilk_ctools_memory_pool_h_
#define _lyramilk_ctools_memory_pool_h_

#ifdef __cplusplus
extern "C"{
#endif
	/*
		@brief 回调申请分配内存
	*/
	typedef void* (*lymalloc_cbk)(unsigned long size,void* userdata);
	typedef void (*lyfree_cbk)(void* ptr,unsigned long size,void* userdata);
	typedef struct lymempool_bucket lymempool_bucket;
	typedef struct lymempool_node lymempool_node;
	

	typedef struct lymempool
	{
		lymalloc_cbk palloc;
		lyfree_cbk pfree;
		void* userdata;
		lymempool_node* bk[7];
		lymempool_bucket* meta;
		lymempool_node* reserve;
		long nodecapacity;
		long pagesize;
		long pagelongcount;
	}lymempool;

	void lymempool_init(lymempool* mpool,lymalloc_cbk alf,lyfree_cbk frf,void* userdata);
	void lymempool_destory(lymempool* mpool);

	void* lymempool_malloc(lymempool* mpool,unsigned long size);
	void lymempool_free(lymempool* mpool,void* ptr);


#ifdef __cplusplus
}
#endif



#endif
