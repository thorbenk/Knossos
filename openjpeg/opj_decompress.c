/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2006-2007, Parvatha Elangovan
 * Copyright (c) 2008;2011-2012, Centre National d'Etudes Spatiales (CNES), France
 * Copyright (c) 2012, CS Systemes d'Information, France
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "opj_apps_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif /* _WIN32 */

#include "openjpeg.h"
#include "opj_getopt.h"
#include "convert.h"
#include "index.h"

#ifdef OPJ_HAVE_LIBLCMS2
#include <lcms2.h>
#endif
#ifdef OPJ_HAVE_LIBLCMS1
#include <lcms.h>
#endif
#include "color.h"

#include "format_defs.h"

typedef struct dircnt{
	/** Buffer for holding images read from Directory*/
	char *filename_buf;
	/** Pointer to the buffer*/
	char **filename;
}dircnt_t;


typedef struct img_folder{
	/** The directory path of the folder containing input images*/
	char *imgdirpath;
	/** Output format*/
	const char *out_format;
	/** Enable option*/
	char set_imgdir;
	/** Enable Cod Format for output*/
	char set_out_format;

}img_fol_t;

/* -------------------------------------------------------------------------- */
/* Declarations                                                               */
int get_file_format(const char *filename);
char get_next_file(int imageno,dircnt_t *dirptr,img_fol_t *img_fol, opj_dparameters_t *parameters);
static int infile_format(const char *fname);

int parse_cmdline_decoder(opj_dparameters_t *parameters,img_fol_t *img_fol, char *infile);
int parse_DA_values( char* inArg, unsigned int *DA_x0, unsigned int *DA_y0, unsigned int *DA_x1, unsigned int *DA_y1);

