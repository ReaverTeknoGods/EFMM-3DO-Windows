#ifndef USSR_Type_Structs_Classes
#define USSR_Type_Structs_Classes
#include <stddef.h>

//____________________________USSR_List_Node
template <class T> class USSR_List_Node{
public:
 T Data;
 USSR_List_Node<T> *prev,*next;

 USSR_List_Node(){next=prev=NULL;};
 ~USSR_List_Node(){delete next;};
 int DoNext();
};

template <class T> int USSR_List_Node<T>::DoNext()
{
 next=new USSR_List_Node<T>;
 next->prev=this;
 return 0;
}
//____________________________USSR_List_Node_End
//____________________________USSR_String
template <class T> class USSR_String{
public:
 T* Data;
 int size;

 USSR_String(){Data=NULL;size=0;};
 ~USSR_String(){delete []Data;size=0;};
 int Add(T El);
 int Add();
};

template <class T> int USSR_String<T>::Add(T El)
{
 T* tmp;
 int i;

 tmp=new T[size+1];
 for(i=0;i<size;i++)
  tmp[i]=Data[i];

 tmp[i]=El;
 size++;
 delete []Data;
 Data=tmp;

 return 0;
}

template <class T> int USSR_String<T>::Add()
{
 T* tmp;
 int i;

 tmp=new T[size+1];
 for(i=0;i<size;i++)
  tmp[i]=Data[i];

 size++;
 delete []Data;
 Data=tmp;

 return 0;
}
//____________________________USSR_String_End
//____________________________USSR_Array
template <class T> class USSR_Array{
public:
 T* Data;
 int size;

 USSR_Array(){Data=NULL;size=0;};
 T& operator[](int ind){return Data[ind];};
 void create(int siz){Data=new T[siz];size=siz;};
 void destroy(){if(Data!=NULL)delete []Data; Data=NULL;size=0;};
};

//____________________________USSR_Array_End
//____________________________USSR_CSDinamic_Array
template <class T> class USSR_CSDinamic_Array{
private:

public:
 USSR_String<USSR_Array<T> > Data;
 int size;
 int size_mem;
 int array_size;

 USSR_CSDinamic_Array(){array_size=64;size=0;size_mem=0;};
 
 //USSR_CSDinamic_Array(int siz){array_size=siz;size=0;size_mem=0;};
 T& operator[](int ind);

 ~USSR_CSDinamic_Array();
 int Add(T El);
 //int setsize(int siz, int flag);
 void clear();
 //int free_anused_memory();

//for use as stack
 int push(T El){return Add(El);};
 T pop();   //memory not free on this operation
};


inline template <class T> T& USSR_CSDinamic_Array<T>::operator[](int ind)
{
 return Data.Data[ind/array_size].Data[ind%array_size];
}

inline template <class T> T USSR_CSDinamic_Array<T>::pop()
{
 size--;
 return Data.Data[(size)/array_size].Data[(size)%array_size];
}

inline template <class T> void USSR_CSDinamic_Array<T>::clear()
{
 size=0;
}

template <class T> USSR_CSDinamic_Array<T>::~USSR_CSDinamic_Array()
{
 int i;
 for(i=0;i<Data.size;i++)
  Data.Data[i].destroy();
}

/*
template <class T> void USSR_CSDinamic_Array<T>::destroy()
{
 int i;
 for(i=0;i<Data.size;i++)
  Data.Data[i].destroy();
} */

inline template <class T> int USSR_CSDinamic_Array<T>::Add(T El)
{
 if(size==size_mem){Data.Add();Data.Data[size/array_size].create(array_size);size_mem+=array_size;}
 Data.Data[(size)/array_size].Data[(size)%array_size]=El;
 size++;
 return 0;
}
//____________________________USSR_CSDinamic_Array_End
//____________________________USSR_BinClass
class USSR_BinClass{
public:
 unsigned int *flag;
 int size;
 int num_of_classes;
 int num_of_int;

 int create(int msiz, int class_siz);
 void set(int ind, int class_ind);
 void destroy(){if(flag!=NULL)delete []flag; flag=NULL;};
 bool test(int ind, int class_ind);
 USSR_BinClass(){flag=NULL;};
 USSR_BinClass(int msiz, int classes_num);
 ~USSR_BinClass(){if(flag!=NULL)delete []flag;flag=NULL;};
};

USSR_BinClass::USSR_BinClass(int msiz, int classes_num)
{
 int i,j;
 i=classes_num/(sizeof(unsigned int)*8);
 if((classes_num%(sizeof(unsigned int)*8))>0)i++;
 flag=new unsigned int[i*msiz];
 size=msiz;
 num_of_classes=classes_num;
 num_of_int=i;
 for(j=0;j<(size*i);j++)
  flag[j]=0;

}

int USSR_BinClass::create(int msiz, int classes_num)
{
 int i,j;
 i=classes_num/(sizeof(unsigned int)*8);
 if((classes_num%(sizeof(unsigned int)*8))>0)i++;
 if(flag!=NULL)delete []flag;
 flag=new unsigned int[i*msiz];
 size=msiz;
 num_of_classes=classes_num;
 num_of_int=i;
 for(j=0;j<(size*i);j++)
  flag[j]=0;

 return 0;
}

inline void USSR_BinClass::set(int ind, int class_ind)
{
 flag[ind*num_of_int+class_ind/(sizeof(unsigned int)*8)]|=1<<(class_ind%(sizeof(unsigned int)*8));
}

