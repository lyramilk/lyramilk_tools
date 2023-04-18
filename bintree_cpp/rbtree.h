#ifndef _lyramilk_ctools_rbtree_h_
#define _lyramilk_ctools_rbtree_h_
#include "tree.h"

namespace lyramilk{ namespace bintree
{
	enum rbnode_color{
		r_black = 0,
		r_red = 1,
	};

	struct rbnode : treenode
	{
		unsigned long cc:(sizeof(unsigned long)<<3)-1;	//	子结点数
		enum rbnode_color color:1;		//	0黑	1红
	};

	class rbtree:public tree
	{
		bool extrb;
		friend class rbtree_iterator;
		rbtree(const rbtree& o);
	  protected:
		static rbnode* find_node_and_rank (rbnode* cur,const void* data,int* eq,unsigned long* rank,datacompare* cmpr);

		virtual void rotate_left2(rbnode* n);
		virtual void rotate_right2(rbnode* r);

		virtual void insert_fixup(rbnode* node);
		virtual void remove_fixup(rbnode* node);
	  public:
		rbtree(datacompare* cmpr);
	  	virtual ~rbtree();

		virtual ec insert(const void* data,const void** old);
		virtual ec remove(const void* key,const void** old);
		virtual ec index(unsigned long idx,const void** data);
		virtual ec rank(const void* key,unsigned long* rank);

		virtual viterator* create_iterator();
		virtual void destory_iterator(viterator* iter);
	};
}}
#endif
