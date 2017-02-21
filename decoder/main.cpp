#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

class Image
{
	public:
	uint32_t width;
	uint32_t height;
	
	uint32_t precision; //бит на канал  (8)
	uint32_t chanals; //количество каналов (3)
	
	char *data;
};

void readJPEG(const char *filename)
{
	Image img;
	
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
				uint16_t blockSize = (buffer[curPosition] << 8) | buffer[curPosition+1]; //записать 2 байта в целочисленную переменную
				printf("comments block size: %d\n", blockSize);
				curPosition += blockSize; //игнорировать блок еомментариев
				break;
			}
			case 0xD9: //индикатор конца изображения
			{
				printf("unexpected end of image\n");
				//flagSuccess = true;
				curPosition = fileSize;
				break;
			}
			case 0xDD: //??? рестарт? что это значит?
			{
				curPosition+=5; 
				break; //пропуск
			}
			
			case 0xDB: //DQT — таблица квантования
			{
				curPosition++;
				uint16_t blockSize = (buffer[curPosition] << 8) | buffer[curPosition+1]; //записать 2 байта в целочисленную переменную
				printf("DQT block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;
			}
			case 0xC0: //Baseline DCT
			{
				curPosition++;
				uint16_t blockSize = (buffer[curPosition] << 8) | buffer[curPosition+1]; //записать 2 байта в целочисленную переменную
				printf("Baseline DCT block size: %d\n", blockSize);		
				
				img.precision = buffer[curPosition+2]; //бит на канал
				img.height = (buffer[curPosition+3] << 8) | buffer[curPosition+4];
				img.width = (buffer[curPosition+5] << 8) | buffer[curPosition+6];
				img.chanals = buffer[curPosition+7];
				
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;			
			}
			case 0xC4: //DHT (таблица Хаффмана)
			{
				curPosition++;
				uint16_t blockSize = (buffer[curPosition] << 8) | buffer[curPosition+1]; //записать 2 байта в целочисленную переменную
				printf("DHT block size: %d\n", blockSize);		
				curPosition += blockSize; //игнорировать блок еомментариев		
				break;			
			}
			case 0xDA: //SOS (Start of Scan)
			{
				curPosition++;
				printf("Data block start\n");
				while(curPosition<fileSize && buffer[curPosition] != 0xD9)
				{
					curPosition++;
				}
				
				if(buffer[curPosition] == 0xD9)
				{
					flagSuccess = true;
					curPosition = fileSize;
				}
				else
				{
					printf("unexpected end of image\n");
					curPosition = fileSize;					
				}
					
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
		printf("WxH: %dx%d, precision: %d, chanals: %d\n", img.width, img.height, img.precision, img.chanals);
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
