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

Image* Image_minSampling(Image *image, int downScale) {
    char *spath= makeNewImagePath(image->path, "_min");
    int swidth = image->width / downScale;
    int sheight = image->height / downScale;
    int schannels = image->channels;
    Image *imageMinSampling = Image_create(spath, swidth, sheight, schannels);
    for(int i = 0; i < sheight; i++) {
        for(int j = 0; j < swidth; j++) {
            for(int k = 0; k < 3; k++) {
                unsigned char min = 255;
                for(int p = 0; p < downScale; p++) {
                    for(int q = 0; q < downScale; q++) {
                        unsigned char temp = image->data[(downScale * i + p) * image->width * image->channels + (downScale * j + q) * image->channels + k];
                        if(min > temp) {
                            min = temp;
                        }
                    }
                }
                imageMinSampling->data[i * swidth * schannels + j * schannels + k] = min;
            }
        }
    }
    free(spath);
    return imageMinSampling;
}

Image* Image_maxSampling(Image *image, int downScale) {
    char *spath= makeNewImagePath(image->path, "_max");
    int swidth = image->width / downScale;
    int sheight = image->height / downScale;
    int schannels = image->channels;
    Image *imageMaxSampling = Image_create(spath, swidth, sheight, schannels);
    for(int i = 0; i < sheight; i++) {
        for(int j = 0; j < swidth; j++) {
            for(int k = 0; k < 3; k++) {
                unsigned char max = 0;
                for(int p = 0; p < downScale; p++) {
                    for(int q = 0; q < downScale; q++) {
                        unsigned char temp = image->data[(downScale * i + p) * image->width * image->channels + (downScale * j + q) * image->channels + k];
                        if(max < temp) {
                            max = temp;
                        }
                    }
                }
                imageMaxSampling->data[i * swidth * schannels + j * schannels + k] = max;
            }
        }
    }
    free(spath);
    return imageMaxSampling;
}

Image* Image_avgSampling(Image *image, int downScale) {
    char *spath= makeNewImagePath(image->path, "_avg");
    int swidth = image->width / downScale;
    int sheight = image->height / downScale;
    int schannels = image->channels;
    Image *imageAvgSampling = Image_create(spath, swidth, sheight, schannels);
    for(int i = 0; i < sheight; i++) {
        for(int j = 0; j < swidth; j++) {
            for(int k = 0; k < 3; k++) {
                int avg = 0;
                for(int p = 0; p < downScale; p++) {
                    for(int q = 0; q < downScale; q++) {
                        unsigned char temp = image->data[(downScale * i + p) * image->width * image->channels + (downScale * j + q) * image->channels + k];
                        avg += temp;
                    }
                }
                avg /= (downScale * downScale);
                imageAvgSampling->data[i * swidth * schannels + j * schannels + k] = avg;
            }
        }
    }
    free(spath);
    return imageAvgSampling;
}

void Image_free(Image *image) {
    free(image->path);
    free(image->data);
    free(image);
}


int main(int argc, char **argv) {
    if(!(argc == 2 || argc == 3)) {
        printf("Usage1 : <PROGRAM> <IMAGE_PATH>\n");
        printf("Usage2 : <PROGRAM> <IMAGE_PATH> <DOWN_SCALE>\n");
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
    
    int downScale = 2;
    if(argc == 3) {
        downScale = atoi(argv[2]);
    }

    Image* imageMinSampling = Image_minSampling(rimage, downScale);
    Image* imageMaxSampling = Image_maxSampling(rimage, downScale);
    Image* imageAvgSampling = Image_avgSampling(rimage, downScale);

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
