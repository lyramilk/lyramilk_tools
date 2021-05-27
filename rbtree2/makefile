.PHONY: test

test: ./test_rbtree2
	./test_rbtree2


./test_rbtree2: test_rbtree2.cc rbtree2.o rbtree2.h
	g++ -O2 -DNDEBUG -o $@ $^

rbtree2.o:rbtree2.c rbtree2.h
	gcc -O2 -DNDEBUG -c -o $@  $<
