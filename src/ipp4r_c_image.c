#include <ipp.h>
#include <ruby.h>
#include <highgui.h> /* REMOVE ME */
#include <time.h> /* for rand seed */
#include <stdio.h> /* for debug purposes */
#include "ipp4r.h"
#include "ipp4r_metatype.h"

// -------------------------------------------------------------------------- //
// Image accessors
// -------------------------------------------------------------------------- //
#define IS_SUBIMAGE(IMAGE) ((IMAGE)->is_subimage)
#define HEIGHT(IMAGE) ((IS_SUBIMAGE(IMAGE) ? (IMAGE)->height : (IMAGE)->data->height))
#define WIDTH(IMAGE) ((IS_SUBIMAGE(IMAGE) ? (IMAGE)->width : (IMAGE)->data->width))
#define PIXELSIZE(IMAGE) ((IMAGE)->data->pixelSize)
#define PIXELS(IMAGE) (image_pixels(IMAGE))
#define PIXEL_AT(IMAGE, X, Y) ((void*)((char*) PIXELS(IMAGE) + Y * WSTEP(IMAGE) + X * PIXELSIZE(IMAGE)))
#define WSTEP(IMAGE) ((IMAGE)->data->wStep)
#define IPPISIZE(IMAGE) (image_ippisize(IMAGE))

#define METATYPE(IMAGE) ((IMAGE)->data->metaType)
#define CHANNELS(IMAGE) (metatype_channels(METATYPE(IMAGE)))
#define DATATYPE(IMAGE) (metatype_datatype(METATYPE(IMAGE)))


// -------------------------------------------------------------------------- //
// Helpers
// -------------------------------------------------------------------------- //
#define PWPWI(src, dst) PIXELS(src), WSTEP(src), PIXELS(dst), WSTEP(dst), IPPISIZE(dst)
#define PWI(src) PIXELS(src), WSTEP(src), IPPISIZE(src)

// -------------------------------------------------------------------------- //
// Supplementary functions
// -------------------------------------------------------------------------- //
/**
 * @returns IppiSize of an Image
 */
static IppiSize image_ippisize(Image* image) {
  IppiSize result;

  assert(image != NULL);

  result.height = HEIGHT(image);
  result.width = WIDTH(image);
  return result;
}


/**
 * @returns pixels pointer of an image
 */
static void* image_pixels(Image* image) {
  assert(image != NULL);

  if(IS_SUBIMAGE(image))
    return (char*) image->data->pixels + image->y * WSTEP(image) + image->x * PIXELSIZE(image);
  else
    return image->data->pixels;
}


// -------------------------------------------------------------------------- //
// image_height
// -------------------------------------------------------------------------- //
int image_height(Image* image) {
  assert(image != NULL);

  return HEIGHT(image);
}


// -------------------------------------------------------------------------- //
// image_width
// -------------------------------------------------------------------------- //
int image_width(Image* image) {
  assert(image != NULL);

  return WIDTH(image);
}


// -------------------------------------------------------------------------- //
// image_channels
// -------------------------------------------------------------------------- //
IppChannels image_channels(Image* image) {
  assert(image != NULL);

  return CHANNELS(image);
}


// -------------------------------------------------------------------------- //
// image_datatype
// -------------------------------------------------------------------------- //
IppDataType image_datatype(Image* image) {
  assert(image != NULL);

  return DATATYPE(image);
}


// -------------------------------------------------------------------------- //
// image_metatype
// -------------------------------------------------------------------------- //
IppMetaType image_metatype(Image* image) {
  assert(image != NULL);

  return METATYPE(image);
}


