#include "rbtree.h"
#include <malloc.h>

namespace lyramilk{ namespace bintree
{

	rbnode* rbtree::find_node_and_rank (rbnode* cur,const void* data,int* eq,unsigned long* rank,datacompare* cmpr)
	{
		int tr;
		unsigned long cc = 0;

		while(1){
			tr = cmpr->compare(data,cur->data);
			if(tr == 0){
				if(cur->left){
					cc += ((rbnode*)cur->left)->cc;
				}
				break;
			}else if(tr < 0){
				if(cur->left == nullptr) break;
				cur = (rbnode*)cur->left;
			}else if(tr > 0){
				if(cur->left){
					cc += ((rbnode*)cur->left)->cc + 1;
				}else{
					++cc;
				}
				if(cur->right == nullptr) break;
				cur = (rbnode*)cur->right;
			}
		}
		*eq = tr;
		*rank = cc;
		return cur;
	}

	void rbtree::rotate_left2(rbnode* n)
	{
		rbnode* r = (rbnode*)n->right;

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

		{
			r->cc = n->cc;
			n->cc = 1;
			n->cc += n->left?((rbnode*)n->left)->cc:0;
			n->cc += n->right?((rbnode*)n->right)->cc:0;
		}
	}

	void rbtree::rotate_right2(rbnode* r)
	{
		rbnode* n = (rbnode*)r->left;
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

		{
			n->cc = r->cc;
			r->cc = 1;
			r->cc += r->left?((rbnode*)r->left)->cc:0;
			r->cc += r->right?((rbnode*)r->right)->cc:0;
		}
	}

	void rbtree::insert_fixup(rbnode* node)
	{
		while (node->parent && ((rbnode*)node->parent)->color == r_red){
			rbnode* grandpa = (rbnode*)node->parent->parent;

			if (node->parent == grandpa->left){
				rbnode* uncle = (rbnode*)grandpa->right;
				if (uncle && uncle->color == r_red){
					((rbnode*)node->parent)->color = r_black;
					uncle->color = r_black;
					grandpa->color = r_red;
					node = grandpa;
				}else{
					if (node == (rbnode*)node->parent->right){
						node = (rbnode*)node->parent;
						rotate_left2(node);
					}
					((rbnode*)node->parent)->color = r_black;
					grandpa->color = r_red;
					rotate_right2(grandpa);
				}
			}else{
				rbnode* uncle = (rbnode*)grandpa->left;
				if (uncle && uncle->color == r_red){
					((rbnode*)node->parent)->color = r_black;
					uncle->color = r_black;
					grandpa->color = r_red;
					node = grandpa;
				}else{
					if (node == (rbnode*)node->parent->left){
						node = (rbnode*)node->parent;
						rotate_right2(node);
					}
					((rbnode*)node->parent)->color = r_black;
					grandpa->color = r_red;
					rotate_left2(grandpa);
				}
			}
		}
		((rbnode*)this->root)->color = r_black;
	}

	static void rbtree_cc_incr(rbnode* node,unsigned long df)
	{
		while(node){
			node->cc += df;
			node = (rbnode*)node->parent;
		}
	}


