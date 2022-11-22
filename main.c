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
void save_image(Image* image, const char* path){
    FILE* file = fopen(path, "wb");
    
    fwrite(&image->header, sizeof(image->header), 1, file);
    fwrite(image->pixels, sizeof(Pixel) * image->canvas.width * image->canvas.height, 1, file);
      
    fclose(file);
}

void write_text(const int posX, const int posY, Image* font_img, Image* to_img){
    for(int i = 0 ; i < font_img->canvas.width; i++){
        for(int j = 0 ; j < font_img->canvas.height; j ++){
            //printf("Line: %d, row %d", j*(to_img->canvas.width), i);
            Pixel pixels = font_img->pixels[j*font_img->canvas.width + i];
            if(pixels.blue == 0 && pixels.green == 0 && pixels.red == 0){
                continue;
            }
            to_img->pixels[((j+posY) * to_img->canvas.width) + (i+posX)] = pixels;
        }
    }
}
void write(char* text, bool bottom, int pos, Font* font, Image* to){
    int width = 0;
    const int font_spacing = 25;
    const int vertical_spacing = 20;
    printf("l%d \n", strlen(text));
    int movement = (to->canvas.width - (strlen(text) * font_spacing)) / 2;
    for(int i = 0 ; i < strlen(text); i ++){
        char c = text[i];
        if(c == ' '){
            width += 15;
            continue;
        }
        Image* image = &font->image[c-65];
        printf("Writing %c at pos: %d, %d\n", c, width+10, pos*(image -> canvas.height + vertical_spacing));
        printf("Image: %d", image);
        if(bottom){
            write_text(width + 10 + movement, (to -> canvas.height - vertical_spacing - 25) - (pos*image->canvas.height), image, to);
        }else{
            write_text(width + 10 + movement, pos*(image -> canvas.height + vertical_spacing) + 10, image, to);
        }
        width += (image -> canvas.width);
    }
}
// Free the memory of the given image.
void image_free(Image* image) {
    free(image->pixels);
}

bool load_fonts(Font* font, char* path){
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
void free_fonts(Font* font){
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
    Image image = {};
    image_load(&image, "img1.tga");
    //write_text(0, 0, &font->image[0], &image);
    write("AHOJ", false, 0, font, &image);
    write("JAK JE", false, 1, font, &image);

    write("TEST", true, 0, font, &image);
    write("AAATESTAAA", true, 1, font, &image);
    save_image(&image, "out.tga");
    free_fonts(font);
    
    return 0;
}