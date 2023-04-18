#ifndef _lyramilk_ctools_avltree_h_
#define _lyramilk_ctools_avltree_h_
#include "tree.h"

namespace lyramilk{ namespace bintree
{
	typedef unsigned long long avlseq;

	struct avlnode
	{
		const void* data;
		avlseq left;
		avlseq right;
		avlseq parent;
		unsigned long cc;
	};


	class avltree:public vtree
	{
		friend class avl_iterator;
	  protected:
		struct avlnode* root;
		avlseq min;
		avlseq max;
		datacompare* cmpr;
		unsigned long _size;
		bool needfree;
		int compator(const void* a,const void* b);
	  protected:
		virtual avlnode* find_node(const void* data,int* eq);
		virtual avlnode* find_node_and_rank (const void* data,int* eq,unsigned long* rank);

		virtual avlnode* get_max_node();
		virtual avlnode* get_min_node();
		virtual avlnode* get_next_node(avlnode* node);
		virtual avlnode* get_last_node(avlnode* node);
	  public:
		avltree(datacompare* cmpr);
	  	virtual ~avltree();

		virtual ec index(unsigned long idx,const void** data);
		virtual ec rank(const void* key,unsigned long* rank);
		virtual ec get(const void* key,const void** data);
		virtual unsigned long size();

		virtual viterator* create_iterator();
		virtual void destory_iterator(viterator* iter);

		virtual ec from(viterator* iter,unsigned long size);

		// 使avltree将指定内存地址的内容视为自身的数据。
		virtual ec mmap(const void* ptr,unsigned long len);
		virtual void* ptr();
		virtual unsigned long bytescount();
	};
}}
#endif
