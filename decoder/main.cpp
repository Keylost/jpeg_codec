#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 

void readJPEG(const char *filename)
{
	FILE *jpegFile = NULL;
	unsigned char *buffer;
	uint32_t fileSize;
	unsigned char format[2];
	bool flagSuccess = false;
	
	jpegFile = fopen(filename, "rb"); //открыть файл
	
	if(!jpegFile) //проверить, что файл открыт успешно
	{
		printf("[E]: Can't open file: %s\n", filename);
		return;
	}
	
	fread(format, 2, 1, jpegFile); //проверить, что сие изображение есть jpeg
	if(format[0] != 0xFF || format[1] != 0xD8)
	{
		printf("[E]: Unknown file format: %s\n", filename);
		return;		
	}
	
	fseek(jpegFile, 0, SEEK_END); //перейти в конец файла
	fileSize = ftell(jpegFile); //текущее смещение внутри файла
	rewind(jpegFile); //в начало файла
	
	buffer = (unsigned char *)malloc((fileSize+1)*sizeof(unsigned char)); //выделить память
	fread(buffer, fileSize, 1, jpegFile); //считать файл
	
	fclose(jpegFile); //закрыть файл
	
	/////////JPEG PARSING BLOCK/////////////////////
	
	uint32_t curPosition = 2; //первые два байта уже проверены
	//uint32_t endPosition = fileSize-2; //последняя п
	
	while(curPosition<fileSize)
	{
		if(buffer[curPosition] != 0xFF)
		{
			printf("[E]: JPEG parsing error\n");
			return;
		}
		curPosition++;
		
		switch(buffer[curPosition])
		{
			case 0xD8: //индикатор начала JPEG изображения (уже должен был быть считан)
			{
				printf("[E]: JPEG parsing error\n");
				return;
			}
			case 0xFE: //комментарий
			{
				curPosition++;
				uint32_t blockSize = *((uint16_t *)(&buffer[curPosition]));
				printf("comments block size: %d\n", blockSize);
				curPosition += blockSize; //игнорировать блок еомментариев
				break;
			}
			case 0xD9: //индикатор конца изображения
			{				
				flagSuccess = true;
				curPosition = fileSize;
				break;
			}
			
			case 0xDB: //DQT — таблица квантования
			{
				curPosition++;
				uint32_t blockSize = *((uint16_t *)(&buffer[curPosition]));
				printf("DQT block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;
			}
			case 0xC0: //Baseline DCT
			{
				curPosition++;
				uint32_t blockSize = *((uint16_t *)(&buffer[curPosition]));
				printf("Baseline DCT block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;			
			}
			case 0xC4: //DHT (таблица Хаффмана)
			{
				curPosition++;
				uint32_t blockSize = *((uint16_t *)(&buffer[curPosition]));
				printf("DHT block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;			
			}
			case 0xDA: //SOS (Start of Scan)
			{
				curPosition++;
				uint32_t blockSize = *((uint16_t *)(&buffer[curPosition]));
				printf("Data block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;			
			}
			case 0xE0:
			{
				printf("The JPEG File Interchange Format (JFIF) detected. Interrupted. \n");
				return;
			}
			
									
			default:
			{
				printf("[E]: Unknown block indicator %02x \n", buffer[curPosition]);
				//return;
				//break;
			}
		}
		
	}
	
	if(flagSuccess)
	{
		printf("[I]: JPEG parsing finished successfully\n");
	}
	else
	{
		printf("[E]: bad eng of JPEG file\n");
	}
	
	return;
}

int main(int argc, char ** argv)
{
	readJPEG(argv[1]);
	
	return 0;
}