	void rbtree::remove_fixup(rbnode* oldnode)
	{
		rbnode* substitute = oldnode;
		rbnode* node = nullptr;
		rbnode* grandpa = nullptr;

		if (substitute->left == nullptr){
			node = (rbnode*)substitute->right;
		}else{
			if (substitute->right == nullptr)
				node = (rbnode*)substitute->left;
			else{
				substitute = (rbnode*)substitute->right;
				while (substitute->left != nullptr){
					substitute = (rbnode*)substitute->left;
				}
				node = (rbnode*)substitute->right;
			}
		}

		if(extrb) rbtree_cc_incr(substitute,-1);

		if (substitute != oldnode){

			oldnode->left->parent = substitute;
			substitute->left = oldnode->left;
			substitute->cc = oldnode->cc;

			if (substitute != oldnode->right){
				grandpa = (rbnode*)substitute->parent;
				if (node){
					node->parent = substitute->parent;
				}
				substitute->parent->left = node;
				substitute->right = oldnode->right;
				oldnode->right->parent = substitute;
			}else{
				grandpa = substitute;
			}
			if (this->root == oldnode){
				this->root = substitute;
			}else if (oldnode->parent->left == oldnode){
				oldnode->parent->left = substitute;
			}else{
				oldnode->parent->right = substitute;
			}
			substitute->parent = oldnode->parent;
			{
				enum rbnode_color c = oldnode->color;
				oldnode->color = substitute->color;
				substitute->color = c;
			}
			substitute = oldnode;
		}else{
			grandpa = (rbnode*)substitute->parent;
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

		if (substitute->color != r_red){
			while (node != this->root && (node == nullptr || node->color == r_black)){
				if (node == grandpa->left){
					rbnode* uncel = (rbnode*)grandpa->right;
					if (uncel->color == r_red){
						uncel->color = r_black;
						grandpa->color = r_red;
						rotate_left2(grandpa);
						uncel = (rbnode*)grandpa->right;
					}
					if ((uncel->left == nullptr || ((rbnode*)uncel->left)->color == r_black) && (uncel->right == nullptr || ((rbnode*)uncel->right)->color == r_black)) {
						uncel->color = r_red;
						node = grandpa;
						grandpa = (rbnode*)grandpa->parent;
					}else{
						if (uncel->right == nullptr || ((rbnode*)uncel->right)->color == r_black){
							((rbnode*)uncel->left)->color = r_black;
							uncel->color = r_red;
							rotate_right2(uncel);
							uncel = (rbnode*)grandpa->right;
						}
						uncel->color = grandpa->color;
						grandpa->color = r_black;
						if (uncel->right){
							((rbnode*)uncel->right)->color = r_black;
						}
						rotate_left2(grandpa);
						break;
					}
				}else{
					rbnode* uncel = (rbnode*)grandpa->left;
					if (uncel->color == r_red){
						uncel->color = r_black;
						grandpa->color = r_red;
						rotate_right2(grandpa);
						uncel = (rbnode*)grandpa->left;
					}
					if ((uncel->right == nullptr || ((rbnode*)uncel->right)->color == r_black) && (uncel->left == nullptr || ((rbnode*)uncel->left)->color == r_black)){
						uncel->color = r_red;
						node = grandpa;
						grandpa = (rbnode*)grandpa->parent;
					}else{
						if (uncel->left == nullptr || ((rbnode*)uncel->left)->color == r_black){
							((rbnode*)uncel->right)->color = r_black;
							uncel->color = r_red;
							rotate_left2(uncel);
							uncel = (rbnode*)grandpa->left;
						}
						uncel->color = grandpa->color;
						grandpa->color = r_black;
						if (uncel->left){
							((rbnode*)uncel->left)->color = r_black;
						}
						rotate_right2(grandpa);
						break;
					}
				}
			}
			if (node) node->color = r_black;
		}
	}

	rbtree::rbtree(datacompare* cmpr):tree(cmpr,sizeof(rbnode))
	{
		extrb = true;
	}

	rbtree::~rbtree()
	{
	}

	ec rbtree::insert(const void* data,const void** old)
	{
		if(this->root == nullptr){
			rbnode* node = (rbnode*)new_node(nullptr,data);
			if(node == nullptr) return ec_oom;
			node->color = r_black;
			node->cc = 1;
			this->min = this->max = this->root = node;
			this->_size = 1;
			return ec_ok;
		}

		//插入
		int eq;
		rbnode* fnode = (rbnode*)find_node(this->root,data,&eq,cmpr);

		if(eq == 0){
			if(old) *old = fnode->data;
			fnode->data = data;
			return ec_update;
		}

		if(eq > 0){
			rbnode* node = (rbnode*)new_node(fnode,data);
			if(node == nullptr) return ec_oom;
			node->color = r_red;
			node->cc = 0;

			fnode->right = node;
			if(fnode == this->max){
				this->max = node;
			}

			if(extrb) rbtree_cc_incr(node,1);
			insert_fixup(node);

			++this->_size;
			return ec_ok;
		}else{
			rbnode* node = (rbnode*)new_node(fnode,data);
			if(node == nullptr) return ec_oom;
			node->color = r_red;
			node->cc = 0;

			fnode->left = node;
			if(fnode == this->min){
				this->min = node;
			}

			if(extrb) rbtree_cc_incr(node,1);
			insert_fixup(node);

			++this->_size;
			return ec_ok;
		}
		return ec_fail;
	}

	ec rbtree::remove(const void* key,const void** old)
	{
		int eq;
		treenode* fnode = find_node(this->root,key,&eq,cmpr);

		if(eq != 0){
			return ec_notfound;
		}

		remove_fixup((rbnode*)fnode);
		if(old) *old = fnode->data;
		--this->_size;
		fnode->left = fnode->right = nullptr;

		fnode->parent = this->reserve;
		this->reserve = fnode;
		return ec_ok;

	}

	ec rbtree::index(unsigned long idx,const void** data)
	{
		if(!extrb) return tree::index(idx,data);
		rbnode* cur = (rbnode*)this->root;
		if(cur == nullptr || idx >= cur->cc){
			return ec_notfound;
		}

		while(1){
			unsigned long cseq = cur->left?((rbnode*)cur->left)->cc:0;
			if(idx == cseq){
				*data = cur->data;
				return ec_ok;
			}else if(idx < cseq){
				cur = (rbnode*)cur->left;
			}else{
				cur = (rbnode*)cur->right;
				idx -= cseq + 1;
			}
		}
		return ec_notfound;
	}

	ec rbtree::rank(const void* key,unsigned long* rank)
	{
		if(!extrb) return tree::rank(key,rank);
		int eq;
		if(this->root == nullptr) return ec_notfound;
		rbnode* fnode = find_node_and_rank((rbnode*)this->root,key,&eq,rank,cmpr);
		if(eq != 0){
			return ec_notfound;
		}

		if(fnode == nullptr) return ec_notfound;
		return ec_ok;
	}





	class rbtree_iterator:public viterator
	{
		rbtree* ctx;
		rbnode* cur;
	  public:
		rbtree_iterator(rbtree* ctx)
		{
			this->ctx = ctx;
			this->cur = (rbnode*)0x1;
		}
		virtual ~rbtree_iterator()
		{
		}

		virtual ec reset()
		{
			this->cur = (rbnode*)0x1;
			if(ctx != nullptr)return ec_ok;
			return ec_fail;
		}

		virtual ec seek(const void* key,const void** data)
		{
			int eq;
			treenode* fnode = ctx->find_node(ctx->root,key,&eq,ctx->cmpr);
			if(eq > 0 && fnode){
				fnode =  ctx->get_next_node(fnode);
			}

			this->cur = (rbnode*)fnode;
			if(fnode){
				*data = fnode->data;
				return ec_ok;
			}
			return ec_notfound;
		}

		virtual ec seekpos(unsigned long rank,const void** data)
		{
			rbnode* cur = (rbnode*)ctx->root;

			if(cur == nullptr || rank >= cur->cc){
				return ec_notfound;
			}

			while(1){
				unsigned long cseq = cur->left?((rbnode*)cur->left)->cc:0;
				if(rank == cseq){
					this->cur = cur;
					*data = this->cur->data;
					return ec_ok;
				}else if(rank < cseq){
					cur = (rbnode*)cur->left;
				}else{
					cur = (rbnode*)cur->right;
					rank -= cseq + 1;
				}
			}
			return ec_notfound;
		}

	  	virtual ec next(const void** data)
		{
			if(this->cur == (rbnode*)0x1){
				this->cur = (rbnode*)ctx->min;
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
			
			this->cur = (rbnode*)ctx->get_next_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}

	  	virtual ec last(const void** data)
		{
			if(this->cur == (rbnode*)0x1){
				this->cur = (rbnode*)ctx->max;
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
			
			this->cur = (rbnode*)ctx->get_last_node(this->cur);
			*data = this->cur->data;
			return ec_ok;
		}
	};

	viterator* rbtree::create_iterator()
	{
		if(!extrb) return tree::create_iterator();
		rbtree_iterator* p = new rbtree_iterator(this);
		return p;
	}

	void rbtree::destory_iterator(viterator* iter)
	{
		if(!extrb){
			tree::destory_iterator(iter);
			return;
		}
		rbtree_iterator* p = (rbtree_iterator*)iter;
		delete p;
	}

}}
