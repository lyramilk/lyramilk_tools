#include "memory_pool.h"

#define nullptr 0
#define TRACE printf


#define CONST_PAGESIZE (1024 * 64)
#define CONST_BITMAP (CONST_PAGESIZE >> 11)


struct lymempool_node
{
	struct lymempool_node* left;
	struct lymempool_node* right;
	unsigned long long bitmap[CONST_BITMAP];
	long capacity;
	long size;
	long datasize;
	unsigned int crb;
	unsigned int id;
	char* data;
};

struct lymempool_bucket
{
	struct lymempool_bucket* next;
	struct lymempool_node node[0];
};



long node_times = 0;

static lymempool_node* lymempool_new_node(lymempool* mpool,struct lymempool_node* left,unsigned int poolid,unsigned long datasize)
{
	unsigned int cr[] = {5,6,7,8,9,11,12};

	node_times += 1;
	lymempool_node* node = nullptr;

	if(mpool->reserve){
		node = mpool->reserve;
		mpool->reserve = node->left;
	}else{
		if(mpool->nodecapacity == 0){
			long* newmemory = (long*)mpool->palloc(mpool->pagesize,mpool->userdata);
			if(newmemory == nullptr) return nullptr;
			long* newmemoryptr = newmemory;
			for(long q = 0;q < mpool->pagelongcount;++q){
				*newmemoryptr = 0;
				++newmemoryptr;
			}
			lymempool_bucket* first_bucket = (lymempool_bucket*)newmemory;
			unsigned long capacity_per_page = (mpool->pagesize - sizeof(lymempool_bucket))/sizeof(lymempool_node);
			mpool->nodecapacity = capacity_per_page;
			first_bucket->next = mpool->meta;
			mpool->meta = first_bucket;
		}
		node = &mpool->meta->node[mpool->nodecapacity - 1];
		--mpool->nodecapacity;
	}

	if(left){
		node->left = left;
		left->right = node;
	}else{
		node->left = nullptr;
		node->right = nullptr;
	}
	node->datasize = datasize;
	node->data = mpool->palloc(node->datasize,mpool->userdata);
	node->crb = cr[poolid];
	node->id = poolid;
	node->size = 0;
	for(long i=0;i<CONST_BITMAP;++i){
		node->bitmap[i] = 0l;
	}
	return node;
}


static void lymempool_free_node(lymempool* mpool,struct lymempool_node* node)
{
	if(node->data == nullptr) return;
	if(node->right){
		node->right->left = node->left;
	}
	if(node->left){
		node->left->right = node->right;
	}
	node->left = node->right = nullptr;

	if(mpool->reserve == nullptr){
		mpool->reserve = node;

	}else{
		mpool->reserve->left = node;
		mpool->reserve = node;
	}

	mpool->pfree(mpool->reserve->data,mpool->reserve->datasize,mpool->userdata);
	mpool->reserve->data = nullptr;
}

void lymempool_init(lymempool* mpool,lymalloc_cbk alf,lyfree_cbk frf,void* userdata)
{
	mpool->palloc = alf;
	mpool->pfree = frf;
	mpool->userdata = userdata;
	mpool->pagesize = CONST_PAGESIZE;
	mpool->pagelongcount = mpool->pagesize / sizeof(long);
	mpool->nodecapacity = 0;
	mpool->reserve = nullptr;
	mpool->meta = nullptr;

	for(long i = 0;i<7;++i){
		mpool->bk[i] = nullptr;
	}
}


void lymempool_destory(lymempool* mpool)
{
	for(long i = 0;i<7;++i){
		while(mpool->bk[i]){
			lymempool_node* node = mpool->bk[i];
			mpool->bk[i] = mpool->bk[i]->left;
			lymempool_free_node(mpool,node);
		}
	}
	while(mpool->meta){
		struct lymempool_bucket* old = mpool->meta;
		mpool->meta = mpool->meta->next;
		mpool->pfree(old,mpool->pagesize,mpool->userdata);
	}

}

void* lymempool_malloc(lymempool* mpool,unsigned long size)
{
	unsigned long poolid = 6;

	unsigned int crg[] = {32,64,128,256,512,2048,4096};
	unsigned int cr[] = {5,6,7,8,9,11,12};

	unsigned long bksize = size + 8;

	for(long i =0;i<6;++i){
		if(bksize <= crg[i]){
			poolid = i;
			break;
		}
	}

	if(poolid != 6){
		lymempool_node* node = mpool->bk[poolid];
		if(node == nullptr){
			lymempool_node* p = lymempool_new_node(mpool,nullptr,poolid,mpool->pagesize);
			unsigned int granularity = 1 << cr[poolid];
			p->capacity = mpool->pagesize / granularity;
			p->size = 0;
			p->id = poolid;

			mpool->bk[poolid] = node = p;
		}

		do{
			if(node->size < node->capacity){
				for(unsigned long i =0;i<CONST_BITMAP;++i){
					if(node->bitmap[i] != 0xffffffffffffffffl){
						long clz = __builtin_clzll(~node->bitmap[i]);
						long b = 63 - clz;
						node->bitmap[i] |= 1ll<<b;

						lymempool_node** ptr = (node->data + ((clz+64 * i)<<node->crb));
						*ptr = node;
						++node->size;
						return (void*)(ptr + 1);
					}
				}
			}
		}while(node->right != nullptr?node = node->right:nullptr);

		{
			node = lymempool_new_node(mpool,mpool->bk[poolid],poolid,mpool->pagesize);
			node->capacity = mpool->bk[poolid]->capacity;
			mpool->bk[poolid] = node;
			{
				long clz = __builtin_clzll(~node->bitmap[0]);
				long b = 63 - clz;
				node->bitmap[0] |= 1ll<<b;

				lymempool_node** ptr = (node->data + (clz<<node->crb));
				*ptr = node;
				++node->size;
				return (void*)(ptr + 1);
			}
		}
	}else{
		lymempool_node* node = lymempool_new_node(mpool,mpool->bk[poolid],poolid,bksize);
		mpool->bk[6] = node;
		lymempool_node** ptr = node->data;
		*ptr = node;
		return (void*)(ptr + 1);
	}
	return nullptr;
}

void lymempool_free(lymempool* mpool,void* ptr)
{
	lymempool_node** ppnode = (lymempool_node**)((char*)ptr - 8);
	lymempool_node* node = *ppnode;

	if(node->crb > 11){
		lymempool_free_node(mpool,node);
		return;
	}


	unsigned long idx = ((unsigned long)((char*)ppnode - node->data)) >> node->crb;

	unsigned long ix = idx>>6;
	unsigned long iy = idx%64;
	unsigned long long mask = 1ll<<(63-iy);
	if(node->bitmap[ix] & mask){
		node->bitmap[ix] &= ~(mask);
		--node->size;
	}

	if(node->size == 0){
		if(node != mpool->bk[node->id]){
			lymempool_free_node(mpool,node);
		}
	}
}

