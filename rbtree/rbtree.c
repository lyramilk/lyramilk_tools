#include "rbtree.h"
#include <malloc.h>

#define nullptr 0
#define BLACK 0
#define RED 1

struct lrbtree_bucket
{
	lrbtree_bucket* next;
	lrbtree_node node[0];
};

struct lrbtree_iter
{
	lrbtree_ctx* ctx;
	lrbtree_node* cur;
};

static int lrbtree_poniter_compare(const void* a,const void* b)
{
	if(a > b) return 1;
	if(a < b) return -1;
	return 0;
}

static lrbtree_node* lrbtree_find_node (lrbtree_ctx* ctx,lrbtree_node* cur,const void* data,int* eq)
{
	int tr;

	while(1){
		tr = ctx->compator(data,cur->data);
		if(tr == 0){
			break;
		}else if(tr < 0){
			if(cur->left == nullptr) break;
			cur = cur->left;
		}else if(tr > 0){
			if(cur->right == nullptr) break;
			cur = cur->right;
		}
	}
	*eq = tr;
	return cur;
}


/***************************
         P                P
        /               /
       N              R
      / \    <-->    /
     L   R          N
        /          / \
       C          L   C
***************************/

static void lrbtree_rotate_left(lrbtree_ctx* ctx,lrbtree_node* n)
{
	//	左旋	右子节点变为父节点
	lrbtree_node* r = n->right;
	//	PN--->PR
	r->parent = n->parent;
	if(r->parent == nullptr){
		// 如果不存在P结点，说明原先的N是root
		ctx->root = r;
	}else if(r->parent->left == n){
		r->parent->left = r;
	}else{
		r->parent->right = r;
	}

	//  NR--->NC
	n->right = r->left;
	if(n->right){
		n->right->parent = n;
	}

	//	RC--->RN
	r->left = n; 
	n->parent = r;
}

static void lrbtree_rotate_right(lrbtree_ctx* ctx,lrbtree_node* r)
{
	//	右旋	左子节点变为父节点
	lrbtree_node* n = r->left;
	//	PR-->PN
	n->parent = r->parent;
	if(n->parent == nullptr){
		ctx->root = n;
	}else if(n->parent->left == r){
		n->parent->left = n;
	}else{
		n->parent->right = n;
	}

	//	NC--->RC
	r->left = n->right;
	if(r->left){
		r->left->parent = r;
	}

	//	NC--->NR
	n->right = r;
	r->parent = n;
}



static lrbtree_node* lrbtree_get_min_node(lrbtree_ctx* ctx)
{
	lrbtree_node* cur = ctx->root;
	while(cur->left) cur = cur->left;
	return cur;
}

static lrbtree_node* lrbtree_get_max_node(lrbtree_ctx* ctx)
{
	lrbtree_node* cur = ctx->root;
	while(cur->right) cur = cur->right;
	return cur;
}

static lrbtree_node* lrbtree_get_last_node(lrbtree_ctx* ctx,lrbtree_node* node)
{
	if(node->left){
		node = node->left;
		while(node->right) node = node->right;
	}else if(node->parent){
		lrbtree_node* parent = node->parent;
		while(parent && node == parent->left){
			node = node->parent;
			parent = parent->parent;
		}
		node = parent;
	}else{
		return nullptr;
	}
	return node;
}


static lrbtree_node* lrbtree_get_next_node(lrbtree_ctx* ctx,lrbtree_node* node)
{
	if(node->right){
		node = node->right;
		while(node->left) node = node->left;
	}else if(node->parent){
		lrbtree_node* parent = node->parent;
		while(parent && node == parent->right){
			node = node->parent;
			parent = parent->parent;
		}
		node = parent;
	}else{
		return nullptr;
	}
	return node;
}


