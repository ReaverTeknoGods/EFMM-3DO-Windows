#ifndef HACK_METHODS_HEAD
#define HACK_METHODS_HEAD

#define HACK_MAINT_BIG 0
#define HACK_MAINT_MULTY 1

#define ARCH_BIG_HEAD 0
#define ARCH_BIG_ELEMENT 1

#include "USSR_std.h"


class archdata{
public:
 int off;
 int size;
 int type;
 int off_head;
 int head_size;
 AnsiString name, tolog;
 USSR_CSDinamic_Array<archdata> *arch;
 archdata(){arch=NULL;};
 ~archdata(){if(arch!=NULL)delete arch;};
};

typedef struct{
int flags;
int maintype;
int extructble;
AnsiString pfname;
USSR_CSDinamic_Array<archdata> arch;
}hackstr;

extern hackstr hack_pak_explore;

void hack_reset();
int hack_test_file(AnsiString ftotest);
void hack_extract(AnsiString fname, hackstr *hack_pak);


#endif
