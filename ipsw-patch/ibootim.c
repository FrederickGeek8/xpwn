#include <stdlib.h>
#include <string.h>
#include "xpwn_common.h"
#include "abstractfile.h"
#include <xpwn/libxpwn.h>
#include <xpwn/ibootim.h>
#include <xpwn/lzss.h>
#include <xpwn/nor_files.h>

void flipIBootIMHeader(IBootIMHeader* header) {
	FLIPENDIANLE(header->unknown);
	FLIPENDIANLE(header->compression_type);
	FLIPENDIANLE(header->format);
	FLIPENDIANLE(header->width);
	FLIPENDIANLE(header->height);
}

size_t readIBootIM(AbstractFile* file, void* data, size_t len) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data); 
	memcpy(data, (void*)((uint8_t*)info->buffer + (uint32_t)info->offset), len);
	info->offset += (size_t)len;
	return len;
}

size_t writeIBootIM(AbstractFile* file, const void* data, size_t len) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data);

	while((info->offset + (size_t)len) > info->length) {
		info->length = info->offset + (size_t)len;
		info->buffer = realloc(info->buffer, info->length);
	}
	
	memcpy((void*)((uint8_t*)info->buffer + (uint32_t)info->offset), data, len);
	info->offset += (size_t)len;
	
	info->dirty = TRUE;
	
	return len;
}

int seekIBootIM(AbstractFile* file, off_t offset) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data);
	info->offset = (size_t)offset;
	return 0;
}

off_t tellIBootIM(AbstractFile* file) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data);
	return (off_t)info->offset;
}

off_t getLengthIBootIM(AbstractFile* file) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data);
	return info->length;
}

void closeIBootIM(AbstractFile* file) {
	InfoIBootIM* info = (InfoIBootIM*) (file->data);
	uint8_t *compressed;
	if(info->dirty) {
		compressed = (uint8_t*)malloc(info->length * 2);
		info->compLength = (uint32_t)(compress_lzss(compressed, info->length * 2, (uint8_t*)info->buffer, info->length) - compressed);
		
		flipIBootIMHeader(&(info->header));
		info->file->seek(info->file, 0);
		info->file->write(info->file, &(info->header), sizeof(info->header));
		
		info->file->seek(info->file, sizeof(info->header));
		info->file->write(info->file, compressed, info->compLength);
		free(compressed);
		
	}
	
	free(info->buffer);
	info->file->close(info->file);
	free(info);
	free(file);
}


AbstractFile* createAbstractFileFromIBootIM(AbstractFile* file) {
	InfoIBootIM* info;
	AbstractFile* toReturn;
	uint8_t *compressed;

	if(!file) {
		return NULL;
	}
	
	info = (InfoIBootIM*) malloc(sizeof(InfoIBootIM));
	info->file = file;
	file->seek(file, 0);
	file->read(file, &(info->header), sizeof(info->header));
	flipIBootIMHeader(&(info->header));
	if(strcmp(info->header.signature, IBOOTIM_SIGNATURE) != 0) {
		free(info);
		XLOG(1, "createAbstractFileFromIBootIM: signature does not match\n");
		return NULL;
	}
	
	info->compLength = file->getLength(file) - sizeof(info->header);
	if(info->header.compression_type != IBOOTIM_LZSS_TYPE) {
		//free(info);
		XLOG(1, "createAbstractFileFromIBootIM: (warning) unsupported compression type: %x\n", info->header.compression_type);
		//return NULL;
	}

	int depth = 0;	
	if(info->header.format == IBOOTIM_ARGB) {
		info->length = 4 * info->header.width * info->header.height;
		depth = 4;
	} else if(info->header.format == IBOOTIM_GREY) {
		info->length = 2 * info->header.width * info->header.height;
		depth = 2;
	} else {
		XLOG(1, "createAbstractFileFromIBootIM: unsupported color type: %x\n", info->header.format);
		free(info);
		return NULL;
	}

	info->buffer = malloc(info->length);
	compressed = (uint8_t*)malloc(info->compLength);
	file->read(file, compressed, info->compLength);

	int length = decompress_lzss((uint8_t*)info->buffer, compressed, info->compLength);
	if(length > info->length) {
		XLOG(1, "createAbstractFileFromIBootIM: decompression error\n");
		free(compressed);
		free(info);
		return NULL;
	} else if(length < info->length) {
		XLOG(1, "createAbstractFileFromIBootIM: (warning) uncompressed data shorter than expected: %d\n", length);
		info->length = length;
	}

	free(compressed);
	
	info->dirty = FALSE;
	
	info->offset = 0;
	toReturn = (AbstractFile*) malloc(sizeof(AbstractFile));
	toReturn->data = info;
	toReturn->read = readIBootIM;
	toReturn->write = writeIBootIM;
	toReturn->seek = seekIBootIM;
	toReturn->tell = tellIBootIM;
	toReturn->getLength = getLengthIBootIM;
	toReturn->close = closeIBootIM;
	toReturn->type = AbstractFileTypeIBootIM;
	
	return toReturn;
}

AbstractFile* duplicateIBootIMFile(AbstractFile* file, AbstractFile* backing) {
	InfoIBootIM* info;
	AbstractFile* toReturn;

	if(!file) {
		return NULL;
	}

	info = (InfoIBootIM*) malloc(sizeof(InfoIBootIM));
	memcpy(info, file->data, sizeof(InfoIBootIM));
	
	info->file = backing;
	info->buffer = malloc(1);
	info->length = 0;
	info->dirty = TRUE;
	info->offset = 0;
	
	toReturn = (AbstractFile*) malloc(sizeof(AbstractFile));
	toReturn->data = info;
	toReturn->read = readIBootIM;
	toReturn->write = writeIBootIM;
	toReturn->seek = seekIBootIM;
	toReturn->tell = tellIBootIM;
	toReturn->getLength = getLengthIBootIM;
	toReturn->close = closeIBootIM;
	toReturn->type = AbstractFileTypeIBootIM;
	
	return toReturn;
}


