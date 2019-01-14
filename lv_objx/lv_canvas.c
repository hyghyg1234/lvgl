/**
 * @file lv_canvas.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_canvas.h"
#if USE_LV_CANVAS != 0

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t lv_canvas_signal(lv_obj_t * canvas, lv_signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_signal_func_t ancestor_signal;
static lv_design_func_t ancestor_design;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a canvas object
 * @param par pointer to an object, it will be the parent of the new canvas
 * @param copy pointer to a canvas object, if not NULL then the new object will be copied from it
 * @return pointer to the created canvas
 */
lv_obj_t * lv_canvas_create(lv_obj_t * par, const lv_obj_t * copy)
{
    LV_LOG_TRACE("canvas create started");

    /*Create the ancestor of canvas*/
    lv_obj_t * new_canvas = lv_img_create(par, copy);
    lv_mem_assert(new_canvas);
    if(new_canvas == NULL) return NULL;

    /*Allocate the canvas type specific extended data*/
    lv_canvas_ext_t * ext = lv_obj_allocate_ext_attr(new_canvas, sizeof(lv_canvas_ext_t));
    lv_mem_assert(ext);
    if(ext == NULL) return NULL;
    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_func(new_canvas);
    if(ancestor_design == NULL) ancestor_design = lv_obj_get_design_func(new_canvas);

    /*Initialize the allocated 'ext' */
    ext->dsc.header.always_zero = 0;
    ext->dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    ext->dsc.header.h = 0;
    ext->dsc.header.w = 0;
    ext->dsc.data_size = 0;
    ext->dsc.data = NULL;

    lv_img_set_src(new_canvas, &ext->dsc);

    /*The signal and design functions are not copied so set them here*/
    lv_obj_set_signal_func(new_canvas, lv_canvas_signal);

    /*Init the new canvas canvas*/
    if(copy == NULL) {

    }
    /*Copy an existing canvas*/
    else {
        lv_canvas_ext_t * copy_ext = lv_obj_get_ext_attr(copy);

        /*Refresh the style with new signal function*/
        lv_obj_refresh_style(new_canvas);
    }

    LV_LOG_INFO("canvas created");

    return new_canvas;
}

/*======================
 * Add/remove functions
 *=====================*/

/**
 * Set the color of a pixel on the canvas
 * @param canvas
 * @param x x coordiante of the point to set
 * @param y x coordiante of the point to set
 * @param c color of the point
 */
void lv_canvas_set_px(lv_obj_t * canvas, lv_coord_t x, lv_coord_t y, lv_color_t c)
{

    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
    if(x >= ext->dsc.header.w || y >= ext->dsc.header.h) {
        LV_LOG_WARN("lv_canvas_set_px: x or y out of the canvas");
        return;
    }

    uint32_t px_size = lv_img_color_format_get_px_size(ext->dsc.header.cf) >> 3;
    uint32_t px = ext->dsc.header.w * y * px_size + x * px_size;

    memcpy((void*)&ext->dsc.data[px], &c, sizeof(lv_color_t));
}


/*=====================
 * Setter functions
 *====================*/

/**
 * Set a buffer for the canvas.
 * @param buf a buffer where the contant of the canvas will be.
 * The required size is (lv_img_color_format_get_px_size(cf) * w * h) / 8)
 * It can be allocated with `lv_mem_alloc()` or
 * it can be statically allocated array (e.g. static lv_color_t buf[100*50]) or
 * it can be an address in RAM or external SRAM
 * @param canvas pointer to a canvas obejct
 * @param w width of the canvas
 * @param h hight of the canvas
 * @param cf color format. The following formats are supported:
 * LV_IMG_CF_TRUE_COLOR,  LV_IMG_CF_TRUE_COLOR_ALPHA, LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED
 *
 */
void lv_canvas_set_buffer(lv_obj_t * canvas, void * buf, lv_coord_t w, lv_coord_t h, lv_img_cf_t cf)
{
    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);

    ext->dsc.header.cf = cf;
    ext->dsc.header.w = w;
    ext->dsc.header.h = h;
    ext->dsc.data = buf;
    ext->dsc.data_size = (lv_img_color_format_get_px_size(cf) * w * h) / 8;

    lv_img_set_src(canvas, &ext->dsc);
}

/**
 * Set a style of a canvas.
 * @param canvas pointer to canvas object
 * @param type which style should be set
 * @param style pointer to a style
 */
void lv_canvas_set_style(lv_obj_t * canvas, lv_canvas_style_t type, lv_style_t * style)
{
    switch(type) {
        case LV_CANVAS_STYLE_MAIN:
            lv_img_set_style(canvas, style);
            break;
    }
}

/*=====================
 * Getter functions
 *====================*/

/**
 * Get style of a canvas.
 * @param canvas pointer to canvas object
 * @param type which style should be get
 * @return style pointer to the style
 */
lv_style_t * lv_canvas_get_style(const lv_obj_t * canvas, lv_canvas_style_t type)
{
    lv_canvas_ext_t * ext = lv_obj_get_ext_attr(canvas);
    lv_style_t * style = NULL;

    switch(type) {
        case LV_CANVAS_STYLE_MAIN:
            style = lv_img_get_style(canvas);
            break;
        default:
            style =  NULL;
    }

    return style;
}

/*=====================
 * Other functions
 *====================*/


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Signal function of the canvas
 * @param canvas pointer to a canvas object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t lv_canvas_signal(lv_obj_t * canvas, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(canvas, sign, param);
    if(res != LV_RES_OK) return res;


    if(sign == LV_SIGNAL_CLEANUP) {
        /*Nothing to cleanup. (No dynamically allocated memory in 'ext')*/
    } else if(sign == LV_SIGNAL_GET_TYPE) {
        lv_obj_type_t * buf = param;
        uint8_t i;
        for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) {  /*Find the last set data*/
            if(buf->type[i] == NULL) break;
        }
        buf->type[i] = "lv_canvas";
    }

    return res;
}

#endif
