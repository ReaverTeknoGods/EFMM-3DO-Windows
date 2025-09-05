unsigned int read_uinte(unsigned char* point)
{
 return (point[0]<<24)|(point[1]<<16)|(point[2]<<8)|(point[3]);
}
unsigned short read_ushorte(unsigned char* point)
{
 return (point[0]<<8)|(point[1]);
}

int search_chank(unsigned int * buf, int size_in_bytes, unsigned int id_chank)
{
int i=0;
 while(i<(size_in_bytes>>2)){if(buf[i]==id_chank)return (i);i++;}
 return -1;
}

unsigned int uint_to_bigend(unsigned int num)
{
 return (((num)&255)<<24)|(((num>>8)&255)<<16)|(((num>>16)&255)<<8)|(((num>>24)&255));
}

unsigned int ConvertPix_16Direct(unsigned short col)
{
  return ((col&0x1f)<<(3+16))|(((col>>5)&0x1f)<<(3+8))|(((col>>10)&0x1f)<<(3));
}
 