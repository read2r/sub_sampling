#include <stdio.h>
#include <string.h>
#include "stb_image.h"
#include "stb_image_write.h"

typedef struct _Image {
    char* path;
    int width;
    int height;
    int channels;
    char *ext;
    unsigned char* data;
} Image;

Image* Image_create(const char *path, int width, int height, int channels) {
    Image* image = (Image*)malloc(sizeof(Image));
    image->path = (char*)malloc(sizeof(char) * strlen(path) + 1);
    strcpy(image->path, path);
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->ext = strchr(image->path, '.');
    image->data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channels);
    return image;
}

Image* Image_load(const char *path) {
    Image* image = (Image*)malloc(sizeof(Image));
    image->path = (char*)malloc(sizeof(char) * strlen(path) + 1);
    strcpy(image->path, path);
    image->ext = strchr(image->path, '.');
    image->data = stbi_load(path, &image->width, &image->height, &image->channels, 0);
    return image;
}

void Image_write(Image *image) {
    const char *ext = strchr(image->path, '.');
    int extlen = strlen(ext);

    const char *path = image->path;
    int width = image->width;
    int height = image->height;
    int channels = image->channels;
    unsigned char *data = image->data;

    if(strncmp(ext, ".jpg", extlen) == 0 || strncmp(ext, ".jpeg", extlen) == 0) {
        stbi_write_jpg(path, width, height, channels, data, 100);
    } else if(strncmp(ext, ".png", extlen) == 0) {
        stbi_write_png(path, width, height, channels, data, width * channels);
    }
}

void Image_show(Image *image) {
    printf("%-12s : %s\n", "Image Path", image->path);
    printf("%-12s : %3d\n", "Width", image->width); 
    printf("%-12s : %3d\n", "Height", image->height); 
    printf("%-12s : %3d\n", "Channel", image->channels); 
}

char* makeNewImagePath(const char *imagePath, const char *suffix) {
    int imagePathLen = strlen(imagePath);
    int suffixLen = strlen(suffix);
    int newPathLen = imagePathLen + suffixLen + 1;

    char *newPath = (char*)malloc(sizeof(char) * newPathLen);
    memset(newPath, 0, sizeof(char) * newPathLen);

    const char *ext = strchr(imagePath, '.');

    int k = 0;
    for(int i = 0; i < imagePathLen; i++) {
        for(int j = 0; j < suffixLen && imagePath + i == ext; j++) {
            newPath[k++] = suffix[j];
        }
        newPath[k++] = imagePath[i];
    }

    return newPath;
}

Image* Image_minSampling(Image *image, int filter) {
    int stride = filter;
    char *spath= makeNewImagePath(image->path, "_min");
    int swidth = (image->width-filter) / stride+1;
    int sheight = (image->height-filter) / stride+1;
    int schannels = image->channels;
    Image *imageSampling = Image_create(spath, swidth, sheight, schannels);
    int n = 0;
    for(int i = 0; i < image->height; i+=stride) {
        for(int j = 0; j < image->width; j+=stride) {
            for(int k = 0; k < 3; k++) {
                int min = 255;
                for(int p = 0; p < filter; p++) {
                    for(int q = 0; q < filter; q++) {
                        int idx = (i + p) * image->width * image->channels + (j + q) * image->channels + k;
                        if(min > image->data[idx]) {
                            min = image->data[idx];
                        }
                    }
                }
                imageSampling->data[n++] = min;
            }
        }
    }
    free(spath);
    return imageSampling;
}

Image* Image_maxSampling(Image *image, int filter) {
    int stride = filter;
    char *spath= makeNewImagePath(image->path, "_max");
    int swidth = (image->width-filter) / stride+1;
    int sheight = (image->height-filter) / stride+1;
    int schannels = image->channels;
    Image *imageSampling = Image_create(spath, swidth, sheight, schannels);
    int n = 0;
    for(int i = 0; i < image->height; i+=stride) {
        for(int j = 0; j < image->width; j+=stride) {
            for(int k = 0; k < 3; k++) {
                int max = 0;
                for(int p = 0; p < filter; p++) {
                    for(int q = 0; q < filter; q++) {
                        int idx = (i + p) * image->width * image->channels + (j + q) * image->channels + k;
                        if(max < image->data[idx]) {
                            max = image->data[idx];
                        }
                    }
                }
                imageSampling->data[n++] = max;
            }
        }
    }
    free(spath);
    return imageSampling;
}

Image* Image_avgSampling(Image *image, int filter) {
    int stride = filter;
    char *spath= makeNewImagePath(image->path, "_avg");
    int swidth = (image->width-filter) / stride+1;
    int sheight = (image->height-filter) / stride+1;
    int schannels = image->channels;
    Image *imageSampling = Image_create(spath, swidth, sheight, schannels);
    int n = 0;
    for(int i = 0; i < image->height; i+=stride) {
        for(int j = 0; j < image->width; j+=stride) {
            for(int k = 0; k < 3; k++) {
                int avg = 0;
                for(int p = 0; p < filter; p++) {
                    for(int q = 0; q < filter; q++) {
                        int idx = (i + p) * image->width * image->channels + (j + q) * image->channels + k;
                        avg += image->data[idx];
                    }
                }
                avg /= (filter * filter);
                imageSampling->data[n++] = avg;
            }
        }
    }
    free(spath);
    return imageSampling;
}

void Image_free(Image *image) {
    free(image->path);
    free(image->data);
    free(image);
}


int main(int argc, char **argv) {
    if(!(argc == 2 || argc == 3)) {
        printf("Usage : <PROGRAM> <IMAGE_PATH>\n");
        return -1;
    }

    const char *imagePath = argv[1];
    Image *rimage = Image_load(imagePath);
    if(rimage->data) {
        Image_show(rimage);
        printf("----------------------------------------\n");
    } else {
        printf("Failed to load image\n");
        return -1;
    }
    
    int filter = 2;
    Image* imageMinSampling = Image_minSampling(rimage, filter);
    Image* imageMaxSampling = Image_maxSampling(rimage, filter);
    Image* imageAvgSampling = Image_avgSampling(rimage, filter);

    Image_write(imageMinSampling);
    Image_write(imageMaxSampling);
    Image_write(imageAvgSampling);

    Image_show(imageMinSampling);
    Image_show(imageMaxSampling);
    Image_show(imageAvgSampling);

    Image_free(imageMinSampling);
    Image_free(imageMaxSampling);
    Image_free(imageAvgSampling);

    return 0;
}
