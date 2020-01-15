#include "query.h"
#include "sort.h"
#include "radix.h"

Query::Query(Relations *rels, SQL *s,uint64_t *&sm):sums(sm) {
    relations=rels;
    sql=s;
    sums= NULL;
    results= nullptr;
    max=0;
    type=create;
}

int Query::isRelationFiltered(int relation) {
    for (int j = 0; j < max; j++) {
        if (filter_results[j]->exists(relation)) {
            return j;
        }
    }
    return -1;
}

bool Query::execute_filter(Predicate *predicate) {
    bool exists;
    int curr;
    exists = false;
    if ((curr = isRelationFiltered(predicate->get_array())) >= 0) exists = true;
    if (!exists) {
        curr = max;
        filter_results[curr] = new JoinArray(relations);
        max++;
    }
    if (predicate->is_comparison()) {
        int comparison = predicate->get_comp();
        if (comparison == '=') {
            if (exists) filter_results[curr]->equal(predicate->get_column(), predicate->get_value());
            else
                filter_results[curr]->create_array(
                        relations->equal(predicate->get_array(), predicate->get_column(), predicate->get_value()),
                        predicate->get_array());
        } else if (comparison == '>') {
            if (exists) filter_results[curr]->grater_than(predicate->get_column(), predicate->get_value());
            else
                filter_results[curr]->create_array(
                        relations->grater_than(predicate->get_array(), predicate->get_column(),
                                               predicate->get_value()), predicate->get_array());
        } else {
            if (exists) filter_results[curr]->less_than(predicate->get_column(), predicate->get_value());
            else
                filter_results[curr]->create_array(
                        relations->less_than(predicate->get_array(), predicate->get_column(),
                                             predicate->get_value()), predicate->get_array());
        }
        return filter_results[curr]->getSize() != 0;
    }
    if (exists)
        filter_results[curr]->compare(predicate->get_array(), predicate->get_column(), predicate->get_column2());
    else
        filter_results[curr]->create_array(
                relations->filter(predicate->get_array(), predicate->get_column(), predicate->get_column2()),
                predicate->get_array());
    return true;
}

bool Query::execute_filters() {
    int filters = sql->get_filters_num();
    filter_results = new JoinArray *[filters];
    Predicate *predicate = nullptr;
    for (int i = 0; i < filters; i++) { //filters arrays
        predicate = sql->getPredicate();  //next filter
        if(!execute_filter(predicate)){
            delete predicate;
            return false;
        }
        delete predicate;
    }
    return true;
}

