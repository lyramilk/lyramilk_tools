#include "ltree.h"
#include <malloc.h>

#define nullptr 0
#define BLACK 0
#define RED 1

struct ltree_bucket
{
	ltree_bucket* next;
	ltree_node node[0];
};

struct ltree_iter
{
	ltree_ctx* ctx;
	ltree_node* cur;
};

static int ltree_poniter_compare(const void* a,const void* b)
{
	if(a > b) return 1;
	if(a < b) return 1;
	return 0;
}

static ltree_node* ltree_find_node (ltree_ctx* ctx,ltree_node* cur,const void* data,int* eq)
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

static void ltree_rotate_left(ltree_ctx* ctx,ltree_node* n)
{
	//	左旋	右子节点变为父节点
	ltree_node* r = n->right;
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

static void ltree_rotate_right(ltree_ctx* ctx,ltree_node* r)
{
	//	右旋	左子节点变为父节点
	ltree_node* n = r->left;
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



static ltree_node* ltree_get_min_node(ltree_ctx* ctx)
{
	ltree_node* cur = ctx->root;
	while(cur->left) cur = cur->left;
	return cur;
}

static ltree_node* ltree_get_max_node(ltree_ctx* ctx)
{
	ltree_node* cur = ctx->root;
	while(cur->right) cur = cur->right;
	return cur;
}

static ltree_node* ltree_get_last_node(ltree_ctx* ctx,ltree_node* node)
{
	if(node->left){
		node = node->left;
		while(node->right) node = node->right;
	}else if(node->parent){
		ltree_node* parent = node->parent;
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


static ltree_node* ltree_get_next_node(ltree_ctx* ctx,ltree_node* node)
{
	if(node->right){
		node = node->right;
		while(node->left) node = node->left;
	}else if(node->parent){
		ltree_node* parent = node->parent;
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


static void ltree_insert_and_rebalance(ltree_ctx* ctx,ltree_node* node)
{
}

static void ltree_remove_and_rebalance(ltree_ctx* ctx,ltree_node* oldnode)
{

	ltree_node* substitute = oldnode;
	ltree_node* node = nullptr;
	ltree_node* parent = nullptr;

	if (substitute->left == 0){
		node = substitute->right;
	}else{
		if (substitute->right == 0)
			node = substitute->left;
		else{
			substitute = substitute->right;
			while (substitute->left != 0){
				substitute = substitute->left;
			}
			node = substitute->right;
		}
	}

	
	if (substitute != oldnode){
		oldnode->left->parent = substitute;
		substitute->left = oldnode->left;
		if (substitute != oldnode->right){
			parent = substitute->parent;
			if (node) node->parent = substitute->parent;
			substitute->parent->left = node;
			substitute->right = oldnode->right;
			oldnode->right->parent = substitute;
		}else{
			parent = substitute;
		}
		if (ctx->root == oldnode){
			ctx->root = substitute;
		}else if (oldnode->parent->left == oldnode){
			oldnode->parent->left = substitute;
		}else{
			oldnode->parent->right = substitute;
		}
		substitute->parent = oldnode->parent;
		substitute = oldnode;
	}else{
		parent = substitute->parent;
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
				ctx->min = ltree_get_min_node(ctx);
			}
		}
		if (ctx->max == oldnode){
			if (oldnode->left == nullptr){
				ctx->max = oldnode->parent;
			}else{
				ctx->max = ltree_get_max_node(ctx);
			}
		}
	}

}

enum ltree_ec ltree_init(ltree_ctx* ctx)
{
	//if(ctx->root != nullptr) return ltree_fail;
	ctx->root = ctx->min = ctx->max = nullptr;
	ctx->size = 0;
	ctx->reserve = nullptr;
	ctx->capacity = 0;
	ctx->mpool = nullptr;

	if(ctx->compator ==nullptr){
		ctx->compator = ltree_poniter_compare;
	}
	return ltree_ok;
}

enum ltree_ec ltree_destory(ltree_ctx* ctx)
{
	if(ctx->mpool){
		while(ctx->mpool->next){
			ltree_bucket* next = ctx->mpool->next;
			free(ctx->mpool);
			ctx->mpool = next;
		}
		free(ctx->mpool);
	}
	return ltree_ok;
}

static ltree_node* ltree_new_node(ltree_ctx* ctx,ltree_node* parent,const void* data)
{
	ltree_node* node = nullptr;

	if(ctx->reserve){
		node = ctx->reserve;
		ctx->reserve = node->parent;
	}else{
		if(ctx->capacity == 0){
			ctx->capacity = 32;
			ltree_bucket* first_bucket = malloc(sizeof(ltree_bucket) + (sizeof(ltree_node) * ctx->capacity));
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

enum ltree_ec ltree_insert(ltree_ctx* ctx,const void* data,const void** old)
{

	if(ctx->root == nullptr){
		ltree_node* node = ltree_new_node(ctx,nullptr,data);
		if(node == nullptr) return ltree_oom;
		ctx->min = ctx->max = ctx->root = node;
		ctx->size = 1;
		return ltree_ok;
	}

	//插入
	int eq;
	ltree_node* fnode = ltree_find_node(ctx,ctx->root,data,&eq);

	if(eq == 0){
		if(old) *old = fnode->data;
		fnode->data = data;
		return ltree_update;
	}

	if(eq > 0){
			ltree_node* node = ltree_new_node(ctx,fnode,data);
			if(node == nullptr) return ltree_oom;
			fnode->right = node;
			if(fnode == ctx->max){
				ctx->max = node;
			}
			ltree_insert_and_rebalance(ctx,node);
			++ctx->size;
			return ltree_ok;
	}else{
		ltree_node* node = ltree_new_node(ctx,fnode,data);
		if(node == nullptr) return ltree_oom;
		fnode->left = node;
		if(fnode == ctx->min){
			ctx->min = node;
		}

		ltree_insert_and_rebalance(ctx,node);
		++ctx->size;
		return ltree_ok;
	}
	return ltree_fail;
}


enum ltree_ec ltree_remove(ltree_ctx* ctx,const void* key,const void** old)
{
	int eq;
	ltree_node* fnode = ltree_find_node(ctx,ctx->root,key,&eq);

	if(eq != 0){
		return ltree_notfound;
	}

	ltree_remove_and_rebalance(ctx,fnode);
	*old = fnode->data;
	--ctx->size;
	fnode->left = fnode->right = nullptr;

	fnode->parent = ctx->reserve;
	ctx->reserve = fnode;
	return ltree_ok;
}

enum ltree_ec ltree_get(ltree_ctx* ctx,const void* key,const void** data)
{
	int eq;
	ltree_node* fnode = ltree_find_node(ctx,ctx->root,key,&eq);
	if(eq != 0){
		return ltree_notfound;
	}

	*data = fnode->data;
	return ltree_ok;
}

enum ltree_ec ltree_scan_init(ltree_ctx* ctx,ltree_iter** iter)
{
	*iter = malloc(sizeof(ltree_iter));
	if(*iter == nullptr){
		return ltree_oom;
	}

	(*iter)->ctx = ctx;
	(*iter)->cur = (ltree_node*)0x00000001;
	return ltree_ok;
}

enum ltree_ec ltree_scan_seek(struct ltree_iter* iter,const void* key,const void** data)
{
	int eq;
	ltree_node* fnode = ltree_find_node(iter->ctx,iter->ctx->root,key,&eq);
	if(eq > 0 && fnode){
		fnode =  ltree_get_next_node(iter->ctx,fnode);
	}

	iter->cur = fnode;
	if(fnode){
		*data = fnode->data;
		return ltree_ok;
	}
	return ltree_notfound;
}

enum ltree_ec ltree_scan_next(ltree_iter* iter,const void** data)
{
	ltree_ctx* ctx = iter->ctx;
	if(iter->cur == (ltree_node*)0x00000001){
		iter->cur = ctx->min;
		if(iter->cur == nullptr) {
			return ltree_end;
		}
		*data = iter->cur->data;
		return ltree_ok;
	}else if(iter->cur == nullptr){
		return ltree_end;
	}

	if(iter->cur == ctx->max){
		return ltree_end;
	}
	
	iter->cur =  ltree_get_next_node(ctx,iter->cur);
	*data = iter->cur->data;
	return ltree_ok;
}


enum ltree_ec ltree_scan_last(ltree_iter* iter,const void** data)
{
	ltree_ctx* ctx = iter->ctx;
	if(iter->cur == (ltree_node*)0x00000001){
		iter->cur = ctx->max;
		if(iter->cur == nullptr) {
			return ltree_end;
		}
		*data = iter->cur->data;
		return ltree_ok;
	}else if(iter->cur == nullptr){
		return ltree_end;
	}

	if(iter->cur == ctx->min){
		return ltree_end;
	}
	
	iter->cur =  ltree_get_last_node(ctx,iter->cur);
	*data = iter->cur->data;
	return ltree_ok;
}

enum ltree_ec ltree_scan_destory(ltree_iter* iter)
{
	free(iter);
	return ltree_ok;
}

static enum ltree_ec ltree_verbose_node(ltree_node* node,long depth,long idx,ltree_lookup lookup_call_back,void* userdata)
{
	lookup_call_back(node,depth,idx,userdata);

	if(node->left){
		ltree_verbose_node(node->left,depth + 1,idx * 2,lookup_call_back,userdata);
	}
	if(node->right){
		ltree_verbose_node(node->right,depth + 1,idx * 2 + 1,lookup_call_back,userdata);
	}
	return ltree_ok;
}

enum ltree_ec ltree_verbose(struct ltree_ctx* ctx,ltree_lookup lookup_call_back,void* userdata)
{
	ltree_node* node = ctx->root;

	if(node == nullptr){
		return ltree_notfound;
	}

	return ltree_verbose_node(node,0,0,lookup_call_back,userdata);
}
