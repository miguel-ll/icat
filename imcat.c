#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int termw=0, termh=0;
static int blend=0;
static unsigned char termbg[3] = { 0,0,0 };

static void get_terminal_size(void) {
	FILE* f = popen( "stty size", "r" );
	if ( !f )
	{
		fprintf( stderr, "Failed to determine terminal size using stty.\n" );
		exit( 1 );
	}
	const int num = fscanf( f, "%d %d", &termh, &termw );
	assert( num == 2 );
	pclose( f );
}

#define RESETALL  "\x1b[0m"

#	define HALFBLOCK "▀"		// Uses Unicode char U+2580
// note: image has alpha pre-multied. Mimic GL_ONE + GL_ONE_MINUS_SRC_ALPHA
#define BLEND \
{ \
	const int t0 = 255; \
	const int t1 = 255-a; \
	r = ( r * t0 + termbg[0] * t1 ) / 255; \
	g = ( g * t0 + termbg[1] * t1 ) / 255; \
	b = ( b * t0 + termbg[2] * t1 ) / 255; \
}

static void print_image_double_res( int w, int h, unsigned char* data) {
	if ( h & 1 )
		h--;
	const int linesz = 32768;
	char line[ linesz ];


	for ( int y=0; y<h; y+=2 ) {
		const unsigned char* row0 = data + (y+0) * w * 4;
		const unsigned char* row1 = data + (y+1) * w * 4;
		line[0] = 0;
		for ( int x=0; x<w; ++x ) {
			// foreground colour.
			strncat( line, "\x1b[38;2;", sizeof(line) - strlen(line) - 1 );
			char tripl[80];
			unsigned char r = *row0++;
			unsigned char g = *row0++;
			unsigned char b = *row0++;
			unsigned char a = *row0++;
			if ( blend )
				BLEND
			snprintf( tripl, sizeof(tripl), "%d;%d;%dm", r,g,b );
			strncat( line, tripl, sizeof(line) - strlen(line) - 1 );
			// background colour.
                        strncat( line, "\x1b[48;2;", sizeof(line) - strlen(line) - 1 );
			r = *row1++;
			g = *row1++;
			b = *row1++;
			a = *row1++;
			if ( blend )
				BLEND
			snprintf( tripl, sizeof(tripl), "%d;%d;%dm" HALFBLOCK, r,g,b );
			strncat( line, tripl, sizeof(line) - strlen(line) - 1 );
		}
		strncat( line, RESETALL, sizeof(line) - strlen(line) - 1 );
		puts( line );
	}
}


static int process_image(const char* nm) {
	int imw=0,imh=0,n=0;
	unsigned char *data = stbi_load( nm, &imw, &imh, &n, 4 );
	if ( !data )
		return -1;
	//fprintf( stderr, "%s has dimension %dx%d w %d components.\n", nm, imw, imh, n );
	float aspectratio = imw / (float) imh;
	float pixels_per_char = imw / (float)termw;
	if ( pixels_per_char < 1 ) pixels_per_char = 1;
	int kernelsize = (int) floorf( pixels_per_char );
	if ( (kernelsize&1) == 0 ) kernelsize--;
	if ( !kernelsize ) kernelsize=1;
	const int kernelradius = (kernelsize-1)/2;

	const int outw = imw < termw ? imw : termw;
	const int outh = (int) roundf( outw / aspectratio );

	unsigned char out[ outh ][ outw ][ 4 ];
	for (int y=0; y<outh; ++y)
		for ( int x=0; x<outw; ++x ) {
			const int cx = (int) roundf( pixels_per_char * x );
			const int cy = (int) roundf( pixels_per_char * y );
			int acc[4] = {0,0,0,0};
			int numsamples=0;
			int sy = cy-kernelradius;
			sy = sy < 0 ? 0 : sy;
			int ey = cy+kernelradius;
			ey = ey >= imh ? imh-1 : ey;
			int sx = cx-kernelradius;
			sx = sx < 0 ? 0 : sx;
			int ex = cx+kernelradius;
			ex = ex >= imw ? imw-1 : ex;
			for (int yy = sy; yy <= ey; ++yy)
				for (int xx = sx; xx <= ex; ++xx) {
					unsigned char* reader = data + ( yy * imw * 4 ) + xx * 4;
					const int a = reader[3];
					acc[ 0 ] += a * reader[0] / 255;
					acc[ 1 ] += a * reader[1] / 255;
					acc[ 2 ] += a * reader[2] / 255;
					acc[ 3 ] += reader[3];
					numsamples++;
				}
			out[ y ][ x ][ 0 ] = acc[ 0 ] / numsamples;
			out[ y ][ x ][ 1 ] = acc[ 1 ] / numsamples;
			out[ y ][ x ][ 2 ] = acc[ 2 ] / numsamples;
			out[ y ][ x ][ 3 ] = acc[ 3 ] / numsamples;
		}
	stbi_image_free( data );
	data = 0;

	print_image_double_res( outw, outh, (unsigned char*) out );
	return 0;
}


int main(int argc, char* argv[]) {
	if ( argc == 1 || !strcmp(argv[1], "--help") ) {
		fprintf( stderr, "Default usage: %s image [image2 .. imageN]\nReducing the imgsize example: %s image 50\n", argv[0], argv[0]);
		exit(0);
	}

	// Parse environment variable for terminal background colour.
	const char* imcatbg = getenv( "IMCATBG" );
	if (imcatbg) {
		const int bg = strtol( imcatbg+1, 0, 16 );
		termbg[ 2 ] = ( bg >>  0 ) & 0xff;
		termbg[ 1 ] = ( bg >>  8 ) & 0xff;
		termbg[ 0 ] = ( bg >> 16 ) & 0xff;
		blend = 1;
	}
	// Step 1: figure out the width and height of terminal.
	get_terminal_size();
	//fprintf( stderr, "Your terminal is size %dx%d\n", termw, termh );
        if (argc == 3 ) {
	    if (strtol(argv[2], (char**)NULL, 10) > 0) {
            int x = strtol(argv[2], (char**)NULL, 10);
	    termh -= x; termw -= x;
	}
	}
	// Step 2: Process all images on the command line.
	for (int i=1; i<argc; ++i) {
		const char* nm = argv[i];
		int rv = process_image(nm);
		if (argc == 3) return 0;
		if (rv < 0) fprintf( stderr, "Could not load image %s\n", nm );
	}
	return 0;
}

