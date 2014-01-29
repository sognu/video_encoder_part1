/* Written by Chad Miller for CS 3505, March 2013.
   Implements a decoder for a 'utah' image format.
*/

#include "avcodec.h"
#include "bytestream.h"
#include "utah.h"
#include "internal.h"


/*Initialize the decoding context*/
static av_cold int utah_decode_init(AVCodecContext *avctx)
{
    UtahContext *s = avctx->priv_data;
    avcodec_get_frame_defaults(&s->picture);
    avctx->coded_frame = &s->picture;

    return 0;
}

/*Decode a frame*/
static int utah_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    const uint8_t *src = avpkt->data;
    const uint8_t *c = src;
    int size       = avpkt->size;
    UtahContext *s      = avctx->priv_data;
    AVFrame *picture   = data;
    AVFrame *p         = &s->picture;
    int fsize, hsize;
    int width, height;
    int num, linesize;
    uint8_t *pixel;

   
   /* 'U' --> 'T' */
   bytestream_get_byte(&src); 
   bytestream_get_byte(&src);
 
   /* Total bytes */
   fsize = bytestream_get_le32(&src); 
   
   /* Header size */
   hsize  = bytestream_get_le32(&src); 
 
   /*Get image width and height*/
   width  = bytestream_get_le32(&src);
   height = bytestream_get_le32(&src);
      
   /*Set image width and height*/
   avctx->width  = width;
   avctx->height = height > 0 ? height : -height;

   /*Set image format*/
   avctx->pix_fmt = AV_PIX_FMT_RGB8;

   /*W/O results in a seg fault*/
   ff_get_buffer(avctx, p);

   /*Total size*/
   src = c + hsize;
  
   /* Number of pixels in a line */
   num = ((avctx->width * 8 + 31) / 8) & ~3;

   /*If image is upside-down, flip it around*/
   if (height > 0) {
        pixel      = p->data[0] + (avctx->height - 1) * p->linesize[0];
        linesize = -p->linesize[0];
    } else {
        pixel      = p->data[0];
        linesize = p->linesize[0];
    }

   /*Copy src pixel data to dest*/
   for (int i = 0; i < avctx->height; i++) {
    memcpy(pixel, src, num);
    src += num;
    pixel += linesize;
   }
    
    /*Got the frame!*/
    *picture = s->picture;
    *got_frame = 1;

    return size;
}

/*Ends the decoding context*/
static av_cold int utah_decode_end(AVCodecContext *avctx)
{
    UtahContext* c = avctx->priv_data;

    if (c->picture.data[0])
        avctx->release_buffer(avctx, &c->picture);

    return 0;
}

/* A 'utah' decoder struct */
AVCodec ff_utah_decoder = {
    .name           = "utah",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_UTAH,
    .priv_data_size = sizeof(UtahContext),
    .init           = utah_decode_init,
    .close          = utah_decode_end,
    .decode         = utah_decode_frame,
    .capabilities   = CODEC_CAP_DR1,
    .long_name      = NULL_IF_CONFIG_SMALL(" (Windows and OS/2 bitmap)"),
};