// -------------------------------------------------------------------------- //
// image_new
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_new, (int width, int height, IppMetaType metaType)) {
  Image* image;
  Data* data;

  assert(width > 0 && height > 0);

  /* See motivation for malloc usage in the source for data_new() function */
  image = (Image*) malloc(sizeof(Image));
  if(image == NULL)
    TRACE_RETURN(NULL);

  data = data_new(width, height, metaType);
  if(data == NULL) {
    free(image);
    TRACE_RETURN(NULL);
  }

  image->data = data;
  image->is_subimage = FALSE;
  image->shared = FALSE; // initially image is not shared

  TRACE_RETURN(image);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_subimage
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_subimage, (Image* image, int x, int y, int width, int height, int* pStatus)) {
  Image* result;
  int status = ippStsNoErr;
  
  assert(image != NULL);
  assert(image->shared);
  assert(x >= 0 && y >= 0 && x + width <= WIDTH(image) && y + height <= HEIGHT(image));

  result = (Image*) malloc(sizeof(Image));
  if(result == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

  result->data = image->data; // data is shared
  result->rb_data = image->rb_data;
  result->is_subimage = TRUE;
  result->x = x;
  result->y = y;
  result->width = width;
  result->height = height;

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_destroy
// -------------------------------------------------------------------------- //
TRACE_FUNC(void, image_destroy, (Image* image)) {
  assert(image != NULL);

  if(!image->shared)
    data_destroy(image->data);
  free(image);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_mark
// -------------------------------------------------------------------------- //
TRACE_FUNC(void, image_mark, (Image* image)) {
  /* I check for NULL not because of paranoia.
   * The problem is that in rb_Image_alloc we initialize image with NULL, and subsequent call to rb_Image_initialize may trigger gc while our image is still NULL! */
  if(image != NULL && image->shared)
    rb_gc_mark(image->rb_data);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_share
// -------------------------------------------------------------------------- //
TRACE_FUNC(void, image_share, (Image* image)) {
  assert(image != NULL);

  image->shared = TRUE;
  image->rb_data = Data_Wrap_Struct(rb_Data, NULL, data_destroy, image->data);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_wrap
// -------------------------------------------------------------------------- //
TRACE_FUNC(VALUE, image_wrap, (Image* image)) {
  if(image != NULL) /* check for NULL is needed, see rb_Image_alloc */
    image_share(image);
  TRACE_RETURN(Data_Wrap_Struct(rb_Image, image_mark, image_destroy, image));
} TRACE_END


// -------------------------------------------------------------------------- //
// image_clone
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_clone, (Image* image, int* pStatus)) {
  Image* result;
  int status;

  assert(image != NULL);

  result = image_new(WIDTH(image), HEIGHT(image), METATYPE(image));

  if(result != NULL) {
    IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, IPPMETAFUNC, (ippiCopy_, R, (PWPWI(image, result))), Unreachable(), ippStsBadArgErr);
    if(IS_ERROR(status)) {
      image_destroy(result);
      result = NULL;
    }
  }

  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_load
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_load, (const char* fileName, int* pStatus)) {
  IplImage* iplImage;
  Image* image;
  int status;

  assert(fileName != NULL);

  iplImage = cvLoadImage(fileName, CV_LOAD_IMAGE_COLOR);
  if(iplImage == NULL) {
    status = ippStsErr; /* seem to be the only reasonable error value */
    goto end;
  }

  image = image_new(iplImage->width, iplImage->height, ipp8u_C3); // always succeeds or throws
  if(image == NULL) {
    status = ippStsNoMemErr;
    goto error;
  }

  status = ippiCopy_8u_C3R(iplImage->imageData, iplImage->widthStep, PWI(image));

error:
  cvReleaseImage(&iplImage);

end:
  if(pStatus != NULL)
    *pStatus = status;

  TRACE_RETURN(image);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_save
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_save, (Image* image, const char* fileName)) {
  IplImage* iplImage;
  Image* converted;
  int status;
  int result;

  assert(image != NULL && fileName != NULL);

  if(METATYPE(image) != ipp8u_C3) {
    converted = image_convert_copy(image, ipp8u_C3, &status);
    if(IS_ERROR(status)) {
      assert(converted == NULL);
      TRACE_RETURN(status);
    }
  } else
    converted = image;

  iplImage = cvCreateImage(cvSize(WIDTH(image), HEIGHT(image)), IPL_DEPTH_8U, 3);
  if(iplImage == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

  status = ippiCopy_8u_C3R(PIXELS(converted), WSTEP(converted), iplImage->imageData, iplImage->widthStep, IPPISIZE(converted));
  if(IS_ERROR(status))
    goto error;
  
  result = cvSaveImage(fileName, iplImage); /* cvSaveImage returns zero on failure, non-zero otherwise */
  if(result == 0)
    status = ippStsErr; /* seem to be the only reasonable error value */

error:
  cvReleaseImage(&iplImage);

end:
  if(METATYPE(image) != ipp8u_C3)
    image_destroy(converted);
  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_addranduniform
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_addranduniform, (Image* image, IppMetaNumber lo, IppMetaNumber hi)) {
  unsigned int seed;
  int status;
  static unsigned int seedMod = 0;

  assert(image != NULL);

  seed = (unsigned int) time(NULL) + seedMod++;

#define METAFUNC(M, ARG) ARX_JOIN_3(ippiAddRandUniform_Direct_, M, IR) (PWI(image), M2C_NUMBER(M, lo), M2C_NUMBER(M, hi), &seed)
  IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, METAFUNC, ~, Unreachable(), ippStsBadArgErr);
#undef METAFUNC  
  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert_datatype_copy
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_convert_datatype_copy, (Image* image, IppDataType dataType, int* pStatus)) {
  int status;
  Image *result;
  Ipp32f scaleColor[4] = {D_SCALE(16u), D_SCALE(16u), D_SCALE(16u), D_SCALE(16u)};

  assert(image != NULL);

  TRACE(("image_convert_datatype from %d to %d", DATATYPE(image), dataType));

  if(DATATYPE(image) == dataType)
    TRACE_RETURN(image_clone(image, pStatus));

  TRACE(("%d %d %d", image, image->data, image->data->metaType));
  result = image_new(WIDTH(image), HEIGHT(image), metatype_compose(dataType, CHANNELS(image)));
  TRACE(("%d %d %d", image, image->data, image->data->metaType));

  if(result == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

  TRACE(("image_convert_datatype from %d to %d", DATATYPE(image), dataType));

#define CONVERT(D, C, NEW_D)                                                    \
  IF_D_EQ_D(D, NEW_D,                                                           \
    Unreachable(),                                                              \
    ARX_IF(DD_IN_DDA((D, NEW_D), (2, ((16u, 8u), (8u, 16u)))),                  \
      status = ARX_JOIN_6(ippiScale_, D, NEW_D, _, C, R) (PWPWI(image, result) ARX_COMMA_IF(D_EQ_D(D, 16u)) ARX_IF(D_EQ_D(D, 16u), ippAlgHintNone, ARX_EMPTY())), \
      ARX_IF(DD_IN_DDA((D, NEW_D), (2, ((32f, 8u), (8u, 32f)))),                \
        status = ARX_JOIN_6(ippiScale_, D, NEW_D, _, C, R) (PWPWI(image, result), 0.0f, 1.0f), \
        IF_DD_EQ_DD((D, NEW_D), (16u, 32f),                                     \
          status = ARX_JOIN_6(ippiConvert_, D, NEW_D, _, C, R) (PWPWI(image, result)); \
          if(!IS_ERROR(status))                                                 \
            status = ARX_JOIN_5(ippiDivC_, NEW_D, _, IF_C_EQ_C(C, AC4, C4, C), IR) (scaleColor IF_C_EQ_C(C, C1, [0], ARX_EMPTY()), PWI(result)), \
          IF_DD_EQ_DD((D, NEW_D), (32f, 16u),                                   \
            {                                                                   \
              Image* newImage = image_new(WIDTH(image), HEIGHT(image), METATYPE(image)); /* TODO */ \
              if(newImage != NULL) {                                            \
                status = ARX_JOIN_5(ippiMulC_, D, _, IF_C_EQ_C(C, AC4, C4, C), R) (PIXELS(image), WSTEP(image), scaleColor IF_C_EQ_C(C, C1, [0], ARX_EMPTY()), PWI(newImage)); \
                if(!IS_ERROR(status))                                           \
                  status = ARX_JOIN_6(ippiConvert_, D, NEW_D, _, C, R) (PWPWI(newImage, result), ippRndZero); \
                image_destroy(newImage);                                        \
              }                                                                 \
            },                                                                  \
            OMG_TEH_DRAMA                                                       \
          )                                                                     \
        )                                                                       \
      )                                                                         \
    )                                                                           \
  )
#define METAFUNC_2(NEW_D, M) CONVERT(M_DATATYPE(M), M_CHANNELS(M), NEW_D)
#define METAFUNC(M, ARGS) IPPMETACALL(dataType, ARX_EMPTY(), D_SUPPORTED, METAFUNC_2, M, Unreachable(); status = ippStsBadArgErr, ARX_EMPTY())
  IPPMETACALL(METATYPE(image), ARX_EMPTY(), M_SUPPORTED, METAFUNC, ~, Unreachable(); status = ippStsBadArgErr, ARX_EMPTY());
#undef METAFUNC
#undef METAFUNC_2
#undef CONVERT

  TRACE(("end_convert"));

  if(IS_ERROR(status)) {
    image_destroy(result);
    result = NULL;
  }

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert_datatype
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_convert_datatype, (Image* image, IppDataType dataType)) {
  int status;
  Image *result;

  assert(image != NULL);

  if(DATATYPE(image) == dataType)
    TRACE_RETURN(ippStsNoErr);

  result = image_convert_datatype_copy(image, dataType, &status);
  if(IS_ERROR(status)) {
    assert(result == NULL);
    TRACE_RETURN(status);
  }

  data_swap(image->data, result->data);
  image_destroy(result);

  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert_channels_copy
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_convert_channels_copy, (Image* image, IppChannels channels, int* pStatus)) {
  int status;
  Image *result;

  assert(image != NULL);

  if(CHANNELS(image) == channels)
    TRACE_RETURN(image_clone(image, pStatus));

  result = image_new(WIDTH(image), HEIGHT(image), metatype_compose(DATATYPE(image), channels));
  if(result == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

  #define CONVERT(D, C, NEW_C)                                                  \
  IF_C_EQ_C(C, NEW_C,                                                           \
    Unreachable(),                                                              \
    ARX_IF(CC_IN_CCA((C, NEW_C), (2, ((AC4, C3), (C3, AC4)))),                  \
      status = ARX_JOIN_6(ippiCopy_, D, _, C, NEW_C, R) (PWPWI(image, result)); \
      IF_CC_EQ_CC((C, NEW_C), (C3, AC4),                                        \
        if(!IS_ERROR(status))                                                   \
          status = ARX_JOIN_3(ippiSet_, D, _C4CR) (D_MAX(D), ((D_CTYPE(D)*) PIXELS(result)) + 3, WSTEP(result), IPPISIZE(result)), \
        ARX_EMPTY()                                                             \
      ),                                                                        \
      ARX_IF(CC_IN_CCA((C, NEW_C), (2, ((C3, C1), (AC4, C1)))),                 \
        status = ARX_JOIN_6(ippiRGBToGray_, D, _, C, NEW_C, R) (PWPWI(image, result)), \
        ARX_IF(ARX_AND(D_EQ_D(D, 8u), CC_EQ_CC((C, NEW_C), (C1, C3))),          \
          status = ARX_JOIN_6(ippiDup_, D, _, C, NEW_C, R) (PWPWI(image, result)), \
          ARX_IF(CC_IN_CCA((C, NEW_C), (2, ((C1, C3), (C1, AC4)))),             \
            {                                                                   \
              D_CTYPE(D)* pSrc[4];                                              \
              pSrc[0] = pSrc[1] = pSrc[2] = pSrc[3] = (D_CTYPE(D)*) PIXELS(image); \
              status = ARX_JOIN_7(ippiCopy_, D, _, P, C_CNUMB(NEW_C), IF_C_EQ_C(NEW_C, AC4, C4, NEW_C), R) (pSrc, WSTEP(image), PIXELS(result), WSTEP(result), IPPISIZE(result)); \
              IF_C_EQ_C(NEW_C, AC4,                                             \
                if(!IS_ERROR(status))                                           \
                  status = ARX_JOIN_3(ippiSet_, D, _C4CR) (D_MAX(D), ((D_CTYPE(D)*) PIXELS(result)) + 3, WSTEP(result), IPPISIZE(result));, \
                ARX_EMPTY()                                                     \
              )                                                                 \
            },                                                                  \
            OMG_TEH_DRAMA                                                       \
          )                                                                     \
        )                                                                       \
      )                                                                         \
    )                                                                           \
  )
#define METAFUNC_2(NEW_C, M) CONVERT(M_DATATYPE(M), M_CHANNELS(M), NEW_C)
#define METAFUNC(M, ARGS) IPPMETACALL(channels, ARX_EMPTY(), C_SUPPORTED, METAFUNC_2, M, Unreachable(); status = ippStsBadArgErr, ARX_EMPTY())
  IPPMETACALL(METATYPE(image), ARX_EMPTY(), M_SUPPORTED, METAFUNC, ~, Unreachable(); status = ippStsBadArgErr, ARX_EMPTY());
#undef METAFUNC
#undef METAFUNC_2
#undef CONVERT

  if(IS_ERROR(status)) {
    image_destroy(result);
    result = NULL;
  }

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert_datatype
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_convert_channels, (Image* image, IppChannels channels)) {
  int status;
  Image *result;

  assert(image != NULL);
  assert(is_channels_supported(channels));

  if(CHANNELS(image) == channels)
    TRACE_RETURN(ippStsNoErr);

  result = image_convert_channels_copy(image, channels, &status);
  if(IS_ERROR(status)) {
    assert(result == NULL);
    TRACE_RETURN(status);
  }

  data_swap(image->data, result->data);
  image_destroy(result);

  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert_copy
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_convert_copy, (Image* image, IppMetaType metaType, int* pStatus)) {
  int status;
  Image* result;
  IppChannels channels;
  IppDataType dataType;

  assert(image != NULL);
  assert(is_metatype_supported(metaType));

  TRACE(("begin_convert from %d to %d", METATYPE(image), metaType));

  channels = metatype_channels(metaType);
  dataType = metatype_datatype(metaType);

  if(CHANNELS(image) != channels) {
    result = image_convert_channels_copy(image, channels, &status);
    if(IS_ERROR(status)) {
      assert(result == NULL);
      goto end;
    }
    
    status = image_convert_datatype(result, dataType);
    if(IS_ERROR(status)) {
      image_destroy(result);
      result = NULL;
    }
  } else
    TRACE_RETURN(image_convert_datatype_copy(image, dataType, pStatus));

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_convert
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_convert, (Image* image, IppMetaType metaType)) {
  int status;
  Image* result;

  assert(image != NULL);
  assert(is_metatype_supported(metaType));

  if(METATYPE(image) == metaType)
    TRACE_RETURN(ippStsNoErr);

  result = image_convert_copy(image, metaType, &status);
  if(IS_ERROR(status)) {
    assert(result == NULL);
    TRACE_RETURN(status);
  }

  data_swap(image->data, result->data);
  image_destroy(result);

  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_get_pixel
// -------------------------------------------------------------------------- //
int image_get_pixel(Image* image, int x, int y, Color* color) {
  void* p;

  assert(image != NULL);
  assert(x >= 0 && y >= 0 && x < WIDTH(image) && y <= HEIGHT(image));

  p = PIXEL_AT(image, x, y);

#define METAFUNC(M, ARGS) C2M_COLOR_TO(M, p, color->as_array)
  IPPMETACALL(METATYPE(image), ARX_EMPTY(), M_SUPPORTED, METAFUNC, ~, Unreachable(); return ippStsBadArgErr, ARX_EMPTY())
#undef METAFUNC

  return ippStsNoErr;
}


// -------------------------------------------------------------------------- //
// image_set_pixel
// -------------------------------------------------------------------------- //
int image_set_pixel(Image* image, int x, int y, Color* color) {
  void* p;

  assert(image != NULL);
  assert(x >= 0 && y >= 0 && x < WIDTH(image) && y <= HEIGHT(image));

  p = PIXEL_AT(image, x, y);

#define METAFUNC(M, ARGS) M2C_COLOR_TO(M, color->as_array, p)
  IPPMETACALL(METATYPE(image), ARX_EMPTY(), M_SUPPORTED, METAFUNC, ~, Unreachable(); return ippStsBadArgErr, ARX_EMPTY())
#undef METAFUNC

  return ippStsNoErr;
}


// -------------------------------------------------------------------------- //
// image_fill
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_fill, (Image* image, Color* color)) {
  int status;
  USING_M2C_COLOR(1);

  assert(image != NULL && color != NULL);

#define METAFUNC(M, ARGS) ARX_JOIN_3(ippiSet_, M, R) (M2C_COLOR(M, color->as_array, 0), PWI(image))
  IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, METAFUNC, ~, Unreachable(), ippStsBadArgErr);
#undef METAFUNC

  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_transpose_copy
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_transpose_copy, (Image* image, int* pStatus)) {
  Image* result;
  int status;

  assert(image != NULL);

  result = image_new(HEIGHT(image), WIDTH(image), CHANNELS(image));
  if(result == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

#define METAFUNC(M, ARGS) ARX_JOIN_3(ippiTranspose_, M_REPLACE_C_IF_C(M_REPLACE_D_IF_D(M, 32f, 32s), AC4, C4), R) (PWPWI(image, result)) /* TODO: test this hack*/
  IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, METAFUNC, ~, Unreachable(), ippStsBadArgErr);
#undef METAFUNC

  if(IS_ERROR(status)) {
    image_destroy(result);
    result = NULL;
  }

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(result);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_threshold_copy
// -------------------------------------------------------------------------- //
TRACE_FUNC(Image*, image_threshold_copy, (Image* image, Color* threshold, IppCmpOp cmp, Color* value, int* pStatus)) {
  Image* newImage;
  int status;
  USING_M2C_COLOR(2);

  assert(image != NULL && threshold != NULL && value != NULL);
  assert(cmp == ippCmpLess || cmp == ippCmpGreater);

  newImage = image_new(WIDTH(image), HEIGHT(image), CHANNELS(image));
  if(newImage == NULL) {
    status = ippStsNoMemErr;
    goto end;
  }

#define METAFUNC(M, ARGS) ARX_JOIN_3(ippiThreshold_Val_, M, R)                  \
  (PIXELS(image), WSTEP(image), PIXELS(newImage), WSTEP(newImage), IPPISIZE(image), M2C_COLOR(M, threshold->as_array, 0), M2C_COLOR(M, value->as_array, 1), cmp)
  IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, METAFUNC, ~, Unreachable(), ippStsBadArgErr);
#undef METAFUNC

end:
  if(pStatus != NULL)
    *pStatus = status;
  TRACE_RETURN(newImage);
} TRACE_END


// -------------------------------------------------------------------------- //
// image_jaehne 
// -------------------------------------------------------------------------- //
TRACE_FUNC(int, image_jaehne, (Image* image)) {
  int status;

  assert(image != NULL);

  IPPMETACALL(METATYPE(image), status =, M_SUPPORTED, IPPMETAFUNC, (ippiImageJaehne_, R, (PWI(image))), Unreachable(), ippStsBadArgErr);

  TRACE_RETURN(status);
} TRACE_END


// -------------------------------------------------------------------------- //
// ippimage_gaussianblur
// -------------------------------------------------------------------------- //
/*IppStatus ippimage_gaussianblur(IppImage* src, IppImage* dst, float sigma) {
  IppStatus status;
  int bufferSize;
  int kernelSize;
  void* buffer;

  if(NOTALLOCATED(src) || sigma <= 0)
    return ippStsBadArgErr;

  kernelSize = ((int)(sigma * 4)) * 2 + 1;
  if(kernelSize < 3)
    kernelSize = 3;

  status = ippiFilterGaussGetBufferSize_32f_C1R(ippimage_ippisize(src), kernelSize, &bufferSize);
  if(IS_ERROR(status))
    return status;

  buffer = malloc(bufferSize);
  if(buffer == NULL)
    return ippStsNoMemErr;

  if(NOTALLOCATED(dst) || src->width != dst->width || src->height != dst->height) {
    ippimage_freebuffer(dst);
    status = ippimage_allocbuffer(dst, src->width, src->height);
    if(IS_ERROR(status))
      goto error;
  }

  status = ippiFilterGaussBorder_32f_C1R(src->data, src->wStep, dst->data, dst->wStep, ippimage_ippisize(src), kernelSize, sigma, ippBorderRepl, 0, buffer);

error:
  free(buffer);
  return status;
}*/


// -------------------------------------------------------------------------- //
// image_error_message
// -------------------------------------------------------------------------- //
const char* image_error_message(int status) {
  return ippGetStatusString(status);
}

