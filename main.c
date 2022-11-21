#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef unsigned char byte;
typedef struct {
    byte id_length;
    byte color_map_type;
    byte image_type;
    byte color_map[5];
    byte x_origin[2];
    byte y_origin[2];
    byte width[2];
    byte height[2];
    byte depth;
    byte descriptor;
} TGAHeader;

typedef struct{
    int width;
    int height;
} Canvas;

typedef struct{
    byte blue;
    byte green;
    byte red;
} Pixel;

typedef struct{
    TGAHeader header;
    Pixel* pixels;
    Canvas canvas;
}Image;

typedef struct{
    Image* image; // list of images indexed from 0 [A-Z] 
} Font;

Canvas getCanvas(TGAHeader *header){
    Canvas canvas = {};
    memcpy(&canvas.width, header->width, 2);   
    memcpy(&canvas.height, header->height, 2);
    return canvas;
}

// Loads an image from the given `path`.
// If loading fails, return false.
bool image_load(Image* image, const char* path) {
    FILE* file = fopen(path, "rb");

    memset(image, 0, sizeof(Image));
    fread(&image->header, sizeof(image->header), 1, file);

    // We only support RGB images.
    assert(image->header.depth == 24);
    assert(image->header.image_type == 2);

    memcpy(&image->canvas.width, image->header.width, sizeof(image->header.width));
    memcpy(&image->canvas.height, image->header.height, sizeof(image->header.height));

    image->pixels = (Pixel*) malloc(sizeof(Pixel) * image->canvas.width * image->canvas.height);
    fread(image->pixels, sizeof(Pixel) * image->canvas.width * image->canvas.height, 1, file);

    fclose(file);

    return true;
}

// Free the memory of the given image.
void image_free(Image* image) {
    free(image->pixels);
}

bool load_fonts(Font * font, char* path){
    int starting_latter = 65; //Latter A 
    // /A.tga
    font -> image = (Image*) malloc(sizeof(Image) * 26);
    int length = strlen(path);
    char* new_path = (char*) calloc(length + 7,1);
    strcat(new_path, path);
    strcat(new_path, "/X.tga");
    printf("%s", new_path);
    for(int i = 0 ; i < 26; i ++){
        new_path[length+1] = (starting_latter + i);//May cause issues ?
        Image image = {};
        bool loaded = image_load(&image, new_path);
        if(loaded){
            font -> image[i] = image;
        }else{
            return false;
        }
    }
    free(new_path);
    new_path = NULL;
    return true;
}
void free_fonts(Font * font){
    for(int i = 0; i < 26; i ++){
        image_free(&font->image[i]);
    }
    free(font -> image);
}

int main(int argc, char **argv){
    char* image_in;
    char* image_out;
    char* font_in = "font";
    if (argc < 4){
        printf("Wrong parameters\n");
        image_in = "img1.tga";
        font_in = "font";
        image_out="meme.tga";
    }else{
        image_in = argv[1];
        image_out = argv[2];
        font_in = argv[3];
    }
    Font* font = (Font*)malloc(sizeof(font));
    if (!load_fonts(font, font_in)){
        printf("Could not load image\n");
        return 1;
    }

    free_fonts(font);
    
    return 0;
}