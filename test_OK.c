#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


unsigned char * MapForward (unsigned char *pMapData, unsigned short   MapDataLength,
unsigned char *pMapPoints, unsigned short   MapPointsLength )
{
    unsigned short DataIndex;
    unsigned short MapPointIndex;

    for(DataIndex = 0; DataIndex < MapDataLength - MapPointsLength + 1; DataIndex++)
    {
        for(MapPointIndex = 0; MapPointIndex < MapPointsLength; MapPointIndex++)
        {
            if( pMapData[DataIndex + MapPointIndex] != pMapPoints[MapPointIndex])
            {
                goto PICK_NEXT_FMAPDATA;
            }
        }
        return(& pMapData[DataIndex]);
    PICK_NEXT_FMAPDATA:;
   }
    return(NULL);
}

void Resetbufer(unsigned char *buf,int size)
{
int i;
for(i=0;i<size;i++)
{
buf[i] = '0';
}
}


int main()
{
const unsigned char OKToken[]={"OK"};

unsigned char buf[6000];
int buf_SIZE=sizeof(buf);


    //usleep(5000000);  /* sleep for 100 milliSeconds */  
    Resetbufer(buf,sizeof(buf));

	strcpy(buf,"");
	
    if(MapForward(buf,buf_SIZE,(unsigned char*)OKToken,2) == NULL)
	{
		printf("Not Found ! %s", buf);
	}
	else{
		printf("OK Found ! %s", buf);
	}



}