void Query::DoQuery(bool filters) {
    Predicate *predicate = nullptr;
    if(results) results->keep_new_results();
    if(type==update_filtered){
        delete filter_results[max-1];
        max--;
    }
    type=create;
    if (filters && (predicate = sql->getPredicate())) {  //joins between relation
        if (predicate->is_filter()) {
            results->compare(predicate->get_array(), predicate->get_column(), predicate->get_array2(),
                             predicate->get_column2());
            delete predicate;
            DoQuery(results->getSize()>0);
            return;
        }
        int array1 = predicate->get_array(), array2 = predicate->get_array2();
        if (!results) {
            int curr = isRelationFiltered(array1);
            if (curr != -1) {
                results = filter_results[curr];
                filter_results[curr] = filter_results[max - 1];
                max--;
            }
            else {
                curr = isRelationFiltered(array2);
                if (curr != -1) {
                    results = filter_results[curr];
                    filter_results[curr] = filter_results[max - 1];
                    max--;
                }
            }
        }
        if (results && results->exists(array1)) {
            int curr = isRelationFiltered(array2);
            if (curr == -1) {
                if (predicate->getSorted())
                    results->sortedJoin(array1, predicate->get_column(), array2, predicate->get_column2(),this);
                else results->Join(array1, predicate->get_column(), array2, predicate->get_column2(),this);
            }
            else {
                if (predicate->getSorted())
                    results->sortedJoin(array1, predicate->get_column(), filter_results[curr], array2,
                                              predicate->get_column2(),this);
                else
                    results->Join(array1, predicate->get_column(), filter_results[curr], array2,
                                        predicate->get_column2(),this);
            }
            delete predicate;
            return;
        }
        if (results && results->exists(array2)) {
            int curr = isRelationFiltered(array1);
            if (curr == -1) {
                if (predicate->getSorted())
                    results->sortedJoin(array2, predicate->get_column2(), array1, predicate->get_column(),this);
                else results->Join(array2, predicate->get_column2(), array1, predicate->get_column(),this);
            } else {
                if (predicate->getSorted())
                    results->sortedJoin(array2, predicate->get_column2(), filter_results[curr], array1,
                                              predicate->get_column(),this);
                else
                    results->Join(array2, predicate->get_column2(), filter_results[curr], array1,
                                        predicate->get_column(),this);
            }
            delete predicate;
            return;
        }
        sem_t *sem=new sem_t;
        sem_init(sem, 1, 0);
        auto arr1 = sort(sem,this,
                new radix(relations->get_relRows(array1), relations->get_column(array1, predicate->get_column())));
        auto arr2 = sort(sem,this,
                new radix(relations->get_relRows(array2), relations->get_column(array2, predicate->get_column2())));
        if(array1<array2)
            js->Schedule(new MergeJob(this,arr1,arr2,true,relations->get_column(array1,predicate->get_column()),relations->get_column(array2,predicate->get_column2()),array1,array2),sem,2*NUMJOBS);
        else
            js->Schedule(new MergeJob(this,arr2,arr1,true,relations->get_column(array2,predicate->get_column2()),relations->get_column(array1,predicate->get_column()),array2,array1),sem,2*NUMJOBS);
        delete predicate;
    }
    //joinPredicates(filter_results,sql,relations,max);
    if (!results ) {
        for(int i=0; i<max; i++) delete filter_results[i];
        delete[] filter_results;
        results = filter_results[0];
        return;
    }
    int res_counter = sql->get_results_counter();
    sums = new uint64_t[res_counter];
    set *select = sql->get_select();
    for (int i = 0; i < res_counter; i++) {
        sums[i] = results->get_sum(select[i].getArray(), select[i].getColumn());
    }
    delete[] filter_results;
    delete results;
}

void Query::add_joined_array(uint64_t size, int array1, int array2) {
    if(!results){
        results = new JoinArray(relations);
        if(array1<array2) results->add_relations(array1, array2,size);
        else results->add_relations(array2, array1,size);
        type=create;
    }
    else if(results->exists(array1)){
        int curr = isRelationFiltered(array2);
        if (curr == -1) {
            results->add_relation(array2,size);
            type=update;
        }
        else {
            results->add_relation(array2,size);
            JoinArray *temp=filter_results[curr];
            filter_results[curr] = filter_results[max - 1];
            filter_results[max - 1]=temp;
            type=update_filtered;
        }
    }
    else if(results->exists(array2)){
        int curr = isRelationFiltered(array1);
        if (curr == -1) {
            results->add_relation(array1,size);
            type=update;
        }
        else {
            results->add_relation(array1,size);
            type=update_filtered;
            JoinArray *temp=filter_results[curr];
            filter_results[curr] = filter_results[max - 1];
            filter_results[max - 1]=temp;
        }
    }
}

void Query::update_array(list *res, uint64_t offset) {
    if(type==create){
        results->create_array(res,offset);
    }
    else if(type==update){
        results->update_array(res,offset);
    }
    else{
        results->update_array(res,filter_results[max-1],offset);
    }
}

void Query::update_array_element(uint64_t num1, uint64_t num2, uint64_t i) {
    if(type==create){
        results->set_array(i,num1,num2);
    }
    else if(type==update){
        results->set_newArray(i,num1,num2);
    }
    else{
        results->set_newArray(i,num1,num2,filter_results[max-1]);
    }
}