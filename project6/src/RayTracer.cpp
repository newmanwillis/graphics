// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>

extern TraceUI* traceUI;

#include <iostream>
#include <fstream>

using namespace std;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
Vec3d RayTracer::trace( double x, double y )
{
  // Clear out the ray cache in the scene for debugging purposes,
  scene->intersectCache.clear();

    ray r(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 0.0, 0.0), ray::VISIBILITY );
    scene->getCamera().rayThrough(x, y, r);
    Vec3d ret = traceRay(r, Vec3d(1.0, 1.0, 1.0), 0.0);
    ret.clamp();
    return ret;
}

Vec3d RayTracer::traceRay( const ray& r, const Vec3d& thresh, int depth )
{
	isect i;
	if (scene->intersect(r, i)) {  // Find intersection
	  const Material& m = i.getMaterial();
	  
	  // Ensure the depth has not been reached
	  if (depth == traceUI->getDepth()) { 
	    return m.shade(scene, r, i); }

	  Vec3d intersection_pos = r.at(i.t);
	  Vec3d v = (-1.0) * r.getDirection();
	  Vec3d n = i.N;
	  n.normalize();
	  v.normalize();

	  // Calculate Reflection
	  Vec3d total_reflection = Vec3d(0, 0, 0);
	  if (!m.kr(i).iszero()) {
	    Vec3d reflection_direction = 2 * (v * n) * n - v;
	    ray reflection = ray(intersection_pos, reflection_direction,
                         ray::REFLECTION);
	    // Ray tracing reflection recursion
	    total_reflection = prod(m.kr(i),
                           traceRay(reflection, thresh, depth + 1.0));
	  }

	  // Calculate Transmission
	  Vec3d total_transmission = Vec3d(0.0, 0.0, 0.0);
	 
	  if (!m.kt(i).iszero()) {
	    double index = 1.0 / (m.index(i));
	    
	    // If the ray is travelling object->air
	    if (depth % 2 == 1) {
	      n = (-1.0) * n;
	      index = 1.0 / index;
	    }
	    
	    // Calculating transmission direction
	    float cosi = n * v;
	    float cost = 1.0 - pow(index, 2.0) * (1.0 - pow(cosi, 2.0));
	    if (cost >= 0.0) {
	      cost = sqrt(cost);
	      Vec3d t = (index * cosi - cost) * n - index * v;
	      t.normalize();
	      ray transmission = ray(intersection_pos, t, ray::REFRACTION);
	      
	      // Ray tracing transmission recursion
	      total_transmission = prod(m.kt(i),
                               traceRay(transmission, thresh, depth + 1.0));
	    }
	  }
	  
	  // Total shade of the intersection point
	  return m.shade(scene, r, i) + total_reflection + total_transmission;
	}
	else 
	  return Vec3d(0.0, 0.0, 0.0);
}

RayTracer::RayTracer()
	: scene( 0 ), buffer( 0 ), buffer_width( 256 ), buffer_height( 256 ), m_bBufferReady( false )
{

}


RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn )
{
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}
	
	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos )
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
    Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}


	if( ! sceneLoaded() )
		return false;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];


	}
	memset( buffer, 0, w*h*3 );
	m_bBufferReady = true;
}

void RayTracer::tracePixel( int i, int j )
{
	Vec3d col;

	if( ! sceneLoaded() )
		return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);


	col = trace( x,y );

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}


