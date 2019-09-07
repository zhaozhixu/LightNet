#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include "lightnet.h"

extern void load_jpeg(char* filename, unsigned char *jdata,
                      int *image_height, int *image_width);

static void exit_usage(int exit_code)
{
    const char *usage = "\
Usage: object-detect [options] DIR\n\
Do object detection of *.jpg files in directory DIR using C API.\n\
\n\
options:\n\
    -h    print this message\n\
";
    fprintf(stderr, "%s", usage);
    exit(exit_code);
}

static const int MAX_PATH_LEN = 1024;
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

int main(int argc, char **argv)
{
    ln_context *ctx;
    const char *net;
    const char *wts;
    struct dirent *de;
    DIR *dir;
    const char *img_dir;
    char img_path[MAX_PATH_LEN];
    unsigned char *img;
    float *anchors;
    float bbox[4];

    if (argc >= 2 && ln_streq(argv[1], "-h"))
        exit_usage(EXIT_SUCCESS);
    if (argc != 4)
        exit_usage(EXIT_FAILURE);
    net = argv[1];
    wts = argv[2];
    img_dir = argv[3];

    ln_arch_init();
    ctx = ln_context_create();
    ln_context_init(ctx, net);
    ln_context_compile(ctx, "tensorrt");
    ln_context_load(ctx, wts);

    anchors = malloc(CONVOUT_H * CONVOUT_W * ANCHOR_PER_GRID * 4
                     * sizeof(float);
    prepare_anchors(anchors);
    ln_context_set_data(ctx, "anchors", anchors);
    free(anchors);

    if (!(dir = opendir(img_dir)))
        err(EXIT_FAILURE, "%s", img_dir);
    img = malloc(sizeof(unsigned char) * IMG_H * IMG_W * 3);
    while ((de = readdir(dir))) {
        if (ln_streq(de->d_name, ".") || ln_streq(de->d_name, "..")
            || !ln_subfixed(de->d_name, ".jpg"))
            continue;
        snprintf(img_path, MAX_PATH_LEN, "%s/%s", img_dir, de->d_name);
        load_jpeg(img_path, img, NULL, NULL);
        ln_context_set_data(ctx, "input", img);
        LN_TIMEIT_START;
        ln_context_run(ctx);
        LN_TIMEIT_END("run time: ");
        ln_context_get_data(ctx, "final_bbox", bbox);
        printf("bbox = [%f, %f, %f, %f]\n", bbox[0], bbox[1], bbox[2], bbox[3]);
    }

    ln_context_unload(ctx);
    ln_context_cleanup(ctx);
    ln_context_free(ctx);
    ln_arch_cleanup();
    free(img);

    return 0;
}
