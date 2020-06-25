#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "iosfwd"
#include <iostream>
#include <iostream>
//#include "SimpleGPIO_SPI.h"
//#include "SPI_SS_Def.H"
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include <spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
using std::cout;
using std::endl;
bool SectorNoHavingNData[128];
bool SectorNoHavingData[128];

#define CLK_PER_SEC_ACTUAL (99000)

void msleep(int ms)
{
	struct timespec ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}
bool SearchForData(  uint8_t *buff,long size)
{
	for (long i=0;i<size;i++)
	{
		if(buff[i]<0xff)
			return true;
	}
	return false;
}

#define FLASH_CERS 0xC7
#define FLASH_SE   0xD8
//definitions for AT25512 device
#define WRITE_CYCLE_TIME (5000)                     //AT25512 write cycle time in us
#define WRSR (0x01)                              //AT25512 write status register
#define WRITE (0x02)                           //AT25512 write data to memory array
#define READ (0x03)                              //AT25512 read data from memory array
#define WRDI (0x04)                              //AT25512 reset write enable latch
#define RDSR (0x05)                              //AT25512 read status register
#define WREN (0x06)                              //AT25512 set write enable latch
static void pabort(const char *s)
{
   perror(s);
   abort();
}
typedef unsigned char tByte;
tByte FlashReadCmd[4];
tByte NVM16ReadCmd[3];
//tByte TempTxData128[128];
tByte Rxd256[256];
tByte Rxd[2048];
//changed to AT25512 write cycle time
unsigned int Spi_address=0;
tByte TempTxData128[128];
tByte TempTxNvmData128[128];
tByte Write16Cmd[3];
tByte TempByteBuffer[256];
char TemCharBuffer[256];

////////////////////////
using namespace std;

typedef unsigned int tShort;
typedef unsigned char tByte;
enum MemoryType{faulty_target,nvm_target,flash_target};
enum Manufacturer{unknown_target,macron_target,sst_target};
#define NO_OF_FPAGES (512*1024)
#define NO_OF_NPAGES (0x20000)
class MemoryPage
{
   public:

      bool DataPresent;   // data is there or no
  char PageData[256];  // Data details.
     int PageNumber;   // Which PAge number
     // tByte MemoryAddressing;
//int pagesize;
};
/////////

int aclem,HowManyFpages;
int SizeofMemory[2];
#define FLASH_MEMORY 0

MemoryPage FMemorypages[NO_OF_FPAGES];
#define FPAGE_SIZE 256
////tByte Rxd256[256];
class MemoryInf
{
   public:

     //

   unsigned int PageSize;
   unsigned long NoOfPages;
   unsigned long sectorsize;
   MemoryType mstatus;
   Manufacturer mf;


     // tByte MemoryAddressing;
};
MemoryInf TargetMemory;
unsigned int bytearray[16];
void InitPages(int m){
for (int i=0;i<m;i++)
	{
		FMemorypages[i].DataPresent=false;
		for(int j=0;j<256;j++)
		FMemorypages[i].PageData[j]=0xff;

	}}
