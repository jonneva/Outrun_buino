#include "TVout.h"
#include "wrapper.h"

// Bitmap overlay functions

void overlaybitmap(int16_t x, int16_t y, const unsigned char * bmp,
				   uint16_t i, uint8_t width, uint8_t lines) {
	uint8_t temp, lshift, rshift, save, xtra;
	uint16_t si = 0;

	rshift = x&7;
	lshift = 8-rshift;
	if (width == 0) {
	  //width = pgm_read_byte((uint32_t)(bmp) + i);
	  width = bmp[i];
	  i++;
	}
	if (lines == 0) {
	  //lines = pgm_read_byte((uint32_t)(bmp) + i);
	  lines = bmp[i];
	  i++;
	}

	if (width&7) {
	  xtra = width&7;
	  width = width/8;
	  width++;
	}
	else {
	  xtra = 8;
	  width = width/8;
	}

	for (uint8_t l = 0; l < lines; l++) {
	  if (y+l >= 0)  {// added this to allow clipped sky
	  si = ((y + l) % display.vres)*display.hres + x/8;
	  //si = (y + l)*display.hres + x/8;
	  if (width == 1)
	    temp = 0xff >> rshift + xtra;
	  else
	    temp = 0;
	  save = display.screen[si];
	  //temp = pgm_read_byte((uint32_t)(bmp) + i++);
	  temp = bmp[i++];
	  display.screen[si++] |= temp >> rshift;
	  for ( uint16_t b = i + width-1; i < b; i++) {
	    if (si % display.hres == 0) {
	      // wrapped around to the left side
	      si -= display.hres;
	    }
	    save = display.screen[si];
	    display.screen[si] |= temp << lshift;
	    //temp = pgm_read_byte((uint32_t)(bmp) + i);
	    temp = bmp[i];
	    display.screen[si++] |= temp >> rshift;
	  }
	  if (si % display.hres == 0) {
	    // wrapped around to the left side
	    si -= display.hres; // added *2 to make wrap on same row
	  }
	  if (rshift + xtra < 8)
	    display.screen[si-1] |= (save & (0xff >> rshift + xtra));	//test me!!!
	  display.screen[si] |= temp << lshift;
	  } else {i+=width; }// increment bitmap pointe
	}
} // end of bitmap

void erasebitmap(uint8_t x, uint8_t y, const unsigned char * bmp,
				   uint16_t i, uint8_t width, uint8_t lines) {

	uint8_t temp, lshift, rshift, save, xtra;
	uint16_t si = 0;

	rshift = x&7;
	lshift = 8-rshift;
	if (width == 0) {
		//width = pgm_read_byte((uint32_t)(bmp) + i);
		width = bmp[i];
		i++;
	}
	if (lines == 0) {
		//lines = pgm_read_byte((uint32_t)(bmp) + i);
		lines = bmp[i];
		i++;
	}

	if (width&7) {
		xtra = width&7;
		width = width/8;
		width++;
	}
	else {
		xtra = 8;
		width = width/8;
	}

	for (uint8_t l = 0; l < lines; l++) {
	  si = ((y + l) % display.vres)*display.hres + x/8;
	  //si = (y + l)*display.hres + x/8;
		if (width == 1)
			temp = 0xff >> rshift + xtra;
		else
			temp = 0;
		save = display.screen[si];
		//temp = pgm_read_byte((uint32_t)(bmp) + i++);
		temp = bmp[i++];
		display.screen[si++] &= ~(temp >> rshift);
		for ( uint16_t b = i + width-1; i < b; i++) {
		  if (si % display.hres == 0) {
		    // wrapped around to the left side
		    si -= display.hres;
		  }
			save = display.screen[si];
			display.screen[si] &= ~(temp << lshift);
			//temp = pgm_read_byte((uint32_t)(bmp) + i);
			temp = bmp[i];
			display.screen[si++] &= ~(temp >> rshift);
		}
		if (si % display.hres == 0) {
		  // wrapped around to the left side
		  si -= display.hres;
		}

		if (rshift + xtra < 8)
			display.screen[si-1] &= ~(save & (0xff >> rshift + xtra));	//test me!!!
		if (rshift + xtra - 8 > 0)
		  display.screen[si] &= ~(temp << lshift);
	}
} // end of bitmap

