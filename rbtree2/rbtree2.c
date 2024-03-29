#include "rbtree2.h"
#include <malloc.h>

#define nullptr 0
#define BLACK 0
#define RED 1

struct lrbtree2_bucket
{
	lrbtree2_bucket* next;
	lrbtree2_node node[0];
};

struct lrbtree2_iter
{
	lrbtree2_ctx* ctx;
	lrbtree2_node* cur;
};

static int lrbtree2_poniter_compare(const void* a,const void* b)
{
	if(a > b) return 1;
	if(a < b) return -1;
	return 0;
}

static lrbtree2_node* lrbtree2_find_node (lrbtree2_ctx* ctx,lrbtree2_node* cur,const void* data,int* eq)
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


static lrbtree2_node* lrbtree2_find_node_and_rank (lrbtree2_ctx* ctx,lrbtree2_node* cur,const void* data,int* eq,long* cc)
{
	int tr;
	long rank = 0;

	while(1){
		tr = ctx->compator(data,cur->data);
		if(tr == 0){
			if(cur->left){
				rank += cur->left->cc;
			}
			break;
		}else if(tr < 0){
			if(cur->left == nullptr) break;
			cur = cur->left;
		}else if(tr > 0){
			if(cur->left){
				rank += cur->left->cc + 1;
			}else{
				++rank;
			}
			if(cur->right == nullptr) break;
			cur = cur->right;
		}
	}
	*eq = tr;
	*cc = rank;
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

static void lrbtree2_rotate_left(lrbtree2_ctx* ctx,lrbtree2_node* n)
{
	//	左旋	右子节点变为父节点
	lrbtree2_node* r = n->right;
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

	{
		r->cc = n->cc;

		n->cc = 1;
		n->cc += n->left?n->left->cc:0;
		n->cc += n->right?n->right->cc:0;
	}
}

static void lrbtree2_rotate_right(lrbtree2_ctx* ctx,lrbtree2_node* r)
{
	//	右旋	左子节点变为父节点
	lrbtree2_node* n = r->left;
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


	{
		n->cc = r->cc;

		r->cc = 1;
		r->cc += r->left?r->left->cc:0;
		r->cc += r->right?r->right->cc:0;
	}

}



static lrbtree2_node* lrbtree2_get_min_node(lrbtree2_ctx* ctx)
{
	lrbtree2_node* cur = ctx->root;
	while(cur->left) cur = cur->left;
	return cur;
}

static lrbtree2_node* lrbtree2_get_max_node(lrbtree2_ctx* ctx)
{
	lrbtree2_node* cur = ctx->root;
	while(cur->right) cur = cur->right;
	return cur;
}

static lrbtree2_node* lrbtree2_get_last_node(lrbtree2_ctx* ctx,lrbtree2_node* node)
{
	if(node->left){
		node = node->left;
		while(node->right) node = node->right;
	}else if(node->parent){
		lrbtree2_node* parent = node->parent;
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


static lrbtree2_node* lrbtree2_get_next_node(lrbtree2_ctx* ctx,lrbtree2_node* node)
{
	if(node->right){
		node = node->right;
		while(node->left) node = node->left;
	}else if(node->parent){
		lrbtree2_node* parent = node->parent;
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


void lrbtree2_child_count_incr(lrbtree2_ctx* ctx,lrbtree2_node* node,long df)
{
	while(node){
		node->cc += df;
		node = node->parent;
	}
}

static void lrbtree2_insert_and_rebalance(lrbtree2_ctx* ctx,lrbtree2_node* node)
{
	while (node->parent && node->parent->color == RED){
		lrbtree2_node* grandpa = node->parent->parent;

		if (node->parent == grandpa->left){
			lrbtree2_node* uncle = grandpa->right;
			if (uncle && uncle->color == RED){
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
			}else{
				if (node == node->parent->right){
					node = node->parent;
					lrbtree2_rotate_left(ctx,node);
				}
				node->parent->color = BLACK;
				grandpa->color = RED;
				lrbtree2_rotate_right(ctx,grandpa);
			}
		}else{
			lrbtree2_node* uncle = grandpa->left;
			if (uncle && uncle->color == RED){
				node->parent->color = BLACK;
				uncle->color = BLACK;
				grandpa->color = RED;
				node = grandpa;
			}else{
				if (node == node->parent->left){
					node = node->parent;
					lrbtree2_rotate_right(ctx,node);
				}
				node->parent->color = BLACK;
				grandpa->color = RED;
				lrbtree2_rotate_left(ctx,grandpa);
			}
		}
	}
	ctx->root->color = BLACK;
}

static void lrbtree2_remove_and_rebalance(lrbtree2_ctx* ctx,lrbtree2_node* oldnode)
{
	lrbtree2_node* substitute = oldnode;
	lrbtree2_node* node = nullptr;
	lrbtree2_node* grandpa = nullptr;

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

	lrbtree2_child_count_incr(ctx,substitute,-1);

	if (substitute != oldnode){

		oldnode->left->parent = substitute;
		substitute->left = oldnode->left;
		substitute->cc = oldnode->cc;

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
				ctx->min = lrbtree2_get_min_node(ctx);
			}
		}
		if (ctx->max == oldnode){
			if (oldnode->left == nullptr){
				ctx->max = oldnode->parent;
			}else{
				ctx->max = lrbtree2_get_max_node(ctx);
			}
		}
	}

	if (substitute->color != RED){
		while (node != ctx->root && (node == nullptr || node->color == BLACK)){
			if (node == grandpa->left){
				lrbtree2_node* uncel = grandpa->right;
				if (uncel->color == RED){
					uncel->color = BLACK;
					grandpa->color = RED;
					lrbtree2_rotate_left(ctx,grandpa);
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
						lrbtree2_rotate_right(ctx,uncel);
						uncel = grandpa->right;
					}
					uncel->color = grandpa->color;
					grandpa->color = BLACK;
					if (uncel->right)
						uncel->right->color = BLACK;
					lrbtree2_rotate_left(ctx,grandpa);
					break;
				}
			}else{
				lrbtree2_node* uncel = grandpa->left;
				if (uncel->color == RED){
					uncel->color = BLACK;
					grandpa->color = RED;
					lrbtree2_rotate_right(ctx,grandpa);
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
						lrbtree2_rotate_left(ctx,uncel);
						uncel = grandpa->left;
					}
					uncel->color = grandpa->color;
					grandpa->color = BLACK;
					if (uncel->left){
						uncel->left->color = BLACK;
					}
					lrbtree2_rotate_right(ctx,grandpa);
					break;
				}
			}
		}
		if (node) node->color = BLACK;
	}
}

enum lrbtree2_ec lrbtree2_init(lrbtree2_ctx* ctx)
{
	//if(ctx->root != nullptr) return lrbtree2_fail;
	ctx->root = ctx->min = ctx->max = nullptr;
	ctx->size = 0;
	ctx->reserve = nullptr;
	ctx->capacity = 0;
	ctx->mpool = nullptr;

	if(ctx->compator ==nullptr){
		ctx->compator = lrbtree2_poniter_compare;
	}
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_destory(lrbtree2_ctx* ctx)
{
	if(ctx->mpool){
		while(ctx->mpool->next){
			lrbtree2_bucket* next = ctx->mpool->next;
			free(ctx->mpool);
			ctx->mpool = next;
		}
		free(ctx->mpool);
	}
	return lrbtree2_ok;
}

static lrbtree2_node* lrbtree2_new_node(lrbtree2_ctx* ctx,lrbtree2_node* parent,const void* data)
{
	lrbtree2_node* node = nullptr;

	if(ctx->reserve){
		node = ctx->reserve;
		ctx->reserve = node->parent;
	}else{
		if(ctx->capacity == 0){
			ctx->capacity = 32;
			lrbtree2_bucket* first_bucket = malloc(sizeof(lrbtree2_bucket) + (sizeof(lrbtree2_node) * ctx->capacity));
			if(first_bucket == nullptr){
				ctx->capacity = 0;
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
	node->cc = 0;
	return node;
}

enum lrbtree2_ec lrbtree2_insert(lrbtree2_ctx* ctx,const void* data,const void** old)
{

	if(ctx->root == nullptr){
		lrbtree2_node* node = lrbtree2_new_node(ctx,nullptr,data);
		if(node == nullptr) return lrbtree2_oom;
		node->color = BLACK;
		node->cc = 1;
		ctx->min = ctx->max = ctx->root = node;
		ctx->size = 1;
		return lrbtree2_ok;
	}

	//插入
	int eq;
	lrbtree2_node* fnode = lrbtree2_find_node(ctx,ctx->root,data,&eq);

	if(eq == 0){
		if(old) *old = fnode->data;
		fnode->data = data;
		return lrbtree2_update;
	}

	if(eq > 0){
			lrbtree2_node* node = lrbtree2_new_node(ctx,fnode,data);
			if(node == nullptr) return lrbtree2_oom;
			node->color = RED;
			fnode->right = node;
			if(fnode == ctx->max){
				ctx->max = node;
			}
			lrbtree2_child_count_incr(ctx,node,1);
			lrbtree2_insert_and_rebalance(ctx,node);
			++ctx->size;
			return lrbtree2_ok;
	}else{
		lrbtree2_node* node = lrbtree2_new_node(ctx,fnode,data);
		if(node == nullptr) return lrbtree2_oom;
		node->color = RED;
		fnode->left = node;
		if(fnode == ctx->min){
			ctx->min = node;
		}

		lrbtree2_child_count_incr(ctx,node,1);
		lrbtree2_insert_and_rebalance(ctx,node);
		++ctx->size;
		return lrbtree2_ok;
	}
	return lrbtree2_fail;
}


enum lrbtree2_ec lrbtree2_remove(lrbtree2_ctx* ctx,const void* key,const void** old)
{
	int eq;
	lrbtree2_node* fnode = lrbtree2_find_node(ctx,ctx->root,key,&eq);

	if(eq != 0){
		return lrbtree2_notfound;
	}

	lrbtree2_remove_and_rebalance(ctx,fnode);

	*old = fnode->data;
	--ctx->size;
	fnode->left = fnode->right = nullptr;

	fnode->parent = ctx->reserve;
	ctx->reserve = fnode;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_get(lrbtree2_ctx* ctx,const void* key,const void** data)
{
	int eq;
	if(ctx->root == nullptr) return lrbtree2_notfound;
	lrbtree2_node* fnode = lrbtree2_find_node(ctx,ctx->root,key,&eq);
	if(eq != 0){
		return lrbtree2_notfound;
	}

	if(fnode == nullptr) return lrbtree2_notfound;
	*data = fnode->data;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_at(struct lrbtree2_ctx* ctx,long rank,const void** data)
{
	lrbtree2_node* cur = ctx->root;

	if(cur == nullptr || rank >= cur->cc){
		return lrbtree2_notfound;
	}

	while(1){
		long cseq = cur->left?cur->left->cc:0;
		if(rank == cseq){
			*data = cur->data;
			return lrbtree2_ok;
		}else if(rank < cseq){
			cur = cur->left;
		}else{
			cur = cur->right;
			rank -= cseq + 1;
		}
	}
	return lrbtree2_notfound;
}

enum lrbtree2_ec lrbtree2_rank(struct lrbtree2_ctx* ctx,const void* key,long* rank)
{
	int eq;
	if(ctx->root == nullptr) return lrbtree2_notfound;
	lrbtree2_node* fnode = lrbtree2_find_node_and_rank(ctx,ctx->root,key,&eq,rank);
	if(eq != 0){
		return lrbtree2_notfound;
	}

	if(fnode == nullptr) return lrbtree2_notfound;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_scan_init(lrbtree2_ctx* ctx,lrbtree2_iter** iter)
{
	*iter = malloc(sizeof(lrbtree2_iter));
	if(*iter == nullptr){
		return lrbtree2_oom;
	}

	(*iter)->ctx = ctx;
	(*iter)->cur = (lrbtree2_node*)0x00000001;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_scan_reset(struct lrbtree2_ctx* ctx,struct lrbtree2_iter* iter)
{
	iter->ctx = ctx;
	iter->cur = (lrbtree2_node*)0x00000001;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_scan_seek(struct lrbtree2_iter* iter,const void* key,const void** data)
{
	int eq;
	lrbtree2_node* fnode = lrbtree2_find_node(iter->ctx,iter->ctx->root,key,&eq);
	if(eq > 0 && fnode){
		fnode =  lrbtree2_get_next_node(iter->ctx,fnode);
	}

	iter->cur = fnode;
	if(fnode){
		*data = fnode->data;
		return lrbtree2_ok;
	}
	return lrbtree2_notfound;
}

enum lrbtree2_ec lrbtree2_scan_seek_rank(struct lrbtree2_iter* iter,long rank,const void** data)
{
	lrbtree2_node* cur = iter->ctx->root;

	if(cur == nullptr || rank >= cur->cc){
		return lrbtree2_notfound;
	}

	while(1){
		long cseq = cur->left?cur->left->cc:0;
		if(rank == cseq){
			iter->cur = cur;
			*data = iter->cur->data;
			return lrbtree2_ok;
		}else if(rank < cseq){
			cur = cur->left;
		}else{
			cur = cur->right;
			rank -= cseq + 1;
		}
	}
	return lrbtree2_notfound;
}

enum lrbtree2_ec lrbtree2_scan_next(lrbtree2_iter* iter,const void** data)
{
	lrbtree2_ctx* ctx = iter->ctx;
	if(iter->cur == (lrbtree2_node*)0x00000001){
		iter->cur = ctx->min;
		if(iter->cur == nullptr) {
			return lrbtree2_end;
		}
		*data = iter->cur->data;
		return lrbtree2_ok;
	}else if(iter->cur == nullptr){
		return lrbtree2_end;
	}

	if(iter->cur == ctx->max){
		return lrbtree2_end;
	}
	
	iter->cur =  lrbtree2_get_next_node(ctx,iter->cur);
	*data = iter->cur->data;
	return lrbtree2_ok;
}


enum lrbtree2_ec lrbtree2_scan_last(lrbtree2_iter* iter,const void** data)
{
	lrbtree2_ctx* ctx = iter->ctx;
	if(iter->cur == (lrbtree2_node*)0x00000001){
		iter->cur = ctx->max;
		if(iter->cur == nullptr) {
			return lrbtree2_end;
		}
		*data = iter->cur->data;
		return lrbtree2_ok;
	}else if(iter->cur == nullptr){
		return lrbtree2_end;
	}

	if(iter->cur == ctx->min){
		return lrbtree2_end;
	}
	
	iter->cur =  lrbtree2_get_last_node(ctx,iter->cur);
	*data = iter->cur->data;
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_scan_destory(lrbtree2_iter* iter)
{
	free(iter);
	return lrbtree2_ok;
}

static enum lrbtree2_ec lrbtree2_verbose_node(lrbtree2_node* node,long depth,long idx,lrbtree2_lookup lookup_call_back,void* userdata)
{
	if(lookup_call_back(node,depth,idx,userdata) == 0){
		return lrbtree2_fail;
	}

	if(node->left){
		lrbtree2_verbose_node(node->left,depth + 1,idx * 2,lookup_call_back,userdata);
	}
	if(node->right){
		lrbtree2_verbose_node(node->right,depth + 1,idx * 2 + 1,lookup_call_back,userdata);
	}
	return lrbtree2_ok;
}

enum lrbtree2_ec lrbtree2_verbose(struct lrbtree2_ctx* ctx,lrbtree2_lookup lookup_call_back,void* userdata)
{
	lrbtree2_node* node = ctx->root;

	if(node == nullptr){
		return lrbtree2_notfound;
	}

	return lrbtree2_verbose_node(node,0,0,lookup_call_back,userdata);
}