inline bool USSR_BinClass::test(int ind, int class_ind)
 {
   if((flag[ind*num_of_int+class_ind/(sizeof(unsigned int)*8)]&(1<<(class_ind%(sizeof(unsigned int)*8))))!=0)return true;
   return false;
 }

//____________________________USSR_BinClass_End
//____________________________USSR_Set
template <class T> class USSR_Set{
public:
 T* Set;
 int size;

 bool in(T El);
 bool cross(USSR_Set<T> S);
 int getin(USSR_Set<T> S);
 int add(T El);
// int remove(T El);
 USSR_Set(){Set=NULL;size=0;};
 ~USSR_Set(){if(Set!=NULL)delete []Set;Set=NULL;size=0;};
 USSR_Set<T>& operator=(const USSR_Set<T>& S);
 USSR_Set(const USSR_Set<T>& S);
};

template <class T> USSR_Set<T>::USSR_Set(const USSR_Set<T>& S)
{
 int i;
 if(Set!=NULL)delete []Set;
 Set=new T[S.size];
 size=S.size;
 for(i=0;i<size;i++)
  Set[i]=S.Set[i];
}

inline template <class T> USSR_Set<T>& USSR_Set<T>::operator=(const USSR_Set<T>& S)
{
 int i;
 if(Set!=NULL)delete []Set;
 Set=new T[S.size];
 size=S.size;
 for(i=0;i<size;i++)
  Set[i]=S.Set[i];
 return *this;
}

inline template <class T> bool USSR_Set<T>::cross(USSR_Set<T> S)
{
 int i;
 for(i=0;i<size;i++)
  if(S.in(Set[i]))return true;
 return false;
}

inline template <class T> bool USSR_Set<T>::in(T El)
{
 int i;
 for(i=0;i<size;i++)
  if(Set[i]==El)return true;
 return false;
}

template <class T> int USSR_Set<T>::add(T El)
{
 int i;
 T* tmp;
 if(in(El))return 0;
 tmp=new T[size+1];
 for(i=0;i<size;i++)
  tmp[i]=Set[i];
 tmp[i]=El;
 size++;
 delete []Set;
 Set=tmp;
 return 0;
}

template <class T> int USSR_Set<T>::getin(USSR_Set<T> S)
{
 int i,k=0;
 T* tmp;
 int *flag;

 flag=new int[S.size];

 for(i=0;i<size;i++)
  if(S.in(Set[i])){k++;flag[i]=1;}
  else {flag[i]=0;}

 tmp=new T[size+S.size-k];
 for(i=0;i<size;i++)
  tmp[i]=Set[i];
 size=size+S.size-k;
 for(k=0;i<size;k++)
  if(flag[k]==0){tmp[i]=S.Set[k];i++;}

 delete []Set;
 delete []flag;
 Set=tmp;
 return 0;
}
//____________________________USSR_Set_End
//____________________________USSR_BinClassNumerator
class USSR_BinClassNumerator{
public:
 int *number;
 int *class_ground;
 int size;
 int num_of_classes;
 int num_of_grounds;

 int create(int msiz, int class_num, int ground_num);
 int create(int class_num);
 int create(int msiz, int ground_num);
 int set(int ind, int class_ind, int num);
 void destroy(){if(number!=NULL)delete []number; number=NULL; if(class_ground!=NULL)delete []class_ground; class_ground=NULL;};
 int get(int ind, int class_ind);
 USSR_BinClassNumerator(){number=NULL;class_ground=NULL;};
 USSR_BinClassNumerator(int msiz, int class_num, int ground_num){create(msiz,class_num,ground_num);};
 ~USSR_BinClassNumerator(){destroy();};
};

inline int USSR_BinClassNumerator::set(int ind, int class_ind, int num)
{
 if(number==NULL || class_ground==NULL) return -1;
 number[ind*num_of_grounds+class_ground[class_ind]]=num;
 return 0;
}

inline int USSR_BinClassNumerator::get(int ind, int class_ind)
{
 if(number==NULL || class_ground==NULL) return -1;
 return number[ind*num_of_grounds+class_ground[class_ind]];
}

inline int USSR_BinClassNumerator::create(int msiz, int class_num, int ground_num)
{
 int j;

 if(number!=NULL){delete []number;number=NULL;}
 if(class_ground!=NULL){delete []class_ground;class_ground=NULL;}
 number=new int[ground_num*msiz];
 class_ground=new int[class_num];
 size=msiz;
 num_of_classes=class_num;
 num_of_grounds=ground_num;
 for(j=0;j<(size*ground_num);j++)
  number[j]=-1;
 for(j=0;j<class_num;j++)
  class_ground[j]=0;

 return 0;
}

inline int USSR_BinClassNumerator::create(int class_num)
{
 int j;
 if(class_ground!=NULL){delete []class_ground;class_ground=NULL;}
 class_ground=new int[class_num];
 num_of_classes=class_num;
 for(j=0;j<class_num;j++)
  class_ground[j]=0;

 return 0;
}

inline int USSR_BinClassNumerator::create(int msiz, int ground_num)
{
 int j;

 if(number!=NULL){delete []number;number=NULL;}
 number=new int[ground_num*msiz];
 size=msiz;
 num_of_grounds=ground_num;
 for(j=0;j<(size*ground_num);j++)
  number[j]=-1;

 return 0;
}

//____________________________USSR_BinClassNumerator

/*
//____________________________USSR_
template <class T> class USSR_{
public:
 T* Data;
 int size;

 USSR_(){Data=NULL;size=0;};

};

//____________________________USSR__End
*/

#endif