void alphabitmap(int8_t x, int8_t y, const unsigned char * bmp, const unsigned char * alpha) {

    // NEEDS AT LEAST 1 byte wide bitmap !!!! Alpha needs to be same size !
	uint8_t temp, lshift=0, rshift=0, save, xtra, width, lines;
	uint8_t atemp, maskedbyte, onbits, offbits;
	uint16_t i=0, si = 0;

    if (x>=0) {
        // NORMAL CASE, NOT CROPPED
        rshift = x&7;      // rshift = bits OVER even bytes, eg. 14 = 110
        lshift = 8-rshift; // lshift = bits UNTIL next even byte, eg. 2 = 10
        width = bmp[i]; i++; //width = pgm_read_byte((uint32_t)(bmp) + i);
        lines = bmp[i]; i++;//lines = pgm_read_byte((uint32_t)(bmp) + i);
        if (width&7) {      // if width is not divisible by 8
            xtra = width&7;   // xtra is amount of xtra bits
            width = width/8;  // width is width in even bytes
            width++;          // plus one uneven one ...
        } else {              // width IS divisible by 8
            xtra = 8;         // xtra = 1000b
            width = width/8;  // width = even num of bytes
        }
        // START Y LOOP
        for (uint8_t l = 0; l < lines; l++) {
            si = ((y + l) % display.vres)*display.hres + x/8; //index to screen
            save = display.screen[si]; // Save is the screen byte
            atemp = alpha[i]; // read first byte of alpha data
            temp = bmp[i]; i++; // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data
            onbits = temp & atemp; // get bits that should be turned on
            offbits = ~temp & atemp; // get bits that should be turned off
            temp = (onbits >> rshift ) | save; // add onbits
            temp = temp & (~(offbits >> rshift)); // delete offbits
            display.screen[si] = temp; si++; //first byte to screen buffer, increment
            // START X LOOP
            for ( uint16_t b = i + width-1; i < b; i++) {
                if (si % display.hres == 0) {
                    //si -= display.hres; // wrap disabled
                } else {
                    // draw the leftovers of a previous shifted byte first
                save = display.screen[si]; // get byte at si
                temp = (onbits << lshift ) | save; // add onbits
                temp = temp & (~(offbits << lshift)); // delete offbits
                }
                save = temp; // waiting for ls bits of the next bmp byte
                // add the leftside bits of a new image byte
                atemp = alpha[i]; // read byte of alpha data
                temp = bmp[i];
                onbits = temp & atemp; // get bits that should be turned on
                offbits = ~temp & atemp; // get bits that should be turned off

                temp = (onbits >> rshift) | save; // add onbits
                temp = temp & (~(offbits >> rshift)); // delete offbits
                display.screen[si] = temp; // was temp
                si++; // increment screen index to next screen byte
            }
            // END OF X LOOP
            if (si % display.hres == 0) { // now drawing last byte of row, check for edge
                //si -= display.hres; // I need to disable this
            } else {
                save = display.screen[si];  // get next screen byte
                temp = (onbits << lshift ) | save; // add leftover onbits
                temp = temp & (~(offbits << lshift)); // delete leftover
                display.screen[si] = temp;// save; //temp; // store 8-lshift leftmost bits
            }
        }
        // END OF LINES (Y) LOOP

    } else
        {
        // CROPPED CASE
        rshift = x&7;      // rshift = bits OVER even bytes, eg. 14 = 110
        if (rshift) {
            lshift = 8-rshift; // lshift = bits UNTIL next even byte, eg. 2 = 10
        } else {
            rshift = 8;        // avoid divisible by eight cases = masks second byte
        }
        width = bmp[i]; i++; //width = pgm_read_byte((uint32_t)(bmp) + i);
        if (width =< -x) return; // its not visible anymore
        lines = bmp[i]; i++;//lines = pgm_read_byte((uint32_t)(bmp) + i);
        width+=x; // delete cropped width from width
        if (width&7) {      // if width is not divisible by 8
                xtra = 8-width&7;   // xtra is amount of xtra bits
                width = width/8;  // width is width in even bytes
                //width++;          // plus one uneven one ...
        } else {              // width IS divisible by 8
                xtra = 0;         // no extra
                width = width/8;  // width = even num of bytes
                //width++;
        }
        // START Y LOOP
        for (uint8_t l = 0; l < lines; l++) {
            i+=-x/8; // increment bmp pointer i to point to first visible byte
            si = ((y + l) % display.vres)*display.hres; // si is index to screen
            save = display.screen[si]; // Save is the screen byte

            // START X LOOP
            for ( uint16_t b = i + width; i < b;) {
                // Add left shifted bits of first byte
                atemp = alpha[i]; // read first byte of alpha data
                temp = bmp[i]; // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data
                onbits = temp & atemp; // get bits that should be turned on
                offbits = ~temp & atemp; // get bits that should be turned off
                temp = (onbits << lshift ) | save; // add onbits to screen pixel
                save = temp & (~(offbits << lshift)); // delete offbits, store result

                // Add right shifted bits of second byte
                atemp = alpha[i+1]; // read first byte of alpha data
                temp = bmp[i+1];    // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data
                onbits = temp & atemp; // get bits that should be turned on
                offbits = ~temp & atemp; // get bits that should be turned off
                temp = (onbits >> rshift ) | save; // add onbits to intermediate result
                temp = temp & (~(offbits >> rshift)); // delete offbits, store result
                //temp = 0xFF;
                display.screen[si] = temp; si++; //first byte to screen buffer, increment
                i++;
            }
            // END OF X LOOP

            if (xtra) { // There are extra bits remaining
                save = display.screen[si]; // Save is the screen byte
                atemp = alpha[i]; // read first byte of alpha data
                temp = bmp[i]; // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data
                onbits = temp & atemp; // get bits that should be turned on
                offbits = ~temp & atemp; // get bits that should be turned off
                temp = (onbits << lshift ) | save; // add onbits to screen pixel
                save = temp & (~(offbits << lshift)); // delete offbits, store result

                // Add right shifted bits of second byte, will be masked out if unnecessary
                atemp = alpha[i+1]; // read first byte of alpha data
                temp = bmp[i+1];    // temp = pgm_read_byte((uint32_t)(bmp) + i++); //bmp data
                onbits = temp & atemp; // get bits that should be turned on
                offbits = ~temp & atemp; // get bits that should be turned off
                temp = (onbits >> rshift ) | save; // add onbits to intermediate result
                temp = temp & (~(offbits >> rshift)); // delete offbits, store result
                temp = (temp >> xtra) << xtra; // clean unwanted bits
                //temp = 0xff;
                display.screen[si] = temp; //first byte to screen buffer, increment
                if ((-x&7)>xtra && lshift) i++; // cases where last si byte includes only a part of last i byte, also avoid divisible by eight cases
            }
            i++;
        }
        // END OF LINES (Y) LOOP
        }
} // end of alphabitmap