static void lrbtree_insert_and_rebalance(lrbtree_ctx* ctx,lrbtree_node* node)
{
	while (node->parent && node->parent->color == RED){
		lrbtree_node* grandpa = node->parent->parent;

		if (node->parent == grandpa->left){
			lrbtree_node* uncle = grandpa->right;
			if (uncle && uncle->color == RED){
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
			}else{
				if (node == node->parent->right){
					node = node->parent;
					lrbtree_rotate_left(ctx,node);
				}
				node->parent->color = BLACK;
				grandpa->color = RED;
				lrbtree_rotate_right(ctx,grandpa);
			}
		}else{
			lrbtree_node* uncle = grandpa->left;
			if (uncle && uncle->color == RED){
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
			}else{
				if (node == node->parent->left){
					node = node->parent;
					lrbtree_rotate_right(ctx,node);
				}
				node->parent->color = BLACK;
				grandpa->color = RED;
				lrbtree_rotate_left(ctx,grandpa);
			}
		}
	}
	ctx->root->color = BLACK;
}

static void lrbtree_remove_and_rebalance(lrbtree_ctx* ctx,lrbtree_node* oldnode)
{
	lrbtree_node* substitute = oldnode;
	lrbtree_node* node = nullptr;
	lrbtree_node* grandpa = nullptr;

	if (substitute->left == nullptr){
		node = substitute->right;
	}else{
		if (substitute->right == nullptr)
			node = substitute->left;
		else{
			substitute = substitute->right;
			while (substitute->left != nullptr){
				substitute = substitute->left;
			}
			node = substitute->right;
		}
	}

	if (substitute != oldnode){
		oldnode->left->parent = substitute;
		substitute->left = oldnode->left;
		if (substitute != oldnode->right){
			grandpa = substitute->parent;
			if (node){
				node->parent = substitute->parent;
			}
			substitute->parent->left = node;
			substitute->right = oldnode->right;
			oldnode->right->parent = substitute;
		}else{
			grandpa = substitute;
		}
		if (ctx->root == oldnode){
			ctx->root = substitute;
		}else if (oldnode->parent->left == oldnode){
			oldnode->parent->left = substitute;
		}else{
			oldnode->parent->right = substitute;
		}
		substitute->parent = oldnode->parent;
		{
			unsigned char c = oldnode->color;
			oldnode->color = substitute->color;
			substitute->color = c;
		}
		substitute = oldnode;
	}else{
		grandpa = substitute->parent;
		if (node){
			node->parent = substitute->parent;
		}
		if (ctx->root == oldnode){
			ctx->root = node;
		}else{
			if (oldnode->parent->left == oldnode){
				oldnode->parent->left = node;
			}else{
				oldnode->parent->right = node;
			}
		}
		if (ctx->min == oldnode){
			if (oldnode->right == nullptr){
				ctx->min = oldnode->parent;
			}else{
				ctx->min = lrbtree_get_min_node(ctx);
			}
		}
		if (ctx->max == oldnode){
			if (oldnode->left == nullptr){
				ctx->max = oldnode->parent;
			}else{
				ctx->max = lrbtree_get_max_node(ctx);
			}
		}
	}

	if (substitute->color != RED){
		while (node != ctx->root && (node == nullptr || node->color == BLACK)){
			if (node == grandpa->left){
				lrbtree_node* uncel = grandpa->right;
				if (uncel->color == RED){
					uncel->color = BLACK;
					grandpa->color = RED;
					lrbtree_rotate_left(ctx,grandpa);
					uncel = grandpa->right;
				}
				if ((uncel->left == nullptr || uncel->left->color == BLACK) && (uncel->right == nullptr || uncel->right->color == BLACK)) {
					uncel->color = RED;
					node = grandpa;
					grandpa = grandpa->parent;
				}else{
					if (uncel->right == nullptr || uncel->right->color == BLACK){
						uncel->left->color = BLACK;
						uncel->color = RED;
						lrbtree_rotate_right(ctx,uncel);
						uncel = grandpa->right;
					}
					uncel->color = grandpa->color;
					grandpa->color = BLACK;
					if (uncel->right)
						uncel->right->color = BLACK;
					lrbtree_rotate_left(ctx,grandpa);
					break;
				}
			}else{
				lrbtree_node* uncel = grandpa->left;
				if (uncel->color == RED){
					uncel->color = BLACK;
					grandpa->color = RED;
					lrbtree_rotate_right(ctx,grandpa);
					uncel = grandpa->left;
				}
				if ((uncel->right == nullptr || uncel->right->color == BLACK) && (uncel->left == nullptr || uncel->left->color == BLACK)){
					uncel->color = RED;
					node = grandpa;
					grandpa = grandpa->parent;
				}else{
					if (uncel->left == nullptr || uncel->left->color == BLACK){
						uncel->right->color = BLACK;
						uncel->color = RED;
						lrbtree_rotate_left(ctx,uncel);
						uncel = grandpa->left;
					}
					uncel->color = grandpa->color;
					grandpa->color = BLACK;
					if (uncel->left){
						uncel->left->color = BLACK;
					}
					lrbtree_rotate_right(ctx,grandpa);
					break;
				}
			}
		}
		if (node) node->color = BLACK;
	}
}

