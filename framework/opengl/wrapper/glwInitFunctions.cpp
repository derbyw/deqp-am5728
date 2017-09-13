/*-------------------------------------------------------------------------
 * drawElements Quality Program OpenGL Utilities
 * ---------------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Function table initialization.
 *//*--------------------------------------------------------------------*/

#include <stdio.h>
#include "GLES2/gl2.h"
#include "glwInitFunctions.hpp"
#include "deSTLUtil.hpp"

#include <string>
#include <set>

namespace glw
{

// \todo [2014-03-19 pyry] Replace this with more generic system based on upstream XML spec desc.


// hold on to actual readpixel so we can hijack it.
void (*saved_readpixel)(GLint x, GLint y, GLsizei width, GLsizei height,
				 GLenum format, GLenum datatype, GLvoid *data);


// hold on to the glGetError function as well..
unsigned int (*glGetError)();
void (*glGetIntegerv)(GLenum pname, GLint *params);
void (*glPixelStorei)(GLenum pname, GLint params);

/*
 * insert stub into read pixel to see why its failing
 */
void MyReadPixel(GLint x, GLint y, GLsizei width, GLsizei height,
				 GLenum format, GLenum datatype, GLvoid *data)

{
	GLenum err;
	GLint  readbuf_format;
	GLint  readbuf_type;

	fprintf(stderr,"readpixel called %d %d w %d h %d format 0x%X type 0x%X\n",x,y,width,height,format,datatype);

	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readbuf_format);
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readbuf_type);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);  // shoudl be default...

	fprintf(stderr,"readpixel supports format 0x%X and type 0x%X\n",readbuf_format, readbuf_type);
	
	while((err = (*glGetError)()) != GL_NO_ERROR) {
 		fprintf(stderr,"Flushing GL Errors  0x%X\n",err);
	}


	(*saved_readpixel)(x,y,width,height,format,datatype, data);
	fprintf(stderr,"post readpixel errors....\n");
	while((err = (*glGetError)()) != GL_NO_ERROR) {
 		fprintf(stderr,"ReadPixel GL Error  0x%X\n",err);
	}
} 

void initES20 (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitES20.inl"

    glGetError = (unsigned int (*)())gl->getError;

    // stick in a middle man to see whats going on
    glGetIntegerv = (void (*)(GLenum, GLint *))gl->getIntegerv;
    glPixelStorei = (void (*)(GLenum, GLint))gl->pixelStorei;

    saved_readpixel = gl->readPixels;
    gl->readPixels = &MyReadPixel;


}



void initES30 (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitES30.inl"
}

void initES31 (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitES31.inl"
}

void initES32 (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitES32.inl"
}

void initGL30Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL30.inl"
}

void initGL31Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL31.inl"
}

void initGL32Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL32.inl"
}

void initGL33Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL33.inl"
}

void initGL40Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL40.inl"
}

void initGL41Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL41.inl"
}

void initGL42Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL42.inl"
}

void initGL43Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL43.inl"
}

void initGL44Core (Functions* gl, const FunctionLoader* loader)
{
#include "glwInitGL44.inl"
}

void initExtensionsGL (Functions* gl, const FunctionLoader* loader, int numExtensions, const char* const* extensions)
{
	using std::string;
	using std::set;

	const set<string> extSet(extensions, extensions+numExtensions);

#include "glwInitExtGL.inl"

}

void initExtensionsES (Functions* gl, const FunctionLoader* loader, int numExtensions, const char* const* extensions)
{
	using std::string;
	using std::set;

	const set<string> extSet(extensions, extensions+numExtensions);

#include "glwInitExtES.inl"

}

} // glw
