#include "tree.h"
#include <malloc.h>

namespace lyramilk{ namespace bintree
{

	static int tree_poniter_compare(const void* a,const void* b)
	{
		if(a > b) return 1;
		if(a < b) return 1;
		return 0;
	}

	treenode* tree::find_node(treenode* cur,const void* data,int* eq,datacompare* cmpr)
	{
		int tr;

		while(1){
			tr = cmpr->compare(data,cur->data);
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

	treenode* tree::new_node(treenode* parent,const void* data)
	{
		treenode* node = nullptr;

		if(this->reserve){
			node = this->reserve;
			this->reserve = node->parent;
		}else{
			if(this->_capacity == 0){
				reserve_node(32);
			}
			char* p = this->mpool->pool + nodesize * (this->_capacity - 1);
			node = (struct treenode*)p;
		}
		--this->_capacity;
		node->left = node->right = nullptr;
		node->parent = parent;
		node->data = data;
		return node;

	}

	ec tree::reserve_node(unsigned long size)
	{
		while(this->_capacity > 0){
			treenode* node = new_node(this->reserve,nullptr);
			this->reserve = node;
		}

		this->_capacity = size;
		bucket* first_bucket = (bucket*)malloc(sizeof(bucket) + (nodesize * this->_capacity));
		if(first_bucket == nullptr){
			return ec_oom;
		}
		first_bucket->next = nullptr;
		first_bucket->next = this->mpool;
		this->mpool = first_bucket;
		return ec_ok;
	}

	void tree::rotate_left(treenode* n)
	{
		treenode* r = n->right;
		r->parent = n->parent;
		if(r->parent == nullptr){
			this->root = r;
		}else if(r->parent->left == n){
			r->parent->left = r;
		}else{
			r->parent->right = r;
		}

		n->right = r->left;
		if(n->right){
			n->right->parent = n;
		}

		r->left = n; 
		n->parent = r;
	}

	void tree::rotate_right(treenode* r)
	{
		treenode* n = r->left;
		n->parent = r->parent;
		if(n->parent == nullptr){
			this->root = n;
		}else if(n->parent->left == r){
			n->parent->left = n;
		}else{
			n->parent->right = n;
		}

		r->left = n->right;
		if(r->left){
			r->left->parent = r;
		}

		n->right = r;
		r->parent = n;
	}

	treenode* tree::get_min_node()
	{
		treenode* cur = this->root;
		while(cur->left) cur = cur->left;
		return cur;
	}

	treenode* tree::get_max_node()
	{
		treenode* cur = this->root;
		while(cur->right) cur = cur->right;
		return cur;
	}

	treenode* tree::get_next_node(treenode* node)
	{
		if(node->right){
			node = node->right;
			while(node->left) node = node->left;
		}else if(node->parent){
			treenode* parent = node->parent;
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

	treenode* tree::get_last_node(treenode* node)
	{
		if(node->left){
			node = node->left;
			while(node->right) node = node->right;
		}else if(node->parent){
			treenode* parent = node->parent;
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

	tree::tree(datacompare* cmpr,unsigned long _nodesize):nodesize(_nodesize)
	{
		this->cmpr = cmpr;

		root = min = max = nullptr;
		_size = 0;
		reserve = nullptr;
		_capacity = 0;
		mpool = nullptr;
	}

	tree::tree(datacompare* cmpr):nodesize(sizeof(treenode))
	{
		this->cmpr = cmpr;

		root = min = max = nullptr;
		_size = 0;
		reserve = nullptr;
		_capacity = 0;
		mpool = nullptr;
	}

	tree::~tree()
	{
		if(mpool){
			while(mpool->next){
				bucket* next = mpool->next;
				free(mpool);
				mpool = next;
			}
			free(mpool);
		}
	}

	ec tree::insert(const void* data,const void** old)
	{
		if(this->root == nullptr){
			treenode* node = new_node(nullptr,data);
			if(node == nullptr) return ec_oom;
			this->min = this->max = this->root = node;
			this->_size = 1;
			return ec_ok;
		}

		//插入
		int eq;
		treenode* fnode = find_node(this->root,data,&eq,cmpr);

		if(eq == 0){
			if(old) *old = fnode->data;
			fnode->data = data;
			return ec_update;
		}

		if(eq > 0){
			treenode* node = new_node(fnode,data);
			if(node == nullptr) return ec_oom;
			fnode->right = node;
			if(fnode == this->max){
				this->max = node;
			}
			++this->_size;
			return ec_ok;
		}else{
			treenode* node = new_node(fnode,data);
			if(node == nullptr) return ec_oom;
			fnode->left = node;
			if(fnode == this->min){
				this->min = node;
			}

			++this->_size;
			return ec_ok;
		}
		return ec_fail;
	}

	ec tree::index(unsigned long idx,const void** data)
	{
		return ec_fail;
	}

	ec tree::rank(const void* key,unsigned long* rank)
	{
		return ec_fail;
	}

	void tree::detach_node(treenode* oldnode)
	{
		treenode* substitute = oldnode;
		treenode* node = nullptr;

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
				if (node) node->parent = substitute->parent;
				substitute->parent->left = node;
				substitute->right = oldnode->right;
				oldnode->right->parent = substitute;
			}
			if (this->root == oldnode){
				this->root = substitute;
			}else if (oldnode->parent->left == oldnode){
				oldnode->parent->left = substitute;
			}else{
				oldnode->parent->right = substitute;
			}
			substitute->parent = oldnode->parent;
			substitute = oldnode;
		}else{
			if (node){
				node->parent = substitute->parent;
			}
			if (this->root == oldnode){
				this->root = node;
			}else{
				if (oldnode->parent->left == oldnode){
					oldnode->parent->left = node;
				}else{
					oldnode->parent->right = node;
				}
			}
			if (this->min == oldnode){
				if (oldnode->right == nullptr){
					this->min = oldnode->parent;
				}else{
					this->min = get_min_node();
				}
			}
			if (this->max == oldnode){
				if (oldnode->left == nullptr){
					this->max = oldnode->parent;
				}else{
					this->max = get_max_node();
				}
			}
		}

	}

	ec tree::remove(const void* key,const void** old)
	{
		int eq;
		treenode* fnode = find_node(this->root,key,&eq,cmpr);

		if(eq != 0){
			return ec_notfound;
		}

		detach_node(fnode);
		*old = fnode->data;
		--this->_size;

		fnode->left = fnode->right = nullptr;
		fnode->parent = this->reserve;
		this->reserve = fnode;
		return ec_ok;
	}

	ec tree::get(const void* key,const void** data)
	{
		int eq;
		treenode* fnode = find_node(this->root,key,&eq,cmpr);
		if(eq != 0){
			return ec_notfound;
		}

		*data = fnode->data;
		return ec_ok;
	}


	unsigned long tree::size()
	{
		return this->_size;
	}

	unsigned long tree::capacity()
	{
		return this->_capacity + this->_size;
	}


	static ec foreach_node(treenode* node,unsigned long depth,unsigned long idx,tree_lookup lookup_call_back,void* userdata)
	{
		if(!lookup_call_back(node,depth,idx,userdata)){
			return ec_fail;
		}

		if(node->left){
			if(foreach_node(node->left,depth + 1,idx * 2,lookup_call_back,userdata) != ec_ok){
				return ec_fail;
			}
		}
		if(node->right){
			if(foreach_node(node->right,depth + 1,idx * 2 + 1,lookup_call_back,userdata) != ec_ok){
				return ec_fail;
			}
		}
		return ec_ok;
	}


	ec tree::foreach(tree_lookup lookup_call_back,void* userdata)
	{
		treenode* node = this->root;

		if(node == nullptr){
			return ec_notfound;
		}

		return foreach_node(node,0,0,lookup_call_back,userdata);
	}

	class viterator_impl:public viterator
	{
		tree* ctx;
		treenode* cur;
	  public:
		viterator_impl(tree* ctx)
		{
			this->ctx = ctx;
			this->cur = (treenode*)0x1;
		}
		virtual ~viterator_impl()
		{
		}

		virtual ec reset()
		{
			this->cur = (treenode*)0x1;
		}

		virtual ec seek(const void* key,const void** data)
		{
			int eq;
			treenode* fnode = ctx->find_node(ctx->root,key,&eq,ctx->cmpr);
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
			cur = (treenode*)0x1;

			for(unsigned long idx =0;next(data) == ec_ok;++idx){
				if(idx == rank){
					return ec_ok;
				}
			}
			return ec_notfound;
		}

	  	virtual ec next(const void** data)
		{
			if(this->cur == (treenode*)0x1){
				this->cur = ctx->min;
				if(this->cur == nullptr) {
					return ec_end;
				}
				*data = this->cur->data;
				return ec_ok;
			}else if(this->cur == nullptr){
				return ec_end;
			}

			if(this->cur == ctx->max){
				return ec_end;
			}
			
			this->cur =  ctx->get_next_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}

	  	virtual ec last(const void** data)
		{
			if(this->cur == (treenode*)0x1){
				this->cur = ctx->max;
				if(this->cur == nullptr) {
					return ec_end;
				}
				*data = this->cur->data;
				return ec_ok;
			}else if(this->cur == nullptr){
				return ec_end;
			}

			if(this->cur == ctx->min){
				return ec_end;
			}
			
			this->cur =  ctx->get_last_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}
	};



	viterator* tree::create_iterator()
	{
		viterator_impl* p = new viterator_impl(this);
		return p;
	}



	void tree::destory_iterator(viterator* iter)
	{
		viterator_impl* p = (viterator_impl*)iter;
		delete p;
	}

}}
