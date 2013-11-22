
#include <stdio.h>
#include "test.h"


#define BLAH 0x100, -1, 1, 0, 100
#define BLAHBLAH 0x200, 10, 20, 30, 40

struct testStruct{
	int a,b,c,d,e;
};

//#define ENTRY(a,b,c,d,e){a,b,c,d,e}

void main(){

	struct testStruct s[] = {BLAH, BLAHBLAH};

//		ENTRY(BLAH),
//		ENTRY(BLAHBLAH)
//	};

	printf("\n ENTRY1: %d %d %d %d %d", s[0].a, s[0].b, s[0].c, s[0].d, s[0].e);
	printf("\n ENTRY2: %d %d %d %d %d", s[1].a, s[1].b, s[1].c, s[1].d, s[1].e);


}