string strg;
string strgff="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
tByte  getNVM_CRC(tByte *CKBlock, uint32_t  Length, tByte Seed)
{
	tByte val,y,crc, bbit1;
	uint32_t  k;
	 crc = Seed;
	 for(k = 0; k < Length; k++)
	 {
		val = CKBlock[k];
	    crc = crc ^ ((val << 4));
		for(y = 0; y < 8; y++)
		{
			bbit1 = crc & 0x80;
			if(bbit1)
			{
				crc = crc ^ 74;
				bbit1 = 1;
			}
			else
				bbit1 = 0;
			crc = ((crc << 1) + bbit1);
		}
	    crc = crc ^ 82;
		crc = crc ^ (val >> 4);
	 }
	 return(crc);
}
tByte  PrintFPageHavingCRC(unsigned int x)///int size)
{
tByte yed=80;
	HowManyFpages=0;
	tByte temp256[256];
	memset(temp256,0xff,256);
	printf("\nNoOFPages=%d",x);
	for (int i=0;i<x;i++)
	{


		if(FMemorypages[i].DataPresent==true)
			{
			cout<<"\nData Present @ PageNo.=,0x";
			cout<<hex << i<<endl;
			HowManyFpages++;
			////usleep(1000000);
			yed=getNVM_CRC((tByte *)FMemorypages[i].PageData,256, yed);
			}
		else
			yed=getNVM_CRC(temp256,256, yed);
	}
	printf("\nTotal Fpages=%d\n",HowManyFpages);
	printf("\nTotal CRC=%d\n",yed);return yed;

}
   int charToByteArray(long int x,char *hexstring){
         int i;
int blen=0;
int offset=0;
int pn=0;
         int str_len = strlen(hexstring);

     	memset(bytearray, 0xFF, sizeof(bytearray));
     	blen=((str_len / 2));

         for (i = 0; i < ((str_len / 2)); i++) {
             sscanf(hexstring + 2*i, "%02x", &bytearray[i]);
         }
////printf("\nconverted bytes=");
offset=x%256;
pn=x/256;
////printf("\npn=%d, offset=%d , len=%d",pn,offset,blen);
int y=0;
        for(i=0;i<blen;i++)
        {

       	 y=offset+i;
       	 if(y>255)
       	 {
       		 FMemorypages[pn+1].PageData[0] =bytearray[i];
       		 printf("\nExceeding the array\n");
       		// sleep(3);

       	 }
       	 else
       		 FMemorypages[pn].PageData[offset+i] =bytearray[i];
       ///	BufferFpages[400][256];
       	//// cout<<hex<<bytearray[i]<<endl;
        }


         return 1;
     }
string getFirstWord(string text)
     {
            bool firstWordFound = false;
            string firstWord = "";

            for(int i = 0; i <= text.length() && firstWordFound == false; i++)
            {
                  if (text[i] = ' ')
                  {
                              firstWordFound = true;

                              for (int j = 0; j <= i; j++)
                              {
                                  firstWord += text[j];
                              }
                  }
            }

            return firstWord;
     }