/* -------------------------------------------------------------------------- */
static void decode_help_display(void) {
	fprintf(stdout,"HELP for opj_decompress\n----\n\n");
	fprintf(stdout,"- the -h option displays this help information on screen\n\n");

/* UniPG>> */
	fprintf(stdout,"List of parameters for the JPEG 2000 "
#ifdef USE_JPWL
		"+ JPWL "
#endif /* USE_JPWL */
		"decoder:\n");
/* <<UniPG */
	fprintf(stdout,"\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"  -ImgDir \n");
	fprintf(stdout,"	Image file Directory path \n");
	fprintf(stdout,"  -OutFor \n");
	fprintf(stdout,"    REQUIRED only if -ImgDir is used\n");
	fprintf(stdout,"	  Need to specify only format without filename <BMP>  \n");
	fprintf(stdout,"    Currently accepts PGM, PPM, PNM, PGX, PNG, BMP, TIF, RAW and TGA formats\n");
	fprintf(stdout,"  -i <compressed file>\n");
	fprintf(stdout,"    REQUIRED only if an Input image directory not specified\n");
	fprintf(stdout,"    Currently accepts J2K-files, JP2-files and JPT-files. The file type\n");
	fprintf(stdout,"    is identified based on its suffix.\n");
	fprintf(stdout,"  -o <decompressed file>\n");
	fprintf(stdout,"    REQUIRED\n");
	fprintf(stdout,"    Currently accepts PGM, PPM, PNM, PGX, PNG, BMP, TIF, RAW and TGA files\n");
	fprintf(stdout,"    Binary data is written to the file (not ascii). If a PGX\n");
	fprintf(stdout,"    filename is given, there will be as many output files as there are\n");
	fprintf(stdout,"    components: an indice starting from 0 will then be appended to the\n");
	fprintf(stdout,"    output filename, just before the \"pgx\" extension. If a PGM filename\n");
	fprintf(stdout,"    is given and there are more than one component, only the first component\n");
	fprintf(stdout,"    will be written to the file.\n");
	fprintf(stdout,"  -r <reduce factor>\n");
	fprintf(stdout,"    Set the number of highest resolution levels to be discarded. The\n");
	fprintf(stdout,"    image resolution is effectively divided by 2 to the power of the\n");
	fprintf(stdout,"    number of discarded levels. The reduce factor is limited by the\n");
	fprintf(stdout,"    smallest total number of decomposition levels among tiles.\n");
	fprintf(stdout,"  -l <number of quality layers to decode>\n");
	fprintf(stdout,"    Set the maximum number of quality layers to decode. If there are\n");
	fprintf(stdout,"    less quality layers than the specified number, all the quality layers\n");
	fprintf(stdout,"    are decoded.\n");
	fprintf(stdout,"  -x  \n");
	fprintf(stdout,"    Create an index file *.Idx (-x index_name.Idx) \n");
	fprintf(stdout,"  -d <x0,y0,x1,y1>\n");
	fprintf(stdout,"    OPTIONAL\n");
	fprintf(stdout,"    Decoding area\n");
	fprintf(stdout,"    By default all the image is decoded.\n");
	fprintf(stdout,"  -t <tile_number>\n");
	fprintf(stdout,"    OPTIONAL\n");
	fprintf(stdout,"    Set the tile number of the decoded tile. Follow the JPEG2000 convention from left-up to bottom-up\n");
	fprintf(stdout,"    By default all tiles are decoded.\n");
	fprintf(stdout,"\n");
/* UniPG>> */
#ifdef USE_JPWL
	fprintf(stdout,"  -W <options>\n");
	fprintf(stdout,"    Activates the JPWL correction capability, if the codestream complies.\n");
	fprintf(stdout,"    Options can be a comma separated list of <param=val> tokens:\n");
	fprintf(stdout,"    c, c=numcomps\n");
	fprintf(stdout,"       numcomps is the number of expected components in the codestream\n");
	fprintf(stdout,"       (search of first EPB rely upon this, default is %d)\n", JPWL_EXPECTED_COMPONENTS);
#endif /* USE_JPWL */
/* <<UniPG */
	fprintf(stdout,"\n");
}

/* -------------------------------------------------------------------------- */

int get_file_format(const char *filename) {
	unsigned int i;
	static const char *extension[] = {"pgx", "pnm", "pgm", "ppm", "bmp","tif", "raw", "rawl", "tga", "png", "j2k", "jp2", "jpt", "j2c", "jpc" };
	static const int format[] = { PGX_DFMT, PXM_DFMT, PXM_DFMT, PXM_DFMT, BMP_DFMT, TIF_DFMT, RAW_DFMT, RAWL_DFMT, TGA_DFMT, PNG_DFMT, J2K_CFMT, JP2_CFMT, JPT_CFMT, J2K_CFMT, J2K_CFMT };
	char * ext = strrchr(filename, '.');
	if (ext == NULL)
		return -1;
	ext++;
	if(ext) {
		for(i = 0; i < sizeof(format)/sizeof(*format); i++) {
			if(strcasecmp(ext, extension[i]) == 0) {
				return format[i];
			}
		}
	}

	return -1;
}

/* -------------------------------------------------------------------------- */
char get_next_file(int imageno,dircnt_t *dirptr,img_fol_t *img_fol, opj_dparameters_t *parameters){
	char image_filename[OPJ_PATH_LEN], infilename[OPJ_PATH_LEN],outfilename[OPJ_PATH_LEN],temp_ofname[OPJ_PATH_LEN];
	char *temp_p, temp1[OPJ_PATH_LEN]="";

	strcpy(image_filename,dirptr->filename[imageno]);
	fprintf(stderr,"File Number %d \"%s\"\n",imageno,image_filename);
	parameters->decod_format = infile_format(image_filename);
	if (parameters->decod_format == -1)
		return 1;
	sprintf(infilename,"%s/%s",img_fol->imgdirpath,image_filename);
	strncpy(parameters->infile, infilename, sizeof(infilename));

	/*Set output file*/
	strcpy(temp_ofname,strtok(image_filename,"."));
	while((temp_p = strtok(NULL,".")) != NULL){
		strcat(temp_ofname,temp1);
		sprintf(temp1,".%s",temp_p);
	}
	if(img_fol->set_out_format==1){
		sprintf(outfilename,"%s/%s.%s",img_fol->imgdirpath,temp_ofname,img_fol->out_format);
		strncpy(parameters->outfile, outfilename, sizeof(outfilename));
	}
	return 0;
}

/* -------------------------------------------------------------------------- */
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
/* position 45: "\xff\x52" */
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

static int infile_format(const char *fname)
{
    FILE *reader;
	const char *s, *magic_s;
	int ext_format, magic_format;
	unsigned char buf[12];
	unsigned int l_nb_read;
	int i;

	reader = fopen(fname, "rb");

	if (reader == NULL) {
        fprintf(stderr, "File open error!\n");
		return -2;
	}

	memset(buf, 0, 12);
	l_nb_read = fread(buf, 1, 12, reader);
	fclose(reader);
	if (l_nb_read != 12) {
        fprintf(stderr, "File read error, only %d bytes read!\n", l_nb_read);
        return -1;
	}

	ext_format = get_file_format(fname);

	if (ext_format == JPT_CFMT)
		return JPT_CFMT;

	if (memcmp(buf, JP2_RFC3745_MAGIC, 12) == 0 || memcmp(buf, JP2_MAGIC, 4) == 0) {
		magic_format = JP2_CFMT;
		magic_s = ".jp2";
	}
	else if (memcmp(buf, J2K_CODESTREAM_MAGIC, 4) == 0) {
		magic_format = J2K_CFMT;
		magic_s = ".j2k or .jpc or .j2c";
	}
	else
    {
        // TODO GetCurrentThreadId() and GetTickCount() prevent compilation on linux (and possibly other platforms?)
        //fprintf(stderr, "DEBUG DEBUG DEBUG infile_format: file magic error %s, TID %d @ %d\n", fname, GetCurrentThreadId(), GetTickCount());
        for (i = 0; i < l_nb_read; i++) {
            fprintf(stderr, "%02X", buf[i]);
        }
        fprintf(stderr, "\n");
		return -1;
    }

	if (magic_format == ext_format)
		return ext_format;

	s = fname + strlen(fname) - 4;

	fputs("\n===========================================\n", stderr);
	fprintf(stderr, "The extension of this file is incorrect.\n"
					"FOUND %s. SHOULD BE %s\n", s, magic_s);
	fputs("===========================================\n", stderr);

	return magic_format;
}

/* -------------------------------------------------------------------------- */
/**
 * Parse the command line
 */
/* -------------------------------------------------------------------------- */
int parse_cmdline_decoder(opj_dparameters_t *parameters,img_fol_t *img_fol, char *infile) {
	img_fol->set_out_format = 0;
    parameters->decod_format = infile_format(infile);
    strncpy(parameters->infile, infile, sizeof(parameters->infile)-1);
    char *outfile = "file.raw";
    parameters->cod_format = get_file_format(outfile);
    strncpy(parameters->outfile, outfile, sizeof(parameters->outfile)-1);

	return 0;
}

/* -------------------------------------------------------------------------- */
/**
 * Parse decoding area input values
 * separator = ","
 */
/* -------------------------------------------------------------------------- */
int parse_DA_values( char* inArg, unsigned int *DA_x0, unsigned int *DA_y0, unsigned int *DA_x1, unsigned int *DA_y1)
{
	int it = 0;
	int values[4];
	char delims[] = ",";
	char *result = NULL;
	result = strtok( inArg, delims );

	while( (result != NULL) && (it < 4 ) ) {
		values[it] = atoi(result);
		result = strtok( NULL, delims );
		it++;
	}

	if (it != 4) {
		return EXIT_FAILURE;
	}
	else{
		*DA_x0 = values[0]; *DA_y0 = values[1];
		*DA_x1 = values[2]; *DA_y1 = values[3];
		return EXIT_SUCCESS;
	}
}

/* -------------------------------------------------------------------------- */

/**
sample error callback expecting a FILE* client object
*/

static void final_printout(const char *msg, const char *prefix) {
    /*
    fprintf(stdout, "%s %s", msg, prefix);
    */
}
static void error_callback(const char *msg, void *client_data) {
	(void)client_data;
	final_printout(msg, "[ERROR]");
}
/**
sample warning callback expecting a FILE* client object
*/
static void warning_callback(const char *msg, void *client_data) {
	(void)client_data;
	final_printout(msg, "[WARNING]");
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *client_data) {
	(void)client_data;
	final_printout(msg, "[INFO]");
}

/* -------------------------------------------------------------------------- */
/**
 * OPJ_DECOMPRESS MAIN
 */
/* -------------------------------------------------------------------------- */
int jp2_decompress_main(char *infile, char *buf, int bufsize)
{
    int i;
    int *dataptr;

	opj_dparameters_t parameters;			/* decompression parameters */
	opj_image_t* image = NULL;
	opj_stream_t *l_stream = NULL;				/* Stream */
	opj_codec_t* l_codec = NULL;				/* Handle to a decompressor */
	opj_codestream_index_t* cstr_index = NULL;

	char indexfilename[OPJ_PATH_LEN];	/* index file name */

	OPJ_INT32 num_images, imageno;
	img_fol_t img_fol;
	dircnt_t *dirptr = NULL;

	/* set decoding parameters to default values */
	opj_set_default_decoder_parameters(&parameters);

	/* FIXME Initialize indexfilename and img_fol */
	*indexfilename = 0;

	/* Initialize img_fol */
	memset(&img_fol,0,sizeof(img_fol_t));

	/* parse input and get user encoding parameters */
	if (0 != parse_cmdline_decoder(&parameters,&img_fol, infile)) {
        return EXIT_FAILURE;
	}

    /* read the input file and put it in memory */
    /* ---------------------------------------- */

    l_stream = opj_stream_create_default_file_stream_v3(parameters.infile,1);
    if (!l_stream){
        return EXIT_FAILURE;
    }

    /* decode the JPEG2000 stream */
    /* ---------------------- */

    switch(parameters.decod_format) {
        case J2K_CFMT:	/* JPEG-2000 codestream */
        {
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_J2K);
            break;
        }
        case JP2_CFMT:	/* JPEG 2000 compressed image data */
        {
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_JP2);
            break;
        }
        case JPT_CFMT:	/* JPEG 2000, JPIP */
        {
            /* Get a decoder handle */
            l_codec = opj_create_decompress(OPJ_CODEC_JPT);
            break;
        }
        default:
            fprintf(stderr, "skipping file..\n");
            opj_stream_destroy_v3(l_stream);
            return EXIT_FAILURE;
    }

    /* catch events using our callbacks and give a local context */
    opj_set_info_handler(l_codec, info_callback,00);
    opj_set_warning_handler(l_codec, warning_callback,00);
    opj_set_error_handler(l_codec, error_callback,00);

    /* Setup the decoder decoding parameters using user parameters */
    if ( !opj_setup_decoder(l_codec, &parameters) ){
        fprintf(stderr, "ERROR -> j2k_dump: failed to setup the decoder\n");
        opj_stream_destroy_v3(l_stream);
        opj_destroy_codec(l_codec);
        return EXIT_FAILURE;
    }


    /* Read the main header of the codestream and if necessary the JP2 boxes*/
    if(! opj_read_header(l_stream, l_codec, &image)){
        fprintf(stderr, "ERROR -> opj_decompress: failed to read the header\n");
        opj_stream_destroy_v3(l_stream);
        opj_destroy_codec(l_codec);
        opj_image_destroy(image);
        return EXIT_FAILURE;
    }

    if (!parameters.nb_tile_to_decode) {
        /* Optional if you want decode the entire image */
        if (!opj_set_decode_area(l_codec, image, parameters.DA_x0,
                parameters.DA_y0, parameters.DA_x1, parameters.DA_y1)){
            fprintf(stderr,	"ERROR -> opj_decompress: failed to set the decoded area\n");
            opj_stream_destroy_v3(l_stream);
            opj_destroy_codec(l_codec);
            opj_image_destroy(image);
            return EXIT_FAILURE;
        }

        /* Get the decoded image */
        if (!(opj_decode(l_codec, l_stream, image) && opj_end_decompress(l_codec,	l_stream))) {
            fprintf(stderr,"ERROR -> opj_decompress: failed to decode image!\n");
            opj_destroy_codec(l_codec);
            opj_stream_destroy_v3(l_stream);
            opj_image_destroy(image);
            return EXIT_FAILURE;
        }
    }
    else {

        /* It is just here to illustrate how to use the resolution after set parameters */
        /*if (!opj_set_decoded_resolution_factor(l_codec, 5)) {
            fprintf(stderr, "ERROR -> opj_decompress: failed to set the resolution factor tile!\n");
            opj_destroy_codec(l_codec);
            opj_stream_destroy_v3(l_stream);
            opj_image_destroy(image);
            return EXIT_FAILURE;
        }*/

        if (!opj_get_decoded_tile(l_codec, l_stream, image, parameters.tile_index)) {
            fprintf(stderr, "ERROR -> opj_decompress: failed to decode tile!\n");
            opj_destroy_codec(l_codec);
            opj_stream_destroy_v3(l_stream);
            opj_image_destroy(image);
            return EXIT_FAILURE;
        }
        fprintf(stdout, "tile %d is decoded!\n\n", parameters.tile_index);
    }

    /* Close the byte stream */
    opj_stream_destroy_v3(l_stream);

    if(image->color_space == OPJ_CLRSPC_SYCC){
        color_sycc_to_rgb(image); /* FIXME */
    }

    if( image->color_space != OPJ_CLRSPC_SYCC
        && image->numcomps == 3 && image->comps[0].dx == image->comps[0].dy
        && image->comps[1].dx != 1 )
        image->color_space = OPJ_CLRSPC_SYCC;
    else if (image->numcomps <= 2)
        image->color_space = OPJ_CLRSPC_GRAY;

    if(image->icc_profile_buf) {
#if defined(OPJ_HAVE_LIBLCMS1) || defined(OPJ_HAVE_LIBLCMS2)
        color_apply_icc_profile(image); /* FIXME */
#endif
        free(image->icc_profile_buf);
        image->icc_profile_buf = NULL; image->icc_profile_len = 0;
    }

    /* create output image */
    /* ------------------- */
    switch (parameters.cod_format) {
    case PXM_DFMT:			/* PNM PGM PPM */
        if (imagetopnm(image, parameters.outfile)) {
            fprintf(stdout,"Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Generated Outfile %s\n",parameters.outfile);
        }
        break;

    case PGX_DFMT:			/* PGX */
        if(imagetopgx(image, parameters.outfile)){
            fprintf(stdout,"Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Generated Outfile %s\n",parameters.outfile);
        }
        break;

    case BMP_DFMT:			/* BMP */
        if(imagetobmp(image, parameters.outfile)){
            fprintf(stdout,"Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Generated Outfile %s\n",parameters.outfile);
        }
        break;
#ifdef OPJ_HAVE_LIBTIFF
    case TIF_DFMT:			/* TIFF */
        if(imagetotif(image, parameters.outfile)){
            fprintf(stdout,"Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Generated Outfile %s\n",parameters.outfile);
        }
        break;
#endif /* OPJ_HAVE_LIBTIFF */
    case RAW_DFMT:			/* RAW */
        dataptr = &image->comps[0].data[0];
        char mask = 0xFF;
        for (i = 0; i < bufsize; i++) {
            *buf++ = *dataptr++ & mask;
        }
        /*
        if(imagetoraw(image, parameters.outfile)){
            fprintf(stdout,"Error generating raw file. Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Successfully generated Outfile %s\n",parameters.outfile);
        }
        */
        break;

    case RAWL_DFMT:			/* RAWL */
        if(imagetorawl(image, parameters.outfile)){
            fprintf(stdout,"Error generating rawl file. Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Successfully generated Outfile %s\n",parameters.outfile);
        }
        break;

    case TGA_DFMT:			/* TGA */
        if(imagetotga(image, parameters.outfile)){
            fprintf(stdout,"Error generating tga file. Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Successfully generated Outfile %s\n",parameters.outfile);
        }
        break;
#ifdef OPJ_HAVE_LIBPNG
    case PNG_DFMT:			/* PNG */
        if(imagetopng(image, parameters.outfile)){
            fprintf(stdout,"Error generating png file. Outfile %s not generated\n",parameters.outfile);
        }
        else {
            fprintf(stdout,"Successfully generated Outfile %s\n",parameters.outfile);
        }
        break;
#endif /* OPJ_HAVE_LIBPNG */
/* Can happen if output file is TIFF or PNG
* and OPJ_HAVE_LIBTIF or OPJ_HAVE_LIBPNG is undefined
*/
        default:
            fprintf(stderr,"Outfile %s not generated\n",parameters.outfile);
    }

    /* free remaining structures */
    if (l_codec) {
        opj_destroy_codec(l_codec);
    }


    /* free image data structure */
    opj_image_destroy(image);

    /* destroy the codestream index */
    opj_destroy_cstr_index(&cstr_index);

	return EXIT_SUCCESS;
}
/*end main*/




