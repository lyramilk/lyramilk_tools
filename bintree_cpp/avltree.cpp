#include "avltree.h"
#include <malloc.h>
#include <memory.h>

namespace lyramilk{ namespace bintree
{

	avlnode* avltree::find_node_and_rank (const void* data,int* eq,unsigned long* rank)
	{
		avlnode* cur = root;
		int tr;
		unsigned long cc = 0;

		while(1){
			tr = this->compator(data,cur->data);
			if(tr == 0){
				if(cur->left != (avlseq)-1){
					cc += (root + cur->left)->cc;
				}
				break;
			}else if(tr < 0){
				if(cur->left == (avlseq)-1) break;
				cur = root + cur->left;
			}else if(tr > 0){
				if(cur->right == (avlseq)-1) break;
				if(cur->left != (avlseq)-1){
					cc += (root + cur->left)->cc + 1;
				}else{
					++cc;
				}
				cur = root + cur->right;
			}
		}
		*eq = tr;
		*rank = cc;
		return cur;
	}

	avlnode* avltree::find_node(const void* data,int* eq)
	{
		avlnode* cur = root;
		int tr;

		while(1){
			tr = this->compator(data,cur->data);
			if(tr == 0){
				break;
			}else if(tr < 0){
				if(cur->left == (avlseq)-1) break;
				cur = root + cur->left;
			}else if(tr > 0){
				if(cur->right == (avlseq)-1) break;
				cur = root + cur->right;
			}
		}
		*eq = tr;
		return cur;
	}

	avlnode* avltree::get_min_node()
	{
		avlnode* cur = this->root;
		while(cur->left != (avlseq)-1) cur = this->root + cur->left;
		return cur;
	}

	avlnode* avltree::get_max_node()
	{
		avlnode* cur = this->root;
		while(cur->right != (avlseq)-1) cur = this->root + cur->right;
		return cur;
	}

	avlnode* avltree::get_next_node(avlnode* node)
	{
		if(node->right != (avlseq)-1){
			node = this->root + node->right;
			while(node->left != (avlseq)-1) node = this->root + node->left;
		}else if(node->parent != (avlseq)-1){
			avlnode* parent = this->root + node->parent;
			while(node->parent != (avlseq)-1 && node == this->root + parent->right){
				node = this->root + node->parent;
				parent = this->root + parent->parent;
			}
			node = parent;
		}else{
			return nullptr;
		}
		return node;
	}

	avlnode* avltree::get_last_node(avlnode* node)
	{
		if(node->left != (avlseq)-1){
			node = this->root + node->left;
			while(node->right != (avlseq)-1) node = this->root + node->right;
		}else if(node->parent != (avlseq)-1){
			avlnode* parent = this->root + node->parent;
			while(node->parent != (avlseq)-1 && node == this->root + parent->left){
				node = this->root + node->parent;
				parent = this->root + parent->parent;
			}
			node = parent;
		}else{
			return nullptr;
		}
		return node;
	}

	int avltree::compator(const void* a,const void* b)
	{
		return this->cmpr->compare(a,b);
	}

	avltree::avltree(datacompare* cmpr)
	{
		this->cmpr = cmpr;
		root = nullptr;
		min = (avlseq)-1;
		max = (avlseq)-1;
		needfree = false;
	}

	avltree::~avltree()
	{
		if(needfree && root) free(root);
	}


	ec avltree::index(unsigned long idx,const void** data)
	{
		avlnode* cur = this->root;
		if(cur == nullptr || idx >= cur->cc){
			return ec_notfound;
		}

		while(1){
			unsigned long cseq = cur->left != ((avlseq)-1)?(root + cur->left)->cc:0;
			if(idx == cseq){
				*data = cur->data;
				return ec_ok;
			}else if(idx < cseq){
				cur = root + cur->left;
			}else{
				cur = root + cur->right;
				idx -= cseq + 1;
			}
		}
		return ec_notfound;

	}

	ec avltree::rank(const void* key,unsigned long* rank)
	{
		int eq;
		if(this->root == nullptr) return ec_notfound;
		avlnode* fnode = find_node_and_rank(key,&eq,rank);
		if(eq != 0){
			return ec_notfound;
		}

		if(fnode == nullptr) return ec_notfound;
		return ec_ok;
	}

	ec avltree::get(const void* key,const void** data)
	{
		int eq;
		avlnode* fnode = find_node(key,&eq);
		if(eq != 0){
			return ec_notfound;
		}

		*data = fnode->data;
		return ec_ok;
	}

	unsigned long avltree::size()
	{
		return _size;
	}

	static void avltree_cc_incr(avlnode* base,avlnode* node,unsigned long df)
	{
		while(node){
			node->cc += df;
			if(node->parent == (avlseq)-1) break;
			node = base + node->parent;
		}
	}

