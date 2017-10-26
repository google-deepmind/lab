/*
   ======================================================================
   clip.c

   Functions for LWO2 image references.

   Ernie Wright  17 Sep 00
   ====================================================================== */

#include "../picointernal.h"
#include "lwo2.h"


/*
   ======================================================================
   lwFreeClip()

   Free memory used by an lwClip.
   ====================================================================== */

void lwFreeClip( lwClip *clip ){
	if ( clip ) {
		lwListFree( (void*) clip->ifilter, (ListFreeFunc) lwFreePlugin );
		lwListFree( (void*) clip->pfilter, (ListFreeFunc) lwFreePlugin );

		switch ( clip->type ) {
		case ID_STIL:
			_pico_free( clip->source.still.name );
			break;

		case ID_ISEQ:
			_pico_free( clip->source.seq.prefix );
			_pico_free( clip->source.seq.suffix );
			break;

		case ID_ANIM:
			_pico_free( clip->source.anim.name );
			_pico_free( clip->source.anim.server );
			_pico_free( clip->source.anim.data );
			break;

		case ID_XREF:
			_pico_free( clip->source.xref.string );
			break;

		case ID_STCC:
			_pico_free( clip->source.cycle.name );
			break;

		default:
			break;
		}

		_pico_free( clip );
	}
}


/*
   ======================================================================
   lwGetClip()

   Read image references from a CLIP chunk in an LWO2 file.
   ====================================================================== */

lwClip *lwGetClip( picoMemStream_t *fp, int cksize ){
	lwClip *clip;
	lwPlugin *filt;
	unsigned int id;
	unsigned short sz;
	int pos, rlen;


	/* allocate the Clip structure */

	clip = _pico_calloc( 1, sizeof( lwClip ) );
	if ( !clip ) {
		goto Fail;
	}

	clip->contrast.val = 1.0f;
	clip->brightness.val = 1.0f;
	clip->saturation.val = 1.0f;
	clip->gamma.val = 1.0f;

	/* remember where we started */

	set_flen( 0 );
	pos = _pico_memstream_tell( fp );

	/* index */

	clip->index = getI4( fp );

	/* first subchunk header */

	clip->type = getU4( fp );
	sz = getU2( fp );
	if ( 0 > get_flen() ) {
		goto Fail;
	}

	sz += sz & 1;
	set_flen( 0 );

	switch ( clip->type ) {
	case ID_STIL:
		clip->source.still.name = getS0( fp );
		break;

	case ID_ISEQ:
		clip->source.seq.digits  = getU1( fp );
		clip->source.seq.flags   = getU1( fp );
		clip->source.seq.offset  = getI2( fp );
		getU2( fp );   /* not sure what this is yet */
		clip->source.seq.start   = getI2( fp );
		clip->source.seq.end     = getI2( fp );
		clip->source.seq.prefix  = getS0( fp );
		clip->source.seq.suffix  = getS0( fp );
		break;

	case ID_ANIM:
		clip->source.anim.name   = getS0( fp );
		clip->source.anim.server = getS0( fp );
		rlen = get_flen();
		clip->source.anim.data   = getbytes( fp, sz - rlen );
		break;

	case ID_XREF:
		clip->source.xref.index  = getI4( fp );
		clip->source.xref.string = getS0( fp );
		break;

	case ID_STCC:
		clip->source.cycle.lo   = getI2( fp );
		clip->source.cycle.hi   = getI2( fp );
		clip->source.cycle.name = getS0( fp );
		break;

	default:
		break;
	}

	/* error while reading current subchunk? */

	rlen = get_flen();
	if ( rlen < 0 || rlen > sz ) {
		goto Fail;
	}

	/* skip unread parts of the current subchunk */

	if ( rlen < sz ) {
		_pico_memstream_seek( fp, sz - rlen, PICO_SEEK_CUR );
	}

	/* end of the CLIP chunk? */

	rlen = _pico_memstream_tell( fp ) - pos;
	if ( cksize < rlen ) {
		goto Fail;
	}
	if ( cksize == rlen ) {
		return clip;
	}

	/* process subchunks as they're encountered */

	id = getU4( fp );
	sz = getU2( fp );
	if ( 0 > get_flen() ) {
		goto Fail;
	}

	while ( 1 ) {
		sz += sz & 1;
		set_flen( 0 );

		switch ( id ) {
		case ID_TIME:
			clip->start_time = getF4( fp );
			clip->duration = getF4( fp );
			clip->frame_rate = getF4( fp );
			break;

		case ID_CONT:
			clip->contrast.val = getF4( fp );
			clip->contrast.eindex = getVX( fp );
			break;

		case ID_BRIT:
			clip->brightness.val = getF4( fp );
			clip->brightness.eindex = getVX( fp );
			break;

		case ID_SATR:
			clip->saturation.val = getF4( fp );
			clip->saturation.eindex = getVX( fp );
			break;

		case ID_HUE:
			clip->hue.val = getF4( fp );
			clip->hue.eindex = getVX( fp );
			break;

		case ID_GAMM:
			clip->gamma.val = getF4( fp );
			clip->gamma.eindex = getVX( fp );
			break;

		case ID_NEGA:
			clip->negative = getU2( fp );
			break;

		case ID_IFLT:
		case ID_PFLT:
			filt = _pico_calloc( 1, sizeof( lwPlugin ) );
			if ( !filt ) {
				goto Fail;
			}

			filt->name = getS0( fp );
			filt->flags = getU2( fp );
			rlen = get_flen();
			filt->data = getbytes( fp, sz - rlen );

			if ( id == ID_IFLT ) {
				lwListAdd( (void **) &clip->ifilter, filt );
				clip->nifilters++;
			}
			else {
				lwListAdd( (void **) &clip->pfilter, filt );
				clip->npfilters++;
			}
			break;

		default:
			break;
		}

		/* error while reading current subchunk? */

		rlen = get_flen();
		if ( rlen < 0 || rlen > sz ) {
			goto Fail;
		}

		/* skip unread parts of the current subchunk */

		if ( rlen < sz ) {
			_pico_memstream_seek( fp, sz - rlen, PICO_SEEK_CUR );
		}

		/* end of the CLIP chunk? */

		rlen = _pico_memstream_tell( fp ) - pos;
		if ( cksize < rlen ) {
			goto Fail;
		}
		if ( cksize == rlen ) {
			break;
		}

		/* get the next chunk header */

		set_flen( 0 );
		id = getU4( fp );
		sz = getU2( fp );
		if ( 6 != get_flen() ) {
			goto Fail;
		}
	}

	return clip;

Fail:
	lwFreeClip( clip );
	return NULL;
}


/*
   ======================================================================
   lwFindClip()

   Returns an lwClip pointer, given a clip index.
   ====================================================================== */

lwClip *lwFindClip( lwClip *list, int index ){
	lwClip *clip;

	clip = list;
	while ( clip ) {
		if ( clip->index == index ) {
			break;
		}
		clip = clip->next;
	}
	return clip;
}
