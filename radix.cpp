#include <iostream>
#include <cstring>
#include "radix.h"

using  namespace std;

radix::radix(uint64_t offset,uint64_t size, mytuple *r,mytuple *_r,int byte){
    this->offset=offset;
    this->size=size;
    R=r;
    _R=_r;
    this->byte=byte;
}

void radix::sort() {
    stack *Stack=new stack();
    Stack->push(this);
    radix *currentRadix;
    while(Stack->notEmpty()){
        currentRadix=Stack->pop();
        currentRadix->group();
        currentRadix->split(Stack);
        if(currentRadix!=this) delete currentRadix;
    }
    delete Stack;

}

//just a wrapper
void radix::group() {
    histogram();
    prefixSum();
    reorderR();
}

void radix::histogram() {
    memset(Hist, 0, N);
    for(int64_t i=0; i<size; i++) Hist[hash(R[i].value)]++;
}

void radix::prefixSum() {
    memset(Psum, 0, N);
    for(int64_t i=1; i<N; i++) Psum[i]=Hist[i-1]+Psum[i-1];

}

void radix::reorderR() {
    uint32_t index,counter[N];
    //memset sets 4 bytes - not 8. so counter is of type *uint
    //memset(counter, 0, N); 
    for (int i = 0; i < N; i++)
        counter[i] = 0;
    for(int64_t i=offset; i<size; i++){
        index=hash(R[i].value);
        _R[Psum[index]+counter[index]]=R[i];
        counter[index]++;
    }
}


uint64_t radix::hash(uint64_t value) {
    uint64_t shift=(8-byte)*8;
    value=value>>shift;
    value=value & 0xFF;
    return value;
}



bool radix::fitsCache(uint64_t i) {
    return Hist[i]*sizeof(uint64_t)<5;
}

void radix::split(stack *Stack) {
    for(int64_t i=0; i<N; i++){
        if(fitsCache(i) || byte==7){
            quicksort(Psum[i],Psum[i]+Hist[i]-1);
            for(int j=Psum[i]; j<Psum[i]+Hist[i]; j++)
               R[j]=_R[j];
        }
        else Stack->push(new radix(Psum[i],Hist[i],_R,R,byte+1));
    }
}

void radix::quicksort(int start, int end) {
    if(start>=end) return;
    int pi=partition(start,end);
    quicksort(start,pi-1);
    quicksort(pi+1,end);
}

int radix::partition(int start, int end) {
    mytuple pivot=_R[end];
    int i=start-1;
    mytuple temp;

    for(int j=start; j<end; j++){
        if(_R[j].value<pivot.value){
            i++;
            temp=_R[i];
            _R[i]=_R[j];
            _R[j]=temp;
        }
    }
    temp=_R[i+1];
    _R[i+1]=_R[end];
    _R[end]=temp;
    return i+1;
}

void radix::print() {
    for(int64_t i=0; i<size; i++){
        cout<<R[i].value<<"\t"<<_R[i].value<<endl;
    }
}

