#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "lightnet.h"

/* load jpeg image using libjpeg */
extern void load_jpeg(char* filename, unsigned char *jdata,
                      int *image_height, int *image_width);

static const int IMG_H = 360;
static const int IMG_W = 640;
static const int INPUT_H = 368;
static const int INPUT_W = 640;
static const int CONVOUT_H = 12;
static const int CONVOUT_W = 20;
static const int ANCHOR_PER_GRID = 9;
static const float ANCHOR_SHAPE[] = { 229., 137., 48., 71., 289., 245.,
                                      185., 134., 85., 142., 31., 41.,
                                      197., 191., 237., 206., 63., 108.};

static void exit_usage(int exit_code)
{
    const char *usage = "\
Usage: object-detect [options] NET_FILE WEIGHT_FILE IMG_DIR\n\
Do object detection of *.jpg images in directory IMG_DIR with network model\n\
in NET_FILE and weight data in WEIGHT_FILE datain directory using C API.\n\
\n\
options:\n\
    -h    print this message\n\
";
    fprintf(stderr, "%s", usage);
    exit(exit_code);
}

/* the original algorithm is from SqueezeDet's official source code */
static void prepare_anchors(float *anchors)
{
     float center_x[CONVOUT_W], center_y[CONVOUT_H];
     int i, j, k;

     for (i = 1; i <= CONVOUT_W; i++)
          center_x[i-1] = i * INPUT_W / (CONVOUT_W + 1.0);
     for (i = 1; i <= CONVOUT_H; i++)
          center_y[i-1] = i * INPUT_H / (CONVOUT_H + 1.0);

     int h_vol = CONVOUT_W * ANCHOR_PER_GRID * 4;
     int w_vol = ANCHOR_PER_GRID * 4;
     int b_vol = 4;
     for (i = 0; i < CONVOUT_H; i++) {
          for (j = 0; j < CONVOUT_W; j++) {
               for (k = 0; k < ANCHOR_PER_GRID; k++) {
                    anchors[i*h_vol+j*w_vol+k*b_vol] = center_x[j];
                    anchors[i*h_vol+j*w_vol+k*b_vol+1] = center_y[i];
                    anchors[i*h_vol+j*w_vol+k*b_vol+2] = ANCHOR_SHAPE[k*2];
                    anchors[i*h_vol+j*w_vol+k*b_vol+3] = ANCHOR_SHAPE[k*2+1];
               }
          }
     }
}

/* filter for JPEG files */
static int jpeg_filter(const struct dirent *de)
{
    if (ln_streq(de->d_name, ".") || ln_streq(de->d_name, "..")
        || !ln_subfixed(de->d_name, ".jpg"))
        return 0;
    return 1;
}

static char **create_jpeg_filelist(const char *dir_name)
{
    struct dirent **files;
    char **filelist;
    size_t len;
    int n;

    if ((n = scandir(dir_name, &files, jpeg_filter, alphasort)) == -1)
        err(EXIT_FAILURE, "%s", dir_name);

    filelist = (char **)malloc(sizeof(char *) * (n + 1)); /* end with NULL */
    for (int i = 0; i < n; i++) {
        len = strlen(dir_name) + strlen(files[i]->d_name) + 2;
        filelist[i] = (char *)malloc(sizeof(char) * len);
        snprintf(filelist[i], len, "%s/%s", dir_name, files[i]->d_name);
        free(files[i]);
    }
    filelist[n] = NULL;
    free(files);

    return filelist;
}

static void free_jpeg_filelist(char **filelist)
{
    for (int i = 0; filelist[i]; i++)
        free(filelist[i]);
    free(filelist);
}

static void do_detection(ln_context *ctx, const char *img_dir)
{
    char **filelist;
    int filenum = 0;
    double time, total_time = 0;
    unsigned char *img;         /* image data */
    int img_width, img_height;  /* image shape */
    float *anchors;             /* anchors for object detection */
    float bbox[4];              /* bounding box result */

    /* prepare and set anchor data, "anchors" is a tensor defined in model */
    anchors = (float *)malloc(CONVOUT_H * CONVOUT_W * ANCHOR_PER_GRID * 4
                              * sizeof(float));
    prepare_anchors(anchors);
    ln_context_set_data(ctx, "anchors", anchors);
    free(anchors);

    /* get a list of JEPG files */
    filelist = create_jpeg_filelist(img_dir);
    img = (unsigned char *)malloc(sizeof(unsigned char) * IMG_H * IMG_W * 3);

    for (int i = 0; filelist[i]; i++) {
        load_jpeg(filelist[i], img, &img_height, &img_width);
        LN_TIMEIT_START;

        /* set input data, 'input' is a tensor defined in the net */
        ln_context_set_data(ctx, "input", img);
        /* Set the original width and height of the image as parameters of
         * operator 'transform_bboxSQD0' in the net.
         * This is not necessary though, since the images' shape is fixed.
         * Just to show the way to set operator parameters while running */
        ln_context_set_param(ctx, "transform_bboxSQD0", "img_width",
                             (double)img_width);
        ln_context_set_param(ctx, "transform_bboxSQD0", "img_height",
                             (double)img_height);
        /* run the net and copy output data 'final_bbox' to bbox, 'final_bbox' is
         * a tensor defined in the net */
        ln_context_run(ctx);
        ln_context_get_data(ctx, "final_bbox", bbox);

        LN_TIMEIT_END(&time);
        total_time += time;
        filenum++;
        printf("[%f, %f, %f, %f]\n", bbox[0], bbox[1], bbox[2], bbox[3]);
    }
    printf("total image number: %d\n", filenum);
    printf("frames per second of detection: %f\n", filenum / total_time);

    free(img);
    free_jpeg_filelist(filelist);
}

int main(int argc, char **argv)
{
    const char *net;     /* neural network model file */
    const char *wts;     /* neural network weight file */
    const char *img_dir; /* image directory */

    ln_context *ctx;     /* lightnet context for compilation and inference */

    /* parse command line arguments */
    if (argc >= 2 && ln_streq(argv[1], "-h"))
        exit_usage(EXIT_SUCCESS);
    if (argc != 4)
        exit_usage(EXIT_FAILURE);
    net = argv[1];
    wts = argv[2];
    img_dir = argv[3];

    /* initialize lightnet, compile network and load weight data */
    ln_arch_init();
    ctx = ln_context_create();
    ln_context_init(ctx, net);
    ln_context_compile(ctx, "tensorrt");
    ln_context_load(ctx, wts);

    /* do inference for images in 'img_dir' */
    do_detection(ctx, img_dir);

    /* clean up */
    ln_context_unload(ctx);
    ln_context_cleanup(ctx);
    ln_context_free(ctx);
    ln_arch_cleanup();

    return 0;
}