enum lrbtree_ec lrbtree_init(lrbtree_ctx* ctx)
{
	//if(ctx->root != nullptr) return lrbtree_fail;
	ctx->root = ctx->min = ctx->max = nullptr;
	ctx->size = 0;
	ctx->reserve = nullptr;
	ctx->capacity = 0;
	ctx->mpool = nullptr;

	if(ctx->compator ==nullptr){
		ctx->compator = lrbtree_poniter_compare;
	}
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_destory(lrbtree_ctx* ctx)
{
	if(ctx->mpool){
		while(ctx->mpool->next){
			lrbtree_bucket* next = ctx->mpool->next;
			free(ctx->mpool);
			ctx->mpool = next;
		}
		free(ctx->mpool);
	}
	return lrbtree_ok;
}

static lrbtree_node* lrbtree_new_node(lrbtree_ctx* ctx,lrbtree_node* parent,const void* data)
{
	lrbtree_node* node = nullptr;

	if(ctx->reserve){
		node = ctx->reserve;
		ctx->reserve = node->parent;
	}else{
		if(ctx->capacity == 0){
			ctx->capacity = 32;
			lrbtree_bucket* first_bucket = malloc(sizeof(lrbtree_bucket) + (sizeof(lrbtree_node) * ctx->capacity));
			if(first_bucket == nullptr){
				return nullptr;
			}
			first_bucket->next = nullptr;
			first_bucket->next = ctx->mpool;
			ctx->mpool = first_bucket;
		}
		node = &ctx->mpool->node[ctx->capacity - 1];
	}
	--ctx->capacity;
	node->left = node->right = nullptr;
	node->parent = parent;
	node->data = data;
	return node;
}

enum lrbtree_ec lrbtree_insert(lrbtree_ctx* ctx,const void* data,const void** old)
{

	if(ctx->root == nullptr){
		lrbtree_node* node = lrbtree_new_node(ctx,nullptr,data);
		if(node == nullptr) return lrbtree_oom;
		node->color = BLACK;
		ctx->min = ctx->max = ctx->root = node;
		ctx->size = 1;
		return lrbtree_ok;
	}

	//插入
	int eq;
	lrbtree_node* fnode = lrbtree_find_node(ctx,ctx->root,data,&eq);

	if(eq == 0){
		if(old) *old = fnode->data;
		fnode->data = data;
		return lrbtree_update;
	}

	if(eq > 0){
			lrbtree_node* node = lrbtree_new_node(ctx,fnode,data);
			if(node == nullptr) return lrbtree_oom;
			node->color = RED;
			fnode->right = node;
			if(fnode == ctx->max){
				ctx->max = node;
			}
			lrbtree_insert_and_rebalance(ctx,node);
			++ctx->size;
			return lrbtree_ok;
	}else{
		lrbtree_node* node = lrbtree_new_node(ctx,fnode,data);
		if(node == nullptr) return lrbtree_oom;
		node->color = RED;
		fnode->left = node;
		if(fnode == ctx->min){
			ctx->min = node;
		}

		lrbtree_insert_and_rebalance(ctx,node);
		++ctx->size;
		return lrbtree_ok;
	}
	return lrbtree_fail;
}


enum lrbtree_ec lrbtree_remove(lrbtree_ctx* ctx,const void* key,const void** old)
{
	int eq;
	lrbtree_node* fnode = lrbtree_find_node(ctx,ctx->root,key,&eq);

	if(eq != 0){
		return lrbtree_notfound;
	}

	lrbtree_remove_and_rebalance(ctx,fnode);
	*old = fnode->data;
	--ctx->size;
	fnode->left = fnode->right = nullptr;