tByte Crc_Buffer[2];
void InitFBufferData()
{
	uint16_t p=0;
	int *bytearray=new int[16];
		string word;
		 char *temp1;
		char *e;
		int k=0;

		string fw="";
		int crc_count=0;
		long int Spi_adresses=0;
		long int temp_adress=0;
		struct timeval t1, t2;
		    double elapsedTime;
	string strg2=";CRC=";

	std::cout <<"Read Clem line by line\n";

 ifstream infile;
infile.open ("/root/Melc/0010603.j46");
aclem=0;
	 HowManyFpages=0;
//////////////////////////////
	// printf("\nTargetMemory.NoOfPages=%l",TargetMemory.NoOfPages);
	//sleep(3);
	    		 SizeofMemory[FLASH_MEMORY]=TargetMemory.NoOfPages*256;//1024*1024;
	 SizeofMemory[FLASH_MEMORY]=TargetMemory.NoOfPages*256;//1024*1024;
	    		 InitPages(2048);///4096);
	    	     while(aclem<1) // To get you all the lines.
	    	    		     		      {
	    	    		     		          getline(infile,strg); // Saves the line in STRING.
	    	    		     		       //   if (STRING != previousLine)
	    	    		     		          {
	    	    		     		             // previousLine=strg;
	    	    		     		              // Prints our STRING.
	    	    		     		              ///////////////////

	    	    		     		              if(strg[0]!=';')// && strg[1])//20084BA1//
	    	    		     		              {
	    	    		     		            	  if(getFirstWord(strg).length()<9)
	    	    		     		            	  {
	    	    		     		            		  if(strg[0]=='2')
	    	    		     		            		  {
	    	    		     		            			 if(strstr(strg.c_str(),strgff.c_str()))
	;
	    	    		     		            			 else
	    	    		     		            				{
	    	    		     		            				 printf("\nLine=");
	    	    		     		            				cout<<strg<<endl;
	    	    		     		            				 	 	 //fw=getFirstWord(strg);
	    	    		     		            				 stringstream stream(strg);
	    	    		     		            				 k=0;
	    	    		     		            			    while( getline(stream, word, ' ') )
	    	    		     		            			    {
	    	    		     		            			    	if(k==0)
	    	    		     		            			    	{
	    	    		     		            			    		 word=word.erase(0,1);
	    	    		     		            			    		      printf("\nLine[0]=");
	    	    		     		            			    		    	    		     		             cout<<word<<endl;
	    	    		     		            			    		 Spi_adresses = strtol (word.c_str(), &e, 16);

	    	    		     		            			    		  	printf("\nSpi Adres=%ld",Spi_adresses);
	    	    		     		            	       				printf("\nPageNo=%ld\n",(Spi_adresses>>8));
	    	    		     		            	       				//if(0x87C==(Spi_adresses>>8))
	    	    		     		            	       				//	sleep(3);
	    	    		     		            	       			//FMemorypages[i].DataPresent==true
	    	    		     		            			              				FMemorypages[Spi_adresses/256].DataPresent=true;//	FMemoryPages.DataPresent=true;

	    	    		     		            			    	}
	    	    		     		            			    	if(k==1)
	    	    		     		            			    	{
	    	    		     		            			    	//	usleep(500000);
	    	    		     		            			    		//temp1=word;
	    	    		     		            			    		temp1 = new char[word.length() +1];
	    	    		     		            			    		strcpy(temp1, word.c_str());

	    	    		     		            			    		//eventually, remember to delete cstr

	    	    		     		            			    		charToByteArray(Spi_adresses,temp1);


	    	    		     		            			    		//delete[] cstr;

	    	    		     		            			    	}
	    	    		     		            			    	k++;
	    	    		     		            			    	//cout << word << "\n";
	    	    		     		            			    }





	    	    		     		            				/// cout <<hex<<strtoul(getFirstWord(strg).substr(0, 2).c_str(), 0, 16)<<endl;

	    	    		     		            				}

	    	    		     		            			  //
	    	    		     		            			  /////////////////////////////////////
	    	    		     		            		  }
	    	    		     		            	  }
	    	    		     		              }
	    	    		     		              if(strstr(strg.c_str(),strg2.c_str()))//comparing two string if they contains
	    	    		     		              {

	    	    		     		                // crc_count++;
	    	    		     		               //  if( crc_count>1)
	    	    		     		                	 {
	    	    		     		                		 cout << " Merge Clem Flash-Data Reading is completed\n";
	    	    		     		                 break;
	    	    		     		                	 }
	    	    		     		              }

	    	    		     		              ////////////////////
	    	    		     		          }

	    	    		     		      }
	    	    		     		      infile.close();
	//sleep(3);
	    	    		     		     Crc_Buffer[0]=PrintFPageHavingCRC(2048);///4096);//  PrintFPagesHavingData(4096);
	//sleep(3);
	    	    		     		   // cout<<"Pls Enter Buffer Fpage No."<<endl;
	    	    		     		    	    		 //   	cin >> p;
std::ofstream ofs;
ofs.open("/root/BBB_SPI_Flash_EEPROM_Porgrammer/CRcFlash/crcflash", std::ofstream::out | std::ofstream::trunc);
ofs.close();
ofstream myfile;
	    	    		     		    	    	////ReadInitFBuffer( p);
myfile.open ("/root/BBB_SPI_Flash_EEPROM_Porgrammer/CRcFlash/crcflash");
myfile<<(int)Crc_Buffer[0];
////myfile.close("/root/CLem/CRcFlash/crcflash");
myfile.close();
/*
stringstream ss;
ss << Crc_Buffer[0];
string str = ss.str();
  ofstream OFileObject;  // Create Object of Ofstream
    OFileObject.open ("/root/CLem/CRcFlash/crcflash"); // Opening a File or creating if not present
    OFileObject <<str.c_str()<<endl;///"I am writing to a file opened from program.\n"; // Writing data to file 
    ////cout<<"Data has been written to file";
    OFileObject.close(); // Closing the file*/
}

void PrintFPagesHavingData(int x)///int size)
{

	HowManyFpages=0;
	printf("\nNoOFPages=%d",x);
	for (int i=0;i<x;i++)
	{
		if(FMemorypages[i].DataPresent==true)
			{
			cout<<"\nData Present @ PageNo.=,0x";
			cout<<hex << i<<endl;
			HowManyFpages++;
			////usleep(1000000);
			}
	}
	printf("\nTotal Fpages=%d\n",HowManyFpages);


}
// MemoryPage *FMemorypages= new MemoryPage[NO_OF_FPAGES];
  //      MemoryPage *NMemorypages= new MemoryPage[NO_OF_NPAGES];

int main()///(int argc, char *argv[])



{
   int ret = 0;
   int fd;
   memset(TempTxData128,0x96,sizeof(TempTxData128));
//////////////////
InitFBufferData();
///////////////////////////////
return 0;
}
