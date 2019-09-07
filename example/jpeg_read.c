/***************************************************
    To read a jpg image file and download
    it as a texture map for openGL
    Derived from Tom Lane's example.c
    -- Obtain & install jpeg stuff from web
    (jpeglib.h, jerror.h jmore.h, jconfig.h,jpeg.lib)
****************************************************/
#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>
//================================
void load_jpeg(char* filename, unsigned char *jdata,
               int *image_height, int *image_width)
//================================
{
    unsigned char * rowptr[1];    // pointer to an array
    struct jpeg_decompress_struct info; //for our jpeg info
    struct jpeg_error_mgr err;          //the error handler

    FILE* file = fopen(filename, "rb");  //open the file

    info.err = jpeg_std_error(& err);
    jpeg_create_decompress(& info);   //fills info structure

    //if the jpeg file doesn't load
    if (!file)
        fprintf(stderr, "Error reading JPEG file %s!", filename);

    jpeg_stdio_src(&info, file);
    jpeg_read_header(&info, TRUE);   // read jpeg file header
    if (image_height)
        *image_height = info.image_height;
    if (image_width)
        *image_width = info.image_width;
    info.out_color_space = JCS_EXT_BGR;
    jpeg_start_decompress(&info);    // decompress the file

    //--------------------------------------------
    // read scanlines one at a time & put bytes
    //    in jdata[] array. Assumes an RGB image
    //--------------------------------------------
    /* jdata = (unsigned char *)malloc(data_size); */
    while (info.output_scanline < info.output_height) // loop
    {
        // Enable jpeg_read_scanlines() to fill our jdata array
        rowptr[0] = (unsigned char *)jdata +  // secret to method
            3* info.output_width * info.output_scanline;

        jpeg_read_scanlines(&info, rowptr, 1);
    }
    //---------------------------------------------------

    jpeg_finish_decompress(&info);   //finish decompressing

    jpeg_destroy_decompress(&info);
    fclose(file);                    //close the file
}