	ec avltree::from(viterator* iter,unsigned long datacount)
	{

		// p代表最下层有多少个节点
		// q代表除p外其它层有多少个节点
		// l代表层数

		if(needfree) free(root);
		root = (avlnode*)malloc(sizeof(avlnode) * datacount);
		needfree = true;
		_size = datacount;

		for(unsigned long idx=0;idx < _size;++idx){
			avlnode* node = root + idx;
			node->cc = 0;
			unsigned long kl = (idx<<1) + 1;
			unsigned long kr = (idx<<1) + 2;

			if(kl < _size){
				node->left = kl;
				avlnode* left = root + kl;
				left->parent = idx;
			}else{
				node->left = (avlseq)-1;
			}

			if(kr < _size){
				node->right = kr;
				avlnode* right = root + kr;
				right->parent = idx;
			}else{
				node->right = (avlseq)-1;
			}
		}
		root->parent = (avlseq)-1;
		iter->reset();

		avlnode* cur = nullptr;

		const void* data;
		if(iter->next(&data) == ec_ok){
			cur = get_min_node();
			cur->data = data;
			avltree_cc_incr(root,cur,1);
		}

		min = get_min_node() - root;
		max = get_max_node() - root;

		while(iter->next(&data) == ec_ok){
			cur = get_next_node(cur);
			cur->data = data;
			avltree_cc_incr(root,cur,1);
		}
		return ec_ok;
	}

	ec avltree::mmap(const void* ptr,unsigned long len)
	{
		if(needfree) free(root);
		needfree = false;
		unsigned long datacount = len / sizeof(avlnode);
		root = (avlnode*)ptr;
		_size = datacount;

		min = get_min_node() - root;
		max = get_max_node() - root;
		return ec_ok;
	}

	void* avltree::ptr()
	{
		return root;
	}

	unsigned long avltree::bytescount()
	{
		return sizeof(avlnode) * _size;
	}

	class avl_iterator:public viterator
	{
		avltree* ctx;
		avlnode* cur;
	  public:
		avl_iterator(avltree* ctx)
		{
			this->ctx = ctx;
			this->cur = (avlnode*)0x1;
		}
		virtual ~avl_iterator()
		{
		}

		virtual ec reset()
		{
			this->cur = (avlnode*)0x1;
			if(ctx != nullptr)return ec_ok;
			return ec_fail;
		}

		virtual ec seek(const void* key,const void** data)
		{
			int eq;
			avlnode* fnode = ctx->find_node(key,&eq);
			if(eq > 0 && fnode){
				fnode =  ctx->get_next_node(fnode);
			}

			this->cur = fnode;
			if(fnode){
				*data = fnode->data;
				return ec_ok;
			}
			return ec_notfound;
		}

		virtual ec seekpos(unsigned long rank,const void** data)
		{
			avlnode* cur = ctx->root;

			if(cur == nullptr || rank >= cur->cc){
				return ec_notfound;
			}

			while(1){
				unsigned long cseq = cur->left != ((avlseq)-1)?(ctx->root + cur->left)->cc:0;
				if(rank == cseq){
					this->cur = cur;
					*data = this->cur->data;
					return ec_ok;
				}else if(rank < cseq){
					cur = ctx->root + cur->left;
				}else{
					cur = ctx->root + cur->right;
					rank -= cseq + 1;
				}
			}
			return ec_notfound;
		}

	  	virtual ec next(const void** data)
		{
			if(this->cur == (avlnode*)0x1){
				if(ctx->min == (avlseq)-1) {
					return ec_end;
				}
				this->cur = ctx->root + ctx->min;
				*data = this->cur->data;
				return ec_ok;
			}else if(this->cur == nullptr){
				return ec_end;
			}

			if(this->cur == ctx->root + ctx->max){
				return ec_end;
			}
			
			this->cur = ctx->get_next_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}

	  	virtual ec last(const void** data)
		{
			if(this->cur == (avlnode*)0x1){
				if(ctx->max == (avlseq)-1) {
					return ec_end;
				}
				this->cur = ctx->root + ctx->max;
				*data = this->cur->data;
				return ec_ok;
			}else if(this->cur == nullptr){
				return ec_end;
			}

			if(this->cur == ctx->root + ctx->min){
				return ec_end;
			}
			
			this->cur =  ctx->get_last_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}
	};



	viterator* avltree::create_iterator()
	{
		avl_iterator* p = new avl_iterator(this);
		return p;
	}



	void avltree::destory_iterator(viterator* iter)
	{
		avl_iterator* p = (avl_iterator*)iter;
		delete p;
	}


}}