	fnode->parent = ctx->reserve;
	ctx->reserve = fnode;
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_get(lrbtree_ctx* ctx,const void* key,const void** data)
{
	int eq;
	if(ctx->root == nullptr) return lrbtree_notfound;
	lrbtree_node* fnode = lrbtree_find_node(ctx,ctx->root,key,&eq);
	if(eq != 0){
		return lrbtree_notfound;
	}

	if(fnode == nullptr) return lrbtree_notfound;
	*data = fnode->data;
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_scan_init(lrbtree_ctx* ctx,lrbtree_iter** iter)
{
	*iter = malloc(sizeof(lrbtree_iter));
	if(*iter == nullptr){
		return lrbtree_oom;
	}

	(*iter)->ctx = ctx;
	(*iter)->cur = (lrbtree_node*)0x00000001;
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_scan_reset(lrbtree_ctx* ctx,lrbtree_iter* iter)
{
	iter->ctx = ctx;
	iter->cur = (lrbtree_node*)0x00000001;
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_scan_seek(struct lrbtree_iter* iter,const void* key,const void** data)
{
	int eq;
	lrbtree_node* fnode = lrbtree_find_node(iter->ctx,iter->ctx->root,key,&eq);
	if(eq > 0 && fnode){
		fnode =  lrbtree_get_next_node(iter->ctx,fnode);
	}

	iter->cur = fnode;
	if(fnode){
		*data = fnode->data;
		return lrbtree_ok;
	}
	return lrbtree_notfound;
}

enum lrbtree_ec lrbtree_scan_seek_rank(struct lrbtree_iter* iter,long rank,const void** data)
{
	iter->cur = (lrbtree_node*)0x00000001;

	for(long idx =0;lrbtree_scan_next(iter,data) == lrbtree_ok;++idx){
		if(idx == rank){
			return lrbtree_ok;
		}
	}
	return lrbtree_notfound;
}

enum lrbtree_ec lrbtree_scan_next(lrbtree_iter* iter,const void** data)
{
	lrbtree_ctx* ctx = iter->ctx;
	if(iter->cur == (lrbtree_node*)0x00000001){
		iter->cur = ctx->min;
		if(iter->cur == nullptr) {
			return lrbtree_end;
		}
		*data = iter->cur->data;
		return lrbtree_ok;
	}else if(iter->cur == nullptr){
		return lrbtree_end;
	}

	if(iter->cur == ctx->max){
		return lrbtree_end;
	}
	
	iter->cur =  lrbtree_get_next_node(ctx,iter->cur);
	*data = iter->cur->data;
	return lrbtree_ok;
}


enum lrbtree_ec lrbtree_scan_last(lrbtree_iter* iter,const void** data)
{
	lrbtree_ctx* ctx = iter->ctx;
	if(iter->cur == (lrbtree_node*)0x00000001){
		iter->cur = ctx->max;
		if(iter->cur == nullptr) {
			return lrbtree_end;
		}
		*data = iter->cur->data;
		return lrbtree_ok;
	}else if(iter->cur == nullptr){
		return lrbtree_end;
	}

	if(iter->cur == ctx->min){
		return lrbtree_end;
	}
	
	iter->cur =  lrbtree_get_last_node(ctx,iter->cur);
	*data = iter->cur->data;
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_scan_destory(lrbtree_iter* iter)
{
	free(iter);
	return lrbtree_ok;
}

static enum lrbtree_ec lrbtree_verbose_node(lrbtree_node* node,long depth,long idx,lrbtree_lookup lookup_call_back,void* userdata)
{
	lookup_call_back(node,depth,idx,userdata);

	if(node->left){
		lrbtree_verbose_node(node->left,depth + 1,idx * 2,lookup_call_back,userdata);
	}
	if(node->right){
		lrbtree_verbose_node(node->right,depth + 1,idx * 2 + 1,lookup_call_back,userdata);
	}
	return lrbtree_ok;
}

enum lrbtree_ec lrbtree_verbose(struct lrbtree_ctx* ctx,lrbtree_lookup lookup_call_back,void* userdata)
{
	lrbtree_node* node = ctx->root;

	if(node == nullptr){
		return lrbtree_notfound;
	}

	return lrbtree_verbose_node(node,0,0,lookup_call_back,userdata);
}
