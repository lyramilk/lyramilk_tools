#ifndef _lyramilk_ctools_tree_h_
#define _lyramilk_ctools_tree_h_

namespace lyramilk{ namespace bintree
{
	class datacompare
	{
	  public:
		virtual int compare(const void* a,const void* b) = 0;
	};


	enum ec{
		ec_ok,			//成功
		ec_update,		//插入时发现己存在，所以更新
		ec_fail,		//失败不解释
		ec_oom,			//malloc失败
		ec_notfound,	//没有找到
		ec_end,			//迭代完成
	};

	class viterator
	{
	  public:
		virtual ec reset() = 0;
		virtual ec seek(const void* key,const void** data) = 0;
		virtual ec seekpos(unsigned long rank,const void** data) = 0;
	  	virtual ec next(const void** data) = 0;
	  	virtual ec last(const void** data) = 0;
	};

	struct bucket
	{
		bucket* next;
		char pool[0];
	};

	struct treenode
	{
		const void* data;
		struct treenode* left;
		struct treenode* right;
		struct treenode* parent;
	};

	typedef bool (*tree_lookup)(treenode* node,int depth,int idx,void* userdata);

	class vtree
	{
	  public:
		virtual ec index(unsigned long idx,const void** data) = 0;
		virtual ec rank(const void* key,unsigned long* rank) = 0;
		virtual ec get(const void* key,const void** data) = 0;
		virtual unsigned long size() = 0;

		virtual viterator* create_iterator() = 0;
		virtual void destory_iterator(viterator* iter) = 0;
	};


	class tree:public vtree
	{
		friend class viterator_impl;
		tree(const tree& o);
	  protected:
		const unsigned long nodesize;
	  protected:
		struct treenode* root;
		struct treenode* min;
		struct treenode* max;
		datacompare* cmpr;
		unsigned long _size;
		unsigned long _capacity;

		struct bucket* mpool;
		struct treenode* reserve;
	  protected:
		static treenode* find_node (treenode* cur,const void* data,int* eq,datacompare* cmpr);
		treenode* new_node(treenode* parent,const void* data);
		ec reserve_node(unsigned long size);

		virtual void rotate_left(treenode* n);
		virtual void rotate_right(treenode* r);

		virtual treenode* get_max_node();
		virtual treenode* get_min_node();
		virtual treenode* get_next_node(treenode* node);
		virtual treenode* get_last_node(treenode* node);
		virtual void detach_node(treenode* node);

		tree(datacompare* cmpr,unsigned long nodesize);
		virtual ec index(unsigned long idx,const void** data);
		virtual ec rank(const void* key,unsigned long* rank);
	  public:
		tree(datacompare* cmpr);
	  	virtual ~tree();

		virtual ec insert(const void* data,const void** old);
		virtual ec remove(const void* key,const void** old);
		virtual ec get(const void* key,const void** data);

		virtual unsigned long size();
		virtual unsigned long capacity();

		virtual enum ec foreach(tree_lookup lookup_call_back,void* userdata);

		virtual viterator* create_iterator();
		virtual void destory_iterator(viterator* iter);
	};
}}
#endif
