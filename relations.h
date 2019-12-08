#ifndef PROJECT_RELATIONS_H
#define PROJECT_RELATIONS_H

#include "relation.h"
#include "list.h"

char* concat(const char *s1, const char *s2);


class Relations {
    Relation **rels;
    int sz;
    public:
    Relations(char *filename);
    Relation *relation(int i);
    int getSize();
    ~Relations();
    array *get_column(int relation,uint64_t column);
    list *filter(int array,uint64_t column1,uint64_t column2);
    list *equal(int array,uint64_t column,uint64_t value);
    list *grater_than(int array,uint64_t column,uint64_t value);
    list *less_than(int array,uint64_t column,uint64_t value);
    bool filter(int array,uint64_t row,uint64_t column1,uint64_t column2);
    bool filter(int array1,int array2,uint64_t row1,uint64_t row2,uint64_t column1,uint64_t column2);
};


#endif