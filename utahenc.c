/* Written by Chad Miller for CS 3505, March 2013.
   Implements an encoder for a 'utah' image format.
*/

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "utah.h"
#include "internal.h"


/*Initialize the encoding context*/
static av_cold int utah_encode_init(AVCodecContext *avctx){
    UtahContext *s = avctx->priv_data;
    avcodec_get_frame_defaults(&s->picture);
    avctx->coded_frame = &s->picture;
    return 0;
}

/*Encode a frame*/
static int utah_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    /*Get the image contexts.*/
    UtahContext *s = avctx->priv_data;
    AVFrame * const p = &s->picture;
    int image, row, bytes, hsize;
    int height = avctx->height;
    int width = avctx->width;
    int space;
    uint8_t *pixel, *dest;
    uint8_t *zero;
    uint8_t *nothing;
    *p = *pict;
    p->pict_type= AV_PICTURE_TYPE_I;
    p->key_frame= 1;

    /*Set size of row, space and image*/
    row = (width * 8 + 7LL) >> 3LL;
    space = (4 - row) & 3;
    image = height * (row + space);

    /*Set size of header*/
    hsize = 54;
    bytes = image + hsize;
    
    /*Allocate space*/
    ff_alloc_packet2(avctx, pkt, bytes);
    dest = pkt->data;
    
    /*Write to byte stream*/
    bytestream_put_byte(&dest, 'U');                   // UTAH header
    bytestream_put_byte(&dest, 'T');                   // ...
    bytestream_put_le32(&dest, bytes);                 // Total bytes
    bytestream_put_le32(&dest, hsize);                 // Header size
    bytestream_put_le32(&dest, width);                 // Image width
    bytestream_put_le32(&dest, height);                // Image height
    pixel = p->data[0] + (height - 1) * p->linesize[0];
    dest = pkt->data + hsize;

	nothing = 00000000;
	zero = nothing + (height - 1) * p->linesize[0];

    /*Copy image pixel data to dest*/
    for(int i = 1; i < height; i++) {
        memcpy(dest, pixel, row);
        dest += row;
        memset(dest, 0, space);
        dest += space;
        pixel -= p->linesize[0]; 
    }
    pkt->flags |= AV_PKT_FLAG_KEY;

    /*Got the packet!*/
    *got_packet = 1;
    return 0;
}

/*A 'utah' encoder struct */
AVCodec ff_utah_encoder = {
    .name           = "utah",
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_UTAH,
    .priv_data_size = sizeof(UtahContext),
    .init           = utah_encode_init,
    .encode2        = utah_encode_frame,
   .pix_fmts = (const enum AVPixelFormat[])  { 
    AV_PIX_FMT_RGB8, AV_PIX_FMT_NONE 
    },
    .long_name      = NULL_IF_CONFIG_SMALL("UTAH (Windows and OS/2 bitmap)"),
};
