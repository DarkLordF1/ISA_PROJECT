#include <stdio.h>
#include <stdlib.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>

// Structure to represent a color (RGBA)
typedef struct {
    CGFloat r;
    CGFloat g;
    CGFloat b;
    CGFloat a;
} color_t;

// Function to draw a filled rectangle in a CGContext
void drawFilledRectangle(CGContextRef context, CGRect rect, color_t color) {
    CGColorRef fillColor = CGColorCreateGenericRGB(color.r, color.g, color.b, color.a);
    CGContextSetFillColorWithColor(context, fillColor);
    CGColorRelease(fillColor);
    CGContextFillRect(context, rect);
}

int main() {
    // 1. Read corner indices from memin.txt
    FILE *mf = fopen("memin.txt", "r");
    if (!mf) {
        fprintf(stderr, "Error opening memin.txt\n");
        return 1;
    }
    unsigned int word;
    unsigned int A=0, B=0, C=0, D=0;
    
    // Skip first 0x100 (256) entries
    for (unsigned int i = 0; i < 0x100; i++) {
        if (fscanf(mf, "%x", &word) != 1) {
            fprintf(stderr, "memin.txt too short at line %u\n", i);
            fclose(mf);
            return 1;
        }
    }
    // Read A, B, C, D
    if (fscanf(mf, "%x", &A) != 1 ||
        fscanf(mf, "%x", &B) != 1 ||
        fscanf(mf, "%x", &C) != 1 ||
        fscanf(mf, "%x", &D) != 1) {
        fprintf(stderr, "Error reading corner indices from memin.txt\n");
        fclose(mf);
        return 1;
    }
    fclose(mf);

    // Decode into (row, col)
    int row_start = (A >> 8) & 0xFF;
    int col_start =  A       & 0xFF;
    int row_end   = (C >> 8) & 0xFF;
    int col_end   =  C       & 0xFF;

    // 2. Create a bitmap graphics context (256x256 monochrome brighten to RGBA)
    const int width = 256;
    const int height = 256;
    const int bitsPerComponent = 8;
    const int bytesPerPixel = 4;
    const int bytesPerRow = bytesPerPixel * width;
    const int totalBytes = bytesPerRow * height;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *bitmapData = malloc(totalBytes);
    CGContextRef context = CGBitmapContextCreate(bitmapData,
                                                 width,
                                                 height,
                                                 bitsPerComponent,
                                                 bytesPerRow,
                                                 colorSpace,
                                                 kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);

    if (!context) {
        fprintf(stderr, "Error creating bitmap context\n");
        free(bitmapData);
        return 1;
    }

    // Clear to black
    CGContextSetFillColorWithColor(context, CGColorCreateGenericRGB(0,0,0,1));
    CGContextFillRect(context, CGRectMake(0,0,width,height));

    // 3. Draw the rectangle based on decoded coordinates
    CGRect rect = CGRectMake(col_start, row_start,
                             col_end - col_start + 1,
                             row_end - row_start + 1);
    color_t whiteColor = {1.0, 1.0, 1.0, 1.0};
    drawFilledRectangle(context, rect, whiteColor);

    // 4. Output as PPM via stdout
    printf("P3\n%d %d\n255\n", width, height);
    CGImageRef image = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(image));
    const unsigned char *pixels = CFDataGetBytePtr(data);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = (y * width + x) * 4;
            printf("%d %d %d ", pixels[offset], pixels[offset+1], pixels[offset+2]);
        }
        printf("\n");
    }

    CFRelease(data);
    CGImageRelease(image);
    free(bitmapData);
    return 0;
}
