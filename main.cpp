#include<iostream>
#include<fstream>
#include <cstdlib>
#include <string>
#include "radix.h"
#include "stack.h"
#include "inputFunctions.h"

using namespace std;

list *finalresults(mytuple *array1,mytuple *array2, uint64_t size1,uint64_t size2){
    list *resultlist=new list();
    uint64_t start=0,j;
    for(uint64_t i=0; i<size1; i++){
        for(j=start; j<size2; j++){
            if(array1[i].value==array2[j].value)
                resultlist->add(array1[i].index,array2[j].index);
            else if(array1[i].value<array2[j].value)
                break;
        }
        if(i!=size1-1 && array1[i].value!=array1[i+1].value)  start=j;
    }
    return resultlist;
}

int main(int argc, char *argv[]) {
    /*int count1=0,count2=0;
    struct mytuple *array1,*array2,*_array1,*_array2;
    int n=25;
    string filename;
    //run under build dir generated by cmake
	filename="../text.txt";
    if(argc>1)
		filename=argv[1];
    if(argc>2){
		n=stoi(argv[2]);
		if(2<<(n-1)<0)
			n=20;
	}
    if(!getArraySize(filename,count1,count2)) {
        cout<<"could not open file.\n";
        exit(-1);
    }
    array1=new mytuple[count1];
    array2=new mytuple[count2];
    if(!makearrays(filename,array1,array2)) {
        cout<<"could not open file.\n";
        exit(-1);
    }
    _array1=new mytuple[count1];
    _array2=new mytuple[count2];
    radix r1(0,count1,array1,_array1,1);
    r1.sort();
    radix r2(0,count2,array2,_array2,1);
    r2.sort();
    list *resultlist=finalresults(array1,array2,count1,count2);
    resultlist->print();
    delete[] array1;
    delete[] array2;
    delete[] _array1;
    delete[] _array2;
    delete resultlist;
    return 0;
    */
}
